#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "mcc_generated_files/mcc.h"

#include "analog.h"
#include "uart.h"
#include "canlib/pic18f26k83/pic18f26k83_can.h"
#include "canlib/util/can_rcv_buffer.h"
#include "canlib/can_common.h"
#include "canlib/util/timing_util.h"
#include "canlib/util/can_tx_buffer.h"
#include "canlib/pic18f26k83/pic18f26k83_timer.h"
#include "sotscon.h"
#include "sotscon_sender.h"
#include "radio_handler.h"
#include "bus_power.h"
#include "timer.h"
#include "led_manager.h"
#include "setup.h"

void can_message_callback(const can_msg_t *msg)
{
    uint16_t message_type = get_message_type(msg);
    if (message_type == MSG_LEDS_ON)
    {
        WHITE_LED_ON();
        RED_LED_ON();
        BLUE_LED_ON();
    }
    else if (message_type == MSG_LEDS_OFF)
    {
        WHITE_LED_OFF();
        RED_LED_OFF();
        BLUE_LED_OFF();
    }

    rcvb_push_message(msg);
}

// enough space to buffer 10 CAN messages
uint8_t can_receive_buffer[140];
uint8_t can_transmit_buffer[140];

int main()
{
    // initialization functions
    SYSTEM_Initialize();
    init_leds();
    WHITE_LED_ON();
    init_pins();
    RED_LED_ON();
    init_adc();
    timer0_init();
    init_interrupts();
    init_uart();
    BLUE_LED_ON();
    init_sotscon();
    rcvb_init(can_receive_buffer, sizeof(can_receive_buffer));
    txb_init(can_transmit_buffer, sizeof(can_transmit_buffer), &can_send, &can_send_rdy);
    init_led_manager();

    WHITE_LED_OFF();
    RED_LED_OFF();
    BLUE_LED_OFF();

    can_timing_t timing;
    can_generate_timing_params(_XTAL_FREQ, &timing);
    can_init(&timing, &can_message_callback);

    BLUE_LED_OFF();

    trigger_bus_powerup();

    // program loop
    while (1)
    {
        if (uart_byte_available())
        {
            radio_handle_input_character(uart_read_byte());
        }

        // We check for CAN messages regardless of whether the bus is powered.
        // It's possible that the debug board is trying to tell us something,
        // and we should really listen to that
        if (!rcvb_is_empty())
        {
            can_msg_t msg;
            rcvb_pop_message(&msg);
            handle_incoming_can_message(&msg);
        }

        if (is_bus_powered())
        {
            // There's no sense in sending CAN messages if the bus isn't
            // powered. There's no one to hear them
            sotscon_heartbeat();
        }
        else
        {
            // TODO, what should the radio board do while the bus is powered
            // down? The ADC stuff I guess?
        }

        bus_power_heartbeat();
        analog_heartbeat();
        txb_heartbeat();
        radio_heartbeat();
        led_manager_heartbeat();
        __delay_ms(10);
    }

    // unreachable
    return 0;
}
