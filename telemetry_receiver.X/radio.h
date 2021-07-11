#ifndef RADIO_H
#define	RADIO_H

#include <stdbool.h>

#include <xc.h>

#include "radio_config.h"
#include "spi.h"

/* 
 * RFM95W Datasheet: 
 * https://www.rfsolutions.co.uk/downloads/1463993415RFM95_96_97_98W.pdf
 */

/* 
 * Contents of radio.h/radio.c are based off this C++ implementation of 
 * RFM95 LoRA drivers by Mike McCauley (mikem@airspayce.com): 
 * https://github.com/Swap-File/RFM95W/blob/master/RadioHead/RH_RF95.h
 * 
 */

/* Write bit in the SPI address*/
#define SPI_WRITE_MASK 0x80

/* Macros to control SS Pin for SPI */
#define SPI_SS_HIGH() (LATC3 = 1)
#define SPI_SS_LOW() (LATC3 = 0)

/* Register names (LoRa Mode, from table 85 on page 84 of the datasheet) */
#define RFM95_REG_00_FIFO                                0x00
#define RFM95_REG_01_OP_MODE                             0x01
#define RFM95_REG_02_RESERVED                            0x02
#define RFM95_REG_03_RESERVED                            0x03
#define RFM95_REG_04_RESERVED                            0x04
#define RFM95_REG_05_RESERVED                            0x05
#define RFM95_REG_06_FRF_MSB                             0x06
#define RFM95_REG_07_FRF_MID                             0x07
#define RFM95_REG_08_FRF_LSB                             0x08
#define RFM95_REG_09_PA_CONFIG                           0x09
#define RFM95_REG_0A_PA_RAMP                             0x0a
#define RFM95_REG_0B_OCP                                 0x0b
#define RFM95_REG_0C_LNA                                 0x0c
#define RFM95_REG_0D_FIFO_ADDR_PTR                       0x0d
#define RFM95_REG_0E_FIFO_TX_BASE_ADDR                   0x0e
#define RFM95_REG_0F_FIFO_RX_BASE_ADDR                   0x0f
#define RFM95_REG_10_FIFO_RX_CURRENT_ADDR                0x10
#define RFM95_REG_11_IRQ_FLAGS_MASK                      0x11
#define RFM95_REG_12_IRQ_FLAGS                           0x12
#define RFM95_REG_13_RX_NB_BYTES                         0x13
#define RFM95_REG_14_RX_HEADER_CNT_VALUE_MSB             0x14
#define RFM95_REG_15_RX_HEADER_CNT_VALUE_LSB             0x15
#define RFM95_REG_16_RX_PACKET_CNT_VALUE_MSB             0x16
#define RFM95_REG_17_RX_PACKET_CNT_VALUE_LSB             0x17
#define RFM95_REG_18_MODEM_STAT                          0x18
#define RFM95_REG_19_PKT_SNR_VALUE                       0x19
#define RFM95_REG_1A_PKT_RSSI_VALUE                      0x1a
#define RFM95_REG_1B_RSSI_VALUE                          0x1b
#define RFM95_REG_1C_HOP_CHANNEL                         0x1c
#define RFM95_REG_1D_MODEM_CONFIG1                       0x1d
#define RFM95_REG_1E_MODEM_CONFIG2                       0x1e
#define RFM95_REG_1F_SYMB_TIMEOUT_LSB                    0x1f
#define RFM95_REG_20_PREAMBLE_MSB                        0x20
#define RFM95_REG_21_PREAMBLE_LSB                        0x21
#define RFM95_REG_22_PAYLOAD_LENGTH                      0x22
#define RFM95_REG_23_MAX_PAYLOAD_LENGTH                  0x23
#define RFM95_REG_24_HOP_PERIOD                          0x24
#define RFM95_REG_25_FIFO_RX_BYTE_ADDR                   0x25
#define RFM95_REG_26_MODEM_CONFIG3                       0x26

#define RFM95_REG_40_DIO_MAPPING1                        0x40
#define RFM95_REG_41_DIO_MAPPING2                        0x41
#define RFM95_REG_42_VERSION                             0x42

#define RFM95_REG_4B_TCXO                                0x4b
#define RFM95_REG_4D_PA_DAC                              0x4d
#define RFM95_REG_5B_FORMER_TEMP                         0x5b
#define RFM95_REG_61_AGC_REF                             0x61
#define RFM95_REG_62_AGC_THRESH1                         0x62
#define RFM95_REG_63_AGC_THRESH2                         0x63
#define RFM95_REG_64_AGC_THRESH3                         0x64

// RFM95_REG_01_OP_MODE                             0x01
#define RFM95_LONG_RANGE_MODE                       0x80
#define RFM95_ACCESS_SHARED_REG                     0x40
#define RFM95_LOW_FREQUENCY_MODE                    0x08
#define RFM95_MODE                                  0x07
#define RFM95_MODE_SLEEP                            0x00
#define RFM95_MODE_STDBY                            0x01
#define RFM95_MODE_FSTX                             0x02
#define RFM95_MODE_TX                               0x03
#define RFM95_MODE_FSRX                             0x04
#define RFM95_MODE_RXCONTINUOUS                     0x05
#define RFM95_MODE_RXSINGLE                         0x06
#define RFM95_MODE_CAD                              0x07

// RFM95_REG_09_PA_CONFIG                           0x09
#define RFM95_PA_SELECT                             0x80
#define RFM95_MAX_POWER                             0x70
#define RFM95_OUTPUT_POWER                          0x0f

// RFM95_REG_0A_PA_RAMP                             0x0a
#define RFM95_LOW_PN_TX_PLL_OFF                     0x10
#define RFM95_PA_RAMP                               0x0f
#define RFM95_PA_RAMP_3_4MS                         0x00
#define RFM95_PA_RAMP_2MS                           0x01
#define RFM95_PA_RAMP_1MS                           0x02
#define RFM95_PA_RAMP_500US                         0x03
#define RFM95_PA_RAMP_250US                         0x0
#define RFM95_PA_RAMP_125US                         0x05
#define RFM95_PA_RAMP_100US                         0x06
#define RFM95_PA_RAMP_62US                          0x07
#define RFM95_PA_RAMP_50US                          0x08
#define RFM95_PA_RAMP_40US                          0x09
#define RFM95_PA_RAMP_31US                          0x0a
#define RFM95_PA_RAMP_25US                          0x0b
#define RFM95_PA_RAMP_20US                          0x0c
#define RFM95_PA_RAMP_15US                          0x0d
#define RFM95_PA_RAMP_12US                          0x0e
#define RFM95_PA_RAMP_10US                          0x0f

// RFM95_REG_0B_OCP                                 0x0b
#define RFM95_OCP_ON                                0x20
#define RFM95_OCP_TRIM                              0x1f

// RFM95_REG_0C_LNA                                 0x0c
#define RFM95_LNA_GAIN                              0xe0
#define RFM95_LNA_GAIN_G1                           0x20
#define RFM95_LNA_GAIN_G2                           0x40
#define RFM95_LNA_GAIN_G3                           0x60                
#define RFM95_LNA_GAIN_G4                           0x80
#define RFM95_LNA_GAIN_G5                           0xa0
#define RFM95_LNA_GAIN_G6                           0xc0
#define RFM95_LNA_BOOST_LF                          0x18
#define RFM95_LNA_BOOST_LF_DEFAULT                  0x00
#define RFM95_LNA_BOOST_HF                          0x03
#define RFM95_LNA_BOOST_HF_DEFAULT                  0x00
#define RFM95_LNA_BOOST_HF_150PC                    0x11

// RFM95_REG_11_IRQ_FLAGS_MASK                      0x11
#define RFM95_RX_TIMEOUT_MASK                       0x80
#define RFM95_RX_DONE_MASK                          0x40
#define RFM95_PAYLOAD_CRC_ERROR_MASK                0x20
#define RFM95_VALID_HEADER_MASK                     0x10
#define RFM95_TX_DONE_MASK                          0x08
#define RFM95_CAD_DONE_MASK                         0x04
#define RFM95_FHSS_CHANGE_CHANNEL_MASK              0x02
#define RFM95_CAD_DETECTED_MASK                     0x01

// RFM95_REG_12_IRQ_FLAGS                           0x12
#define RFM95_RX_TIMEOUT                            0x80
#define RFM95_RX_DONE                               0x40
#define RFM95_PAYLOAD_CRC_ERROR                     0x20
#define RFM95_VALID_HEADER                          0x10
#define RFM95_TX_DONE                               0x08
#define RFM95_CAD_DONE                              0x04
#define RFM95_FHSS_CHANGE_CHANNEL                   0x02
#define RFM95_CAD_DETECTED                          0x01

// RFM95_REG_18_MODEM_STAT                          0x18
#define RFM95_RX_CODING_RATE                        0xe0
#define RFM95_MODEM_STATUS_CLEAR                    0x10
#define RFM95_MODEM_STATUS_HEADER_INFO_VALID        0x08
#define RFM95_MODEM_STATUS_RX_ONGOING               0x04
#define RFM95_MODEM_STATUS_SIGNAL_SYNCHRONIZED      0x02
#define RFM95_MODEM_STATUS_SIGNAL_DETECTED          0x01

// RFM95_REG_1C_HOP_CHANNEL                         0x1c
#define RFM95_PLL_TIMEOUT                           0x80
#define RFM95_RX_PAYLOAD_CRC_IS_ON                  0x40
#define RFM95_FHSS_PRESENT_CHANNEL                  0x3f

// RFM95_REG_1D_MODEM_CONFIG1                       0x1d
#define RFM95_BW                                    0xf0

#define RFM95_BW_7_8KHZ                             0x00
#define RFM95_BW_10_4KHZ                            0x10
#define RFM95_BW_15_6KHZ                            0x20
#define RFM95_BW_20_8KHZ                            0x30
#define RFM95_BW_31_25KHZ                           0x40
#define RFM95_BW_41_7KHZ                            0x50
#define RFM95_BW_62_5KHZ                            0x60
#define RFM95_BW_125KHZ                             0x70
#define RFM95_BW_250KHZ                             0x80
#define RFM95_BW_500KHZ                             0x90
#define RFM95_CODING_RATE                           0x0e
#define RFM95_CODING_RATE_4_5                       0x02
#define RFM95_CODING_RATE_4_6                       0x04
#define RFM95_CODING_RATE_4_7                       0x06
#define RFM95_CODING_RATE_4_8                       0x08
#define RFM95_IMPLICIT_HEADER_MODE_ON               0x01

// RFM95_REG_1E_MODEM_CONFIG2                       0x1e
#define RFM95_SPREADING_FACTOR                      0xf0
#define RFM95_SPREADING_FACTOR_64CPS                0x60
#define RFM95_SPREADING_FACTOR_128CPS               0x70
#define RFM95_SPREADING_FACTOR_256CPS               0x80
#define RFM95_SPREADING_FACTOR_512CPS               0x90
#define RFM95_SPREADING_FACTOR_1024CPS              0xa0
#define RFM95_SPREADING_FACTOR_2048CPS              0xb0
#define RFM95_SPREADING_FACTOR_4096CPS              0xc0
#define RFM95_TX_CONTINUOUS_MOE                     0x08

#define RFM95_PAYLOAD_CRC_ON                        0x04
#define RFM95_SYM_TIMEOUT_MSB                       0x03

// RFM95_REG_4D_PA_DAC                              0x4d
#define RFM95_PA_DAC_DISABLE                        0x04
#define RFM95_PA_DAC_ENABLE                         0x07

typedef enum {
    RadModeInitialising = 0, ///< Transport is initialising. Initial default value until init() is called...
    RadModeSleep, ///< Transport hardware is in low power sleep mode (if supported)
    RadModeIdle, ///< Transport is idle.
    RadModeTX, ///< Transport is in the process of transmitting a message.
    RadModeRX, ///< Transport is in the process of receiving a message.
    RadModeCad ///< Transport is in the process of detecting channel activity (if supported)
} RadMode;

/*
 * Number of bytes in the buffer
 */
volatile uint8_t _recv_buf_len;

/*
 * The receiver/transmitter buffer
 */
uint8_t _recv_buf[RFM95_MAX_PAYLOAD_LEN];

/*
 * Last received RSSI value // TODO: add units
 */
volatile uint8_t _last_RSSI;

/*
 * Current mode of the LoRa Module
 */
volatile RadMode _curr_mode;

/*
 * True when there is a valid message in the buffer
 */
volatile bool _valid_RX_buf;

/*
 * True when there is channel activity detected
 */
volatile bool _cad;

/*
 * Timeout time for CAD
 */
unsigned long _cad_timeout;

/*
 * Number of bad messages (eg. bad checksum) received
 */
volatile uint8_t _num_rx_bad_msgs;

/*
 * Number of good messages (eg. correct checksum) received
 */
volatile uint8_t _num_rx_good_msgs;

/*
 * Number of messages successfully transmitted
 */
volatile uint8_t _num_tx_good_msgs;

/*
 * \brief Defines register values for a set of modem configuration registers
 * Defines register values for a set of modem configuration registers
 * that can be passed to setModemRegisters() if none of the choices in
 * ModemConfigChoice suit your need setModemRegisters() writes the
 * register values from this structure to the appropriate registers
 * to set the desired spreading factor, coding rate and bandwidth
 */

typedef struct {
    uint8_t reg_1d; ///< Value for register RFM95_REG_1D_MODEM_CONFIG1
    uint8_t reg_1e; ///< Value for register RFM95_REG_1E_MODEM_CONFIG2
    uint8_t reg_26; ///< Value for register RFM95_REG_26_MODEM_CONFIG3
} modem_config;

/*
 * Initialize the Driver transport hardware and software.
 * Make sure the Driver is properly configured before calling init_radio().
 * \return true if initialization succeeded.
 */
bool init_radio(void);

/*
 * Prints the value of all chip registers to the Serial device if 
 * RH_HAVE_SERIAL is defined for the current platform.
 * For debugging purposes only.
 * \return true on success
 */
bool print_registers(void);

/*
 * Sets all the registered required to configure the data modem in the RF95/96/97/98, including the bandwidth, 
 * spreading factor etc. You can use this to configure the modem with custom configurations if none of the 
 * canned configurations in modem_config_choice suit you.
 * \param[in] config A modem_config structure containing values for the modem configuration registers.
 */
void set_modem_registers(const modem_config* config);

/*
 * Select one of the predefined modem configurations. If you need a modem configuration not provided 
 * here, use setModemRegisters() with your own ModemConfig.
 * \param[in] index The configuration choice.
 * \return true if index is a valid choice.
 */
bool set_modem_config(modem_config_choice index);

/*
 * Tests whether a new message is available
 * from the Driver. 
 * On most drivers, this will also put the Driver into RHModeRx mode until
 * a message is actually received by the transport, when it wil be returned to RHModeIdle.
 * This can be called multiple times in a timeout loop
 * \return true if a new, complete, error-free uncollected message is available to be retreived by recv()
 */
bool available(void);

/*
 * Turns the receiver on if it not already on.
 * If there is a valid message available, copy it to buf and return true
 * else return false.
 * If a message is copied, *len is set to the length (Caution, 0 length messages are permitted).
 * You should be sure to call this function frequently enough to not miss any messages
 * It is recommended that you call it in your main loop.
 * \param[in] buf Location to copy the received message
 * \param[in,out] len Pointer to available space in buf. Set to the actual number of bytes copied.
 * \return true if a valid message was copied to buf
 */
bool radio_receive_data(uint8_t* buf, uint8_t* len);

/*
 * Waits until any previous transmit packet is finished being transmitted with waitPacketSent().
 * Then optionally waits for Channel Activity Detection (CAD) 
 * to show the channnel is clear (if the radio supports CAD) by calling waitCAD().
 * Then loads a message into the transmitter and starts the transmitter. Note that a message length
 * of 0 is permitted. 
 * \param[in] data Array of data to be sent
 * \param[in] len Number of bytes of data to send
 * specify the maximum time in ms to wait. If 0 (the default) do not wait for CAD before transmitting.
 * \return true if the message length was valid and it was correctly queued for transmit. Return false
 * if CAD was requested and the CAD timeout timed out before clear channel was detected.
 */
bool radio_transmit_data(const uint8_t* data, uint8_t len);

/*
 * Sets the length of the preamble
 * in bytes. 
 * Caution: this should be set to the same 
 * value on all nodes in your network. Default is 8.
 * Sets the message preamble length in RFM95_REG_??_PREAMBLE_?SB
 * \param[in] bytes Preamble length in bytes.
 */
void set_preamble_len(uint16_t bytes);

/*
 * Returns the maximum message length 
 * available in this Driver.
 * \return The maximum legal message length
 */
uint8_t max_msg_len(void);

/*
 * Writes a single byte to the SPI device
 * \param[in] reg Register number
 * \param[in] data The data to write
 */
void radio_spi_write_byte(uint8_t reg, uint8_t data);

/*
 * Reads a single byte from the SPI device
 * \param[in] reg Register number
 * \return data byte from read.
 */
uint8_t radio_spi_read_byte(uint8_t reg);

/*
 * Writes multiple bytes of data to the SPI device
 * \param[in] reg Register number
 * \param[in] data The data to write
 * \param[in] len The number of data bytes
 */
void radio_spi_write_buf(uint8_t reg, const uint8_t *data, uint8_t len);

/*
 * Reads multiple bytes of data from the SPI device
 * \param[in] reg Register number
 * \param[out] data The data to read, contents are filled from read.
 * \param[in] len The number of data packets to read
 */
void radio_spi_read_buf(uint8_t reg, uint8_t *data, uint8_t len);

/*
 * Sets the transmitter and receiver 
 * center frequency.
 * \param[in] center Frequency in MHz. 137.0 to 1020.0. Caution: RFM95/96/97/98 comes in several
 * different frequency ranges, and setting a frequency outside that range of your radio will probably not work
 * \return true if the selected frequency center is within range
 */
bool set_frequency(float centre);

/*
 * If current mode is Rx or Tx changes it to Idle. If the transmitter or receiver is running, 
 * disables them.
 */
void set_mode_idle(void);

/*
 * If current mode is Tx or Idle, changes it to Rx. 
 * Starts the receiver in the RF95/96/97/98.
 */
void set_mode_RX(void);

/*
 * If current mode is Rx or Idle, changes it to Rx. F
 * Starts the transmitter in the RF95/96/97/98.
 */
void set_mode_TX(void);

/*
 * Sets the transmitter power output level, and configures the transmitter pin.
 * You must set the appropriate power level and useRFO argument for your module.
 * Failure to do so will result in very low transmitter power output.
 * Caution: legal power limits may apply in certain countries.
 * After init_radio(), the power will be set to 13dBm, with useRFO false (ie PA_BOOST enabled).
 * \param[in] power Transmitter power level in dBm. For RFM95/96/97/98 LORA with useRFO false, 
 * valid values are from +5 to +23.
 * For Modtronix inAir4 and inAir9 with useRFO true (ie RFO pins in use), 
 * valid values are from -1 to 14.
 * \param[in] useRFO If true, enables the use of the RFO transmitter pins instead of
 * the PA_BOOST pin (false). Choose the correct setting for your module.
 */
void set_TX_pow(int8_t power, bool useRFO);

/*
 * Sets the radio into low-power sleep mode.
 * If successful, the transport will stay in sleep mode until woken by 
 * changing mode it idle, transmit or receive (eg by calling send(), recv(), available() etc)
 * Caution: there is a time penalty as the radio takes a finite time to wake from sleep mode.
 * \return true if sleep mode was successfully entered.
 */
bool sleep(void);

/*
 * Bent G Christensen (bentor@gmail.com), 08/15/2016
 * Use the radio's Channel Activity Detect (CAD) function to detect channel activity.
 * Sets the RF95 radio into CAD mode and waits until CAD detection is complete.
 * To be used in a listen-before-talk mechanism (Collision Avoidance)
 * with a reasonable time backoff algorithm.
 * This is called automatically by waitCAD().
 * \return true if channel is in use.
 */
bool is_channel_active(void);

/*
 * This is a low level function to handle the interrupts for one instance of RFM95.
 */
void radio_interrupt_handler(void);

/*
 * Examine the receive buffer to determine whether the message is for this node
 */
void validate_RX_buf(void);

/*
 * Clear our local receive buffer
 */
void clear_RX_buf(void);

/*
 * Blocks until channel activity is finished or CAD timeout occurs.
 * Uses the radio's CAD function (if supported) to detect channel activity.
 * Implements random delays of 100 to 1000ms while activity is detected and until timeout.
 * Permits the implementation of listen-before-talk mechanism (Collision Avoidance).
 * Calls the isChannelActive() member function for the radio (if supported) 
 * to determine if the channel is active. If the radio does not support isChannelActive(),
 * always returns true immediately
 * \return true if the radio-specific CAD (as returned by isChannelActive())
 * shows the channel is clear within the timeout period (or the timeout period is 0), else returns false.
 */
bool wait_CAD(void);

/*
 * Sets the Channel Activity Detection timeout in milliseconds to be used by waitCAD().
 * The default is 0, which means do not wait for CAD detection.
 */
void set_CAD_timeout(unsigned long cad_timeout);

/*
 * Blocks until the transmitter is no longer transmitting.
 * or until the timeout occurs, whichever happens first.
 * \param[in] timeout Maximum time to wait in milliseconds.
 * \return true if the radio completed transmission within the timeout period. 
 * False if it timed out.
 */
bool wait_packet_sent(uint16_t timeout);

#endif	/* RADIO_H */

