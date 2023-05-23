#ifndef RADIO_H
#define	RADIO_H

#include "canlib.h"

void radio_handle_input_character(char c);
void serialize_can_msg(can_msg_t *msg);

#endif