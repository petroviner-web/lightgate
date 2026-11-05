#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/sync.h"

#include "hal_pico.h"
#include "sprint_fsm.h"

#define PIN_GATE_START   2u
#define PIN_GATE_FINISH  3u
#define LOCKOUT_US       300000u   /* ignore re-triggers within 300 ms */
#define REARM_DELAY_MS   2000u

static sprint_fsm_t fsm;

/* Written in the IRQ, read in main(). */
static volatile bool     g_result_ready = false;
static volatile uint64_t g_start_us;
static volatile uint64_t g_finish_us;

/* Runs in interrupt context: no printf here, only state updates. */
static void on_gate(sprint_gate_t gate, uint64_t ts_us)
{
    if (sprint_on_gate(&fsm, gate, ts_us)) {
        g_start_us = fsm.start_us;
        g_finish_us = fsm.finish_us;
        g_result_ready = true;
    }
}

int main(void)
{
    stdio_init_all();
    sprint_init(&fsm, LOCKOUT_US);
    hal_gates_init(PIN_GATE_START, PIN_GATE_FINISH, on_gate);

    printf("READY\n");

    for (;;) {
        if (g_result_ready) {
            g_result_ready = false;

            /* Protocol line parsed by the backend: RUN;<start_us>;<finish_us>;<elapsed_us> */
            const uint64_t start = g_start_us;
            const uint64_t finish = g_finish_us;
            printf("RUN;%llu;%llu;%llu\n",
                   (unsigned long long)start,
                   (unsigned long long)finish,
                   (unsigned long long)(finish - start));

            sleep_ms(REARM_DELAY_MS);

            uint32_t irq_state = save_and_disable_interrupts();
            sprint_rearm(&fsm);
            restore_interrupts(irq_state);

            printf("ARMED\n");
        }
        tight_loop_contents();
    }
}
