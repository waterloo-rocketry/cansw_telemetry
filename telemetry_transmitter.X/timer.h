#ifndef TIMER_H
#define	TIMER_H

#include <stdint.h>
#include "canlib/pic18f26k83/pic18f26k83_timer.h"

/*
 * Returns the number of microseconds since we woke up
 */
uint32_t micros(void);

#endif	/* TIMER_H */
