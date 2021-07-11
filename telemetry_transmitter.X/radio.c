#include <stdbool.h>
#include <string.h>

#include "mcc_generated_files/interrupt_manager.h"
#include <xc.h>

#include "radio.h"
#include "radio_config.h"
#include "spi.h"
#include "timer.h"

/* 
 * RFM95W Datasheet: 
 * https://www.rfsolutions.co.uk/downloads/1463993415RFM95_96_97_98W.pdf
 */

/* 
 * Contents of radio.c are based off this C++ implementation of 
 * RFM95 LoRA drivers by Mike McCauley (mikem@airspayce.com): 
 * hhttps://github.com/Swap-File/RFM95W/blob/master/RadioHead/RFM95.cpp
 * 
 */

// These are indexed by the values of ModemConfigChoice
// Stored in flash (program) memory to save SRAM
static const modem_config MODEM_CONFIG_TABLE[] = {
    //  1d,   1e,   26
    { 0x72, 0x74, 0x00}, // Bw125Cr45Sf128 (the chip default)
    { 0x92, 0x74, 0x00}, // Bw500Cr45Sf128
    { 0x48, 0x94, 0x00}, // Bw31_25Cr48Sf512
    { 0x78, 0xc4, 0x00}, // Bw125Cr48Sf4096

};

bool init_radio() {
    // No way to check the device type :-(

    // Set sleep mode, so we can also set LORA mode:
    radio_spi_write_byte(RFM95_REG_01_OP_MODE, RFM95_MODE_SLEEP | RFM95_LONG_RANGE_MODE);
    __delay_ms(10); // Wait for sleep mode to take over from say, CAD

    // Check we are in sleep mode, with LORA set
    if (radio_spi_read_byte(RFM95_REG_01_OP_MODE) != (RFM95_MODE_SLEEP | RFM95_LONG_RANGE_MODE)) {
        return false; // No device present?
    }

    // Set up FIFO
    // We configure so that we can use the entire 256 byte FIFO for either receive
    // or transmit, but not both at the same time
    radio_spi_write_byte(RFM95_REG_0E_FIFO_TX_BASE_ADDR, 0);
    radio_spi_write_byte(RFM95_REG_0F_FIFO_RX_BASE_ADDR, 0);

    // Packet format is preamble + explicit-header + payload + CRC
    // Explicit Header Mode
    // payload is TO + FROM + ID + FLAGS + message data
    // RX mode is implemented with RXCONTINUOUS
    // max message data length is 255 - 4 = 251 bytes

    set_mode_idle();

    // Set up default configuration
    // No Sync Words in LORA mode.
    set_modem_config(Bw125Cr45Sf128); // Radio default
    // setModemConfig(Bw125Cr48Sf4096); // slow and reliable?
    set_preamble_len(8); // Default is 8
    // An innocuous ISM frequency
    set_frequency(434.0);
    // Low power
    set_TX_pow(13, false);

    return true;
}

// C++ level interrupt handler for this instance
// LORA is unusual in that it has several interrupt lines, and not a single, combined one.
// We use this to get RxDone and TxDone interrupts

void radio_interrupt_handler() {
    // Read the interrupt register
    uint8_t irq_flags = radio_spi_read_byte(RFM95_REG_12_IRQ_FLAGS);
    if (_curr_mode == RadModeRX && irq_flags & (RFM95_RX_TIMEOUT | RFM95_PAYLOAD_CRC_ERROR)) {
        _num_rx_bad_msgs++;
    } else if (_curr_mode == RadModeRX && irq_flags & RFM95_RX_DONE) {
        // Have received a packet
        uint8_t len = radio_spi_read_byte(RFM95_REG_13_RX_NB_BYTES);

        // Reset the fifo read ptr to the beginning of the packet
        radio_spi_write_byte(RFM95_REG_0D_FIFO_ADDR_PTR, radio_spi_read_byte(RFM95_REG_10_FIFO_RX_CURRENT_ADDR));
        radio_spi_read_buf(RFM95_REG_00_FIFO, _recv_buf, len);
        _recv_buf_len = len;
        radio_spi_write_byte(RFM95_REG_12_IRQ_FLAGS, 0xff); // Clear all IRQ flags

        // Remember the RSSI of this packet
        // weakest receiveable signals are reported RSSI at about -66
        _last_RSSI = radio_spi_read_byte(RFM95_REG_1A_PKT_RSSI_VALUE) - 137;

        // We have received a message.
        validate_RX_buf();
        if (_valid_RX_buf)
            set_mode_idle(); // Got one 
    } else if (_curr_mode == RadModeTX && irq_flags & RFM95_TX_DONE) {
        _num_tx_good_msgs++;
        set_mode_idle();
    } else if (_curr_mode == RadModeCad && irq_flags & RFM95_CAD_DONE) {
        _cad = irq_flags & RFM95_CAD_DETECTED;
        set_mode_idle();
    }

    radio_spi_write_byte(RFM95_REG_12_IRQ_FLAGS, 0xff); // Clear all IRQ flags
}

// Check whether the latest received message is complete and un-corrupted

void validate_RX_buf() {
    // Verify message length
    if (_recv_buf_len < 4)
        return;

    // Verify if the recipient address is this radio node
    if (_recv_buf[0] == CURR_RADIO_NODE_ADDRESS) {
        _num_rx_good_msgs++;
        _valid_RX_buf = true;
    }
}

bool available() {
    if (_curr_mode == RadModeTX)
        return false;
    set_mode_RX();
    return _valid_RX_buf; // Will be set by the interrupt handler when a good message is received
}

void clear_RX_buf() {
    INTERRUPT_GlobalInterruptDisable();
    _valid_RX_buf = false;
    _recv_buf_len = 0;
    INTERRUPT_GlobalInterruptEnable();
}

bool radio_receive_data(uint8_t* buf, uint8_t* len) {
    if (!available())
        return false;
    if (buf && len) {
        INTERRUPT_GlobalInterruptDisable();
        // Skip the 4 headers that are at the beginning of the rxBuf
        if (*len > _recv_buf_len - RFM95_HEADER_LEN)
            *len = _recv_buf_len - RFM95_HEADER_LEN;
        memcpy(buf, _recv_buf + RFM95_HEADER_LEN, *len);
        INTERRUPT_GlobalInterruptEnable();
    }
    clear_RX_buf(); // This message accepted and cleared
    return true;
}

bool radio_transmit_data(const uint8_t* data, uint8_t len) {
    if (len > RFM95_MAX_MESSAGE_LEN)
        return false;

    wait_packet_sent(100); // Make sure we don't interrupt an outgoing message
    set_mode_idle();

    if (!wait_CAD())
        return false; // Check channel activity

    // Position at the beginning of the FIFO
    radio_spi_write_byte(RFM95_REG_0D_FIFO_ADDR_PTR, 0);
    // The headers
    radio_spi_write_byte(RFM95_REG_00_FIFO, DEST_RADIO_NODE_ADDRESSES[NODE_INDEX]);
    radio_spi_write_byte(RFM95_REG_00_FIFO, CURR_RADIO_NODE_ADDRESS);
    radio_spi_write_byte(RFM95_REG_00_FIFO, CURR_RADIO_NODE_ID);
    radio_spi_write_byte(RFM95_REG_00_FIFO, CURR_RADIO_NODE_FLAGS);
    // The message data
    radio_spi_write_buf(RFM95_REG_00_FIFO, data, len);
    radio_spi_write_byte(RFM95_REG_22_PAYLOAD_LENGTH, len + RFM95_HEADER_LEN);

    set_mode_TX(); // Start the transmitter
    // when Tx is done, interruptHandler will fire and radio mode will return to STANDBY
    return true;
}

bool print_registers() {
#ifdef RH_HAVE_SERIAL
    uint8_t registers[] = {0x01, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x014, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};

    uint8_t i;
    for (i = 0; i < sizeof (registers); i++) {
        Serial.print(registers[i], HEX);
        Serial.print(": ");
        Serial.println(radio_spi_read_byte(registers[i]), HEX);
    }
    return true;
#else
    return false;
#endif
}

uint8_t max_msg_len() {
    return RFM95_MAX_MESSAGE_LEN;
}

bool set_frequency(float centre) {
    // Frf = FRF / FSTEP
    uint32_t frf = (centre * 1000000.0) / RFM95_FSTEP;
    radio_spi_write_byte(RFM95_REG_06_FRF_MSB, (frf >> 16) & 0xff);
    radio_spi_write_byte(RFM95_REG_07_FRF_MID, (frf >> 8) & 0xff);
    radio_spi_write_byte(RFM95_REG_08_FRF_LSB, frf & 0xff);

    return true;
}

void set_mode_idle() {
    if (_curr_mode != RadModeIdle) {
        radio_spi_write_byte(RFM95_REG_01_OP_MODE, RFM95_MODE_STDBY);
        _curr_mode = RadModeIdle;
    }
}

bool sleep() {
    if (_curr_mode != RadModeSleep) {
        radio_spi_write_byte(RFM95_REG_01_OP_MODE, RFM95_MODE_SLEEP);
        _curr_mode = RadModeSleep;
    }
    return true;
}

void set_mode_RX() {
    if (_curr_mode != RadModeRX) {
        radio_spi_write_byte(RFM95_REG_01_OP_MODE, RFM95_MODE_RXCONTINUOUS);
        radio_spi_write_byte(RFM95_REG_40_DIO_MAPPING1, 0x00); // Interrupt on RxDone
        _curr_mode = RadModeRX;
    }
}

void set_mode_TX() {
    if (_curr_mode != RadModeTX) {
        radio_spi_write_byte(RFM95_REG_01_OP_MODE, RFM95_MODE_TX);
        radio_spi_write_byte(RFM95_REG_40_DIO_MAPPING1, 0x40); // Interrupt on TxDone
        _curr_mode = RadModeTX;
    }
}

void set_TX_pow(int8_t power, bool useRFO) {
    // Sigh, different behaviours depending on whther the module use PA_BOOST or the RFO pin
    // for the transmitter output
    if (useRFO) {
        if (power > 14)
            power = 14;
        if (power < -1)
            power = -1;
        radio_spi_write_byte(RFM95_REG_09_PA_CONFIG, RFM95_MAX_POWER | (power + 1));
    } else {
        if (power > 23)
            power = 23;
        if (power < 5)
            power = 5;

        // For RFM95_PA_DAC_ENABLE, manual says '+20dBm on PA_BOOST when OutputPower=0xf'
        // RFM95_PA_DAC_ENABLE actually adds about 3dBm to all power levels. We will us it
        // for 21, 22 and 23dBm
        if (power > 20) {
            radio_spi_write_byte(RFM95_REG_4D_PA_DAC, RFM95_PA_DAC_ENABLE);
            power -= 3;
        } else {
            radio_spi_write_byte(RFM95_REG_4D_PA_DAC, RFM95_PA_DAC_DISABLE);
        }

        // RFM95/96/97/98 does not have RFO pins connected to anything. Only PA_BOOST
        // pin is connected, so must use PA_BOOST
        // Pout = 2 + OutputPower.
        // The documentation is pretty confusing on this topic: PaSelect says the max power is 20dBm,
        // but OutputPower claims it would be 17dBm.
        // My measurements show 20dBm is correct
        radio_spi_write_byte(RFM95_REG_09_PA_CONFIG, RFM95_PA_SELECT | (power - 5));
    }
}

// Sets registers from a canned modem configuration structure

void set_modem_registers(const modem_config* config) {
    radio_spi_write_byte(RFM95_REG_1D_MODEM_CONFIG1, config->reg_1d);
    radio_spi_write_byte(RFM95_REG_1E_MODEM_CONFIG2, config->reg_1e);
    radio_spi_write_byte(RFM95_REG_26_MODEM_CONFIG3, config->reg_26);
}

// Set one of the canned FSK Modem configs
// Returns true if its a valid choice

bool set_modem_config(modem_config_choice index) {
    if (index > (signed int) (sizeof (MODEM_CONFIG_TABLE) / sizeof (modem_config)))
        return false;

    modem_config cfg;
    memcpy(&cfg, &MODEM_CONFIG_TABLE[index], sizeof (modem_config));
    set_modem_registers(&cfg);

    return true;
}

void set_preamble_len(uint16_t bytes) {
    radio_spi_write_byte(RFM95_REG_20_PREAMBLE_MSB, bytes >> 8);
    radio_spi_write_byte(RFM95_REG_21_PREAMBLE_LSB, bytes & 0xff);
}

uint8_t radio_spi_read_byte(uint8_t reg) {
    uint8_t data;
    INTERRUPT_GlobalInterruptDisable();
    SPI_SS_LOW();
    spi_write_byte(reg & ~SPI_WRITE_MASK); // Send address w/ write mask off
    data = spi_read_byte(); // read data
    SPI_SS_HIGH();
    INTERRUPT_GlobalInterruptEnable();
    return data;
}

void radio_spi_write_byte(uint8_t reg, uint8_t data) {
    INTERRUPT_GlobalInterruptDisable();
    SPI_SS_LOW();
    spi_write_byte(reg | SPI_WRITE_MASK); // Send address w/ write mask on
    spi_write_byte(data); // Send data
    SPI_SS_HIGH();
    INTERRUPT_GlobalInterruptEnable();
}

void radio_spi_read_buf(uint8_t reg, uint8_t *data, uint8_t len) {
    INTERRUPT_GlobalInterruptDisable();
    SPI_SS_LOW();
    spi_write_byte(reg & ~SPI_WRITE_MASK); // Send address w/ write mask off
    while (len--)
        *data++ = spi_read_byte(); // read data
    SPI_SS_HIGH();
    INTERRUPT_GlobalInterruptEnable();
}

void radio_spi_write_buf(uint8_t reg, const uint8_t *data, uint8_t len) {
    INTERRUPT_GlobalInterruptDisable();
    SPI_SS_LOW();
    spi_write_byte(reg | SPI_WRITE_MASK); // Send address w/ write mask on
    spi_write_buffer(data, len); // Send data
    SPI_SS_HIGH();
    INTERRUPT_GlobalInterruptEnable();
}

bool is_channel_active() {
    // Set mode RadModeCad
    if (_curr_mode != RadModeCad) {
        radio_spi_write_byte(RFM95_REG_01_OP_MODE, RFM95_MODE_CAD);
        radio_spi_write_byte(RFM95_REG_40_DIO_MAPPING1, 0x80); // Interrupt on CadDone
        _curr_mode = RadModeCad;
    }

    while (_curr_mode == RadModeCad)
        __delay_ms(10);

    return _cad;
}

bool wait_CAD(void) {
    if (!_cad_timeout)
        return true;

    // Wait for any channel activity to finish or timeout
    // Sophisticated DCF function...
    // DCF : BackoffTime = random() x aSlotTime
    // 100 - 1000 ms
    // 10 sec timeout
    unsigned long t = millis();
    while (is_channel_active()) {
        if (millis() - t > _cad_timeout)
            return false;
        __delay_ms(10);
    }

    return true;
}

void set_CAD_timeout(unsigned long cad_timeout)
{
    _cad_timeout = cad_timeout;
}

bool wait_packet_sent(uint16_t timeout)
{
    unsigned long starttime = millis();
    while ((millis() - starttime) < timeout)
    {
        if (_curr_mode != RadModeTX) // Any previous transmit finished?
           return true;
        __delay_ms(10);
    }
    return false;
}