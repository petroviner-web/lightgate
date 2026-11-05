#ifndef SPRINT_FSM_H
#define SPRINT_FSM_H

/*
 * Portable state machine for a two-gate sprint timer.
 * Contains no hardware code, so it can be unit-tested on a PC.
 */

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    GATE_START = 0,   /* runner breaks the start light gate */
    GATE_FINISH = 1   /* runner breaks the finish light gate */
} sprint_gate_t;

typedef enum {
    SPRINT_IDLE,      /* waiting for the start gate */
    SPRINT_RUNNING,   /* run in progress, waiting for the finish gate */
    SPRINT_DONE       /* result is ready, waiting for sprint_rearm() */
} sprint_state_t;

typedef struct {
    sprint_state_t state;
    uint64_t start_us;
    uint64_t finish_us;
    uint64_t last_accepted_us;
    uint32_t lockout_us;   /* ignore new events this long after an accepted one */
    bool has_last;
} sprint_fsm_t;

void sprint_init(sprint_fsm_t *fsm, uint32_t lockout_us);

/* Returns true exactly when a run completes (finish gate accepted). */
bool sprint_on_gate(sprint_fsm_t *fsm, sprint_gate_t gate, uint64_t timestamp_us);

/* Goes back to IDLE. The lockout window is kept, so bounce can't restart a run. */
void sprint_rearm(sprint_fsm_t *fsm);

/* Elapsed time of the last completed run, or 0 if there is none. */
uint64_t sprint_elapsed_us(const sprint_fsm_t *fsm);

#endif /* SPRINT_FSM_H */
