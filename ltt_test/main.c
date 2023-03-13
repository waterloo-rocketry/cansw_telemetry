#include <xc.h>
#include "canlib.h"

#include "device_config.h"
#include "platform.h"
#include "radio.h"
#include "uart.h"
#include "adc.h"

#define MAX_LOOP_TIME_DIFF_ms 500
#define BUS_DOWN_MAX_LOOP_TIME_DIFF_ms 3000
#define MAX_SENSOR_TIME_DIFF_ms 5
// Time (in multiples of MAX_LOOP_TIME_DIFF_ms) between the bus down warning and power off
#define CYCLES_TILL_POWER_DOWN 40

////#define BATT_WARNING_MV 9500
////#define BATT_WARNING_MA 3000
////#define BUS_WARNING_MA 900

#define HIGH_SPEED_MSG_DIVIDER 23 //Used to downsample sensor data, MUST BE PRIME
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
    rcvb_init(rx_pool, sizeof(rx_pool));
    txb_init(tx_pool, sizeof(tx_pool), can_send, can_send_rdy);

    // loop timer
    uint32_t last_millis = millis();
    uint32_t last_sensor_millis = millis();
    
    bool heartbeat = false;
    while (1) {
        if (millis() - last_millis > (bus_powered ? MAX_LOOP_TIME_DIFF_ms : BUS_DOWN_MAX_LOOP_TIME_DIFF_ms)) {
            // update our loop counter
            last_millis = millis();

            // visual heartbeat indicator
            BLUE_LED_SET(heartbeat);
            heartbeat = !heartbeat;
            
            // current checks
            can_msg_t msg;
            uint8_t data[2] = {0};

            uint16_t radio_curr = read_radio_curr_low_pass_ma();
            build_analog_data_msg(millis(), SENSOR_BATT_CURR, radio_curr, &msg);
            txb_enqueue(&msg);

            uint16_t bus_curr = read_bus_curr_low_pass_ma();
            build_analog_data_msg(millis(), SENSOR_BUS_CURR, bus_curr, &msg);
            txb_enqueue(&msg);
            
            build_board_stat_msg(millis(), E_NOMINAL, NULL, 0, &msg);
            txb_enqueue(&msg);
            
//            // control bus power
//            // first check if we just got asked to power down (ticks_until idles at zero)
//            if (bus_powered && !req_bus_powered && ticks_until_power_down == 0) {
//                // If so, send the power down warning and then start our countdown to power off
//                can_msg_t msg;
//                build_general_cmd_msg(millis(), BUS_DOWN_WARNING, &msg);
//                txb_enqueue(&msg);
//                ticks_until_power_down = CYCLES_TILL_POWER_DOWN;
//            } else if (bus_powered && !req_bus_powered && ticks_until_power_down > 0) {
//                // If we are currently counting down to a power off, keep counting
//                ticks_until_power_down--;
//                // and actually shut off power if we are done counting
//                if (ticks_until_power_down == 0) {
//                    bus_powered = false;
//                }
//            } else if (req_bus_powered) {
//                // If we get were last asked to turn on just do it. Yes this will get called
//                // every iteration we are powered, no it doesn't matter.
//                bus_powered = true;
//                ticks_until_power_down = 0;
//            }
//            SET_BUS_POWER(bus_powered);
        }
        
//        if (millis() - last_sensor_millis > MAX_SENSOR_TIME_DIFF_ms) {
//            update_sensor_low_pass();     // looks like it is a function regarding bus and batt
//        }
        
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
    uint16_t msg_type = get_message_type(msg);
    
    // A little hacky, but convenient way to filter out the high speed stuff 
    //(not including altitude cause we need that)
    if (msg_type >= MSG_SENSOR_ACC && msg_type <= MSG_SENSOR_MAG){
        //while we want to discard most of the messages, we want to send them once in a while to alow checking that everything is alive
        // we use a prime number to avoid aliasing ensure that every message gets a chance to be sent
        high_fq_data_counter++;
        if (high_fq_data_counter >= HIGH_SPEED_MSG_DIVIDER) {
            high_fq_data_counter = 0;
            rcvb_push_message(msg);
        }
    }
    else {
        // Don't send the message over uart if the bus is down and it's not from us
        if (get_board_unique_id(msg) == BOARD_UNIQUE_ID) {
            // Send the message over UART
            rcvb_push_message(msg);
        }
    }
    
    // ignore messages that were sent from this board
    if (get_board_unique_id(msg) == BOARD_UNIQUE_ID) {
        return;
    }
    
    int dest_id = -1;

    switch (msg_type) {
//        case MSG_ACTUATOR_CMD:
//            if (get_actuator_id(msg) == CANBUS) {
//                req_bus_powered = get_req_actuator_state(msg) == ACTUATOR_CLOSED;
//            }
//            break;

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
            if (dest_id == BOARD_UNIQUE_ID || dest_id == 0 ){
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
