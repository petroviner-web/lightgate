#ifndef HAL_PICO_H
#define HAL_PICO_H

/*
 * Hardware layer for Raspberry Pi Pico (RP2040, ARM Cortex-M0+).
 * Turns falling edges on two GPIO pins into timestamped gate events.
 */

#include "sprint_fsm.h"

typedef void (*hal_gate_cb_t)(sprint_gate_t gate, uint64_t timestamp_us);

void hal_gates_init(unsigned int start_pin, unsigned int finish_pin, hal_gate_cb_t cb);

#endif /* HAL_PICO_H */
