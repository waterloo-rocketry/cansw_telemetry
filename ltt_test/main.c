#include <xc.h>
#include "canlib.h"

#include "device_config.h"
#include "platform.h"
#include "radio.h"
#include "uart.h"
#include "adc.h"

#define MAX_LOOP_TIME_DIFF_ms 500
#define MAX_BUS_DEAD_TIME_ms 1000
#define MAX_SENSOR_TIME_DIFF_ms 5
// reset radio, 1 minute after the last message received from radio
#define RESET_RADIO_ms 60000

#define HIGH_SPEED_MSG_DIVIDER 23 //Used to downsample sensor data, MUST BE PRIME

static void can_msg_handler(const can_msg_t *msg);

//memory pool for the CAN tx buffer
uint8_t tx_pool[100];
uint8_t rx_pool[100];

uint32_t last_bus_message_millis = 0;
uint32_t last_radio_message_millis = 0;

int main(void) {
    // initialize the external oscillator
    oscillator_init();

    // init our millis() function
    timer0_init();

    // set up pins
    gpio_init();

    uart_init();

    adc_init();

    // power on radio
    SET_RADIO_POWER(1);

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
    rcvb_init(rx_pool, sizeof (rx_pool));
    txb_init(tx_pool, sizeof (tx_pool), can_send, can_send_rdy);

    // loop timer
    uint32_t last_millis = millis();
    uint32_t last_sensor_millis = millis();

    bool heartbeat = false;

    while (1) {
        // clear watchdog timer
        CLRWDT();
        
        if (OSCCON2 != 0x70) { // If the fail-safe clock monitor has triggered
            oscillator_init();
        }
        
        uint32_t dt = millis() - last_bus_message_millis;
        // prevent race condition where last_message_millis is greater than millis
        // by checking for overflow
        if (dt > MAX_BUS_DEAD_TIME_ms && dt < (1 << 15)) {
            // We've got too long without seeing a valid CAN message (including one of ours)
            RESET();
        }
        
        if (millis() - last_millis > MAX_LOOP_TIME_DIFF_ms) {
            // update our loop counter
            last_millis = millis();

            // visual heartbeat indicator
            BLUE_LED_SET(heartbeat);
            heartbeat = !heartbeat;

            // radio current checks
            can_msg_t msg;

            uint16_t radio_curr = read_radio_curr_ma();
            build_analog_data_msg(millis(), SENSOR_RADIO_CURR, radio_curr, &msg);
            txb_enqueue(&msg);

            build_board_stat_msg(millis(), E_NOMINAL, NULL, 0, &msg);
            txb_enqueue(&msg);
        }

        if (millis() - last_sensor_millis > MAX_SENSOR_TIME_DIFF_ms) {
            update_sensor_low_pass();
        }
        
        while (uart_byte_available()) {
            radio_handle_input_character(uart_read_byte());
            last_radio_message_millis = millis();
        }
        
        // reset radio if no message received for over 1 minute
        if (millis() - last_radio_message_millis > RESET_RADIO_ms){
            // reset radio
            SET_RADIO_POWER(0);
            uint32_t wait = millis();
            // send error msg
            can_msg_t msg;
            uint8_t error_data[2] = {0,0};
            build_board_stat_msg(millis(), E_RADIO_SIGNAL_LOST, error_data, 2, &msg);
            txb_enqueue(&msg);
            // wait for 50 ms
            while (millis() - wait < 50);    
            SET_RADIO_POWER(1);
            last_radio_message_millis = millis();
        }

        if (!rcvb_is_empty()) {
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
    last_bus_message_millis = millis();
    uint16_t msg_type = get_message_type(msg);

    // A little hacky, but convenient way to filter out the high speed stuff 
    //(not including altitude cause we need that)
    if (msg_type >= MSG_SENSOR_ACC && msg_type <= MSG_SENSOR_MAG) {
        //while we want to discard most of the messages, we want to send them once in a while to alow checking that everything is alive
        // we use a prime number to avoid aliasing ensure that every message gets a chance to be sent
        high_fq_data_counter++;
        if (high_fq_data_counter >= HIGH_SPEED_MSG_DIVIDER) {
            high_fq_data_counter = 0;
            rcvb_push_message(msg);
        }
    } else {
        rcvb_push_message(msg);
    }

    // ignore messages that were sent from this board
    if (get_board_unique_id(msg) == BOARD_UNIQUE_ID) {
        return;
    }

    int dest_id = -1;

    switch (msg_type) {
        case MSG_ACTUATOR_CMD:
            if (get_actuator_id(msg) == ACTUATOR_RADIO) {
                if (get_req_actuator_state(msg) == ACTUATOR_OFF) {
                    SET_RADIO_POWER(false);
                }
                else if (get_req_actuator_state(msg) == ACTUATOR_ON) {
                    last_radio_message_millis = millis();
                    SET_RADIO_POWER(true);
                }
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

        case MSG_RESET_CMD:
            dest_id = get_reset_board_id(msg);
            if (dest_id == BOARD_UNIQUE_ID || dest_id == 0) {
                RESET();
            }
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
