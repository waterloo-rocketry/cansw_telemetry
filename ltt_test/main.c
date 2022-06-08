#include <xc.h>
#include "canlib.h"

#include "device_config.h"
#include "platform.h"
#include "radio.h"
#include "uart.h"
#include "adc.h"

#define MAX_LOOP_TIME_DIFF_ms 500
// Time (in multiples of MAX_LOOP_ITME_DIFF_ms) between the bus down warning and power off
#define CYCLES_TILL_POWER_DOWN 40

#define BATT_WARNING_MV 9500
#define BATT_WARNING_MA 3000
#define BUS_WARNING_MA 900

#define HIGH_SPEED_MSG_DIVIDER 199 //Used to downsample sensor data, MUST BE PRIME
//lets about 1 in 800 messages of each type through, good if we are running around 1 kHz

static void can_msg_handler(const can_msg_t *msg);

//memory pool for the CAN tx buffer
uint8_t tx_pool[100];
uint8_t rx_pool[100];

// bus state
bool bus_powered = true;
bool req_bus_powered = true;
uint8_t ticks_until_power_down = 0;

int main(void) {
    // initialize the external oscillator
    oscillator_init();

    // init our millis() function
    timer0_init();

    // set up pins
    gpio_init();
    
    uart_init();
    
    adc_init();

    // Enable global interrupts
    INTCON0bits.GIE = 1;

    // Set up CAN TX
    TRISC0 = 0; // set as output
    RC0PPS = 0x33; // make C1 transmit CAN TX (page 267)

    // Set up CAN RX
    TRISC1 = 1; // set as input
    ANSELC1 = 0; // not analog
    CANRXPPS = 0x11; // make CAN read from C1 (page 264-265)

    // set up CAN module
    can_timing_t can_setup;
    can_generate_timing_params(_XTAL_FREQ * 4, &can_setup);
    can_init(&can_setup, can_msg_handler);
    // set up CAN buffers
    rcvb_init(rx_pool, sizeof(rx_pool));
    txb_init(tx_pool, sizeof(tx_pool), can_send, can_send_rdy);

    // loop timer
    uint32_t last_millis = millis();
    
    bool heartbeat = false;
    while (1) {
        if (millis() - last_millis > MAX_LOOP_TIME_DIFF_ms) {
            // update our loop counter
            last_millis = millis();

            // visual heartbeat indicator
            BLUE_LED_SET(heartbeat);
            heartbeat = !heartbeat;
            
            // current and voltage checks
            bool status_ok = true;
            uint16_t batt_volt = read_batt_volt_mv();
            can_msg_t msg;
            uint8_t data[2] = {0};
            build_analog_data_msg(millis(), SENSOR_ROCKET_BATT, batt_volt, &msg);
            txb_enqueue(&msg);
            if (batt_volt < BATT_WARNING_MV) {
                status_ok = false;
                data[0] = (batt_volt >> 8) & 0xff;
                data[1] = (batt_volt >> 0) & 0xff;
                build_board_stat_msg(millis(), E_BATT_UNDER_VOLTAGE, data, 2, &msg);
                txb_enqueue(&msg);
            }
            uint16_t batt_curr = read_batt_curr_ma();
            build_analog_data_msg(millis(), SENSOR_BATT_CURR, batt_curr, &msg);
            txb_enqueue(&msg);
            if (batt_curr > BATT_WARNING_MA) {
                status_ok = false;
                data[0] = (batt_curr >> 8) & 0xff;
                data[1] = (batt_curr >> 0) & 0xff;
                build_board_stat_msg(millis(), E_BATT_OVER_CURRENT, data, 2, &msg);
                txb_enqueue(&msg);
            }
            uint16_t bus_curr = read_bus_curr_ma();
            build_analog_data_msg(millis(), SENSOR_BUS_CURR, bus_curr, &msg);
            txb_enqueue(&msg);
            if (bus_curr > BUS_WARNING_MA) {
                status_ok = false;
                data[0] = (bus_curr >> 8) & 0xff;
                data[1] = (bus_curr >> 0) & 0xff;
                build_board_stat_msg(millis(), E_BUS_OVER_CURRENT, data, 2, &msg);
                txb_enqueue(&msg);
            }
            if (status_ok) {
                build_board_stat_msg(millis(), E_NOMINAL, NULL, 0, &msg);
                txb_enqueue(&msg);
            }
            
            // control bus power
            // first check if we just got asked to power down (ticks_until idles at zero)
            if (bus_powered && !req_bus_powered && ticks_until_power_down == 0) {
                // If so, send the power down warning and then start our countdown to power off
                can_msg_t msg;
                build_general_cmd_msg(millis(), BUS_DOWN_WARNING, &msg);
                txb_enqueue(&msg);
                ticks_until_power_down = CYCLES_TILL_POWER_DOWN;
            } else if (bus_powered && !req_bus_powered && ticks_until_power_down > 0) {
                // If we are currently counting down to a power off, keep counting
                ticks_until_power_down--;
                // and actually shut off power if we are done counting
                if (ticks_until_power_down == 0) {
                    bus_powered = false;
                }
            } else if (req_bus_powered) {
                // If we get were last asked to turn on just do it. Yes this will get called
                // every iteration we are powered, no it doesn't matter.
                bus_powered = true;
                ticks_until_power_down = 0;
            }
            SET_BUS_POWER(bus_powered);
        }
        
        while (uart_byte_available())
        {
            radio_handle_input_character(uart_read_byte());
        }
        
        if (!rcvb_is_empty())
        {
            can_msg_t msg;
            rcvb_pop_message(&msg);
            serialize_can_msg(&msg);
        }
        
        //send any queued CAN messages
        txb_heartbeat();
    }
}

uint8_t high_fq_data_counter = 0;
static void can_msg_handler(const can_msg_t *msg) {
    if (TXB0CONbits.TXERR) { // If the bus is down we will see tx errors
        return;
    }
    uint16_t msg_type = get_message_type(msg);
    
    // A little hacky, but convenient way to filter out the high speed stuff 
    //(not including altitude cause we need that)
    if (msg_type >= MSG_SENSOR_ACC && msg_type <= MSG_SENSOR_MAG){
        //while we want to discard most of the messages, we want to send them once in a while to alow checking that everything is alive
        // we use a prime number to avoid aliasing ensure that every message gets a chance to be sent
        if (!(high_fq_data_counter % HIGH_SPEED_MSG_DIVIDER)){
            rcvb_push_message(msg);
        }
        high_fq_data_counter++;
    }
    else{
        // Send the message over UART
        rcvb_push_message(msg);
    }
    
    // ignore messages that were sent from this board
    if (get_board_unique_id(msg) == BOARD_UNIQUE_ID) {
        return;
    }

    switch (msg_type) {
        case MSG_ACTUATOR_CMD:
            if (get_actuator_id(msg) == CANBUS) {
                req_bus_powered = get_req_actuator_state(msg) == ACTUATOR_CLOSED;
            }
            break;

        case MSG_LEDS_ON:
            RED_LED_SET(true);
            BLUE_LED_SET(true);
            WHITE_LED_SET(true);
            break;

        case MSG_LEDS_OFF:
            RED_LED_SET(false);
            BLUE_LED_SET(false);
            WHITE_LED_SET(false);
            break;

        // all the other ones - do nothing
        default:
            break;
    }
}

static void __interrupt() interrupt_handler(void) {
    if (PIR5) {
        can_handle_interrupt();
    }

    // Timer0 has overflowed - update millis() function
    // This happens approximately every 500us
    if (PIE3bits.TMR0IE == 1 && PIR3bits.TMR0IF == 1) {
        timer0_handle_interrupt();
        PIR3bits.TMR0IF = 0;
    } else if (PIR3 & 0x78) {
        //it's one of the UART1 interrupts, let them handle it
        uart_interrupt_handler();
        //that function is responsible for clearing PIR3
    }
}
