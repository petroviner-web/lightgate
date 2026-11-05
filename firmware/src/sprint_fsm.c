#include "sprint_fsm.h"

void sprint_init(sprint_fsm_t *fsm, uint32_t lockout_us)
{
    fsm->state = SPRINT_IDLE;
    fsm->start_us = 0;
    fsm->finish_us = 0;
    fsm->last_accepted_us = 0;
    fsm->lockout_us = lockout_us;
    fsm->has_last = false;
}

static bool in_lockout(const sprint_fsm_t *fsm, uint64_t ts)
{
    return fsm->has_last && (ts - fsm->last_accepted_us) < fsm->lockout_us;
}

static void accept(sprint_fsm_t *fsm, uint64_t ts)
{
    fsm->last_accepted_us = ts;
    fsm->has_last = true;
}

bool sprint_on_gate(sprint_fsm_t *fsm, sprint_gate_t gate, uint64_t ts)
{
    if (in_lockout(fsm, ts)) {
        return false;
    }

    switch (fsm->state) {
    case SPRINT_IDLE:
        if (gate == GATE_START) {
            fsm->start_us = ts;
            fsm->state = SPRINT_RUNNING;
            accept(fsm, ts);
        }
        return false;

    case SPRINT_RUNNING:
        if (gate == GATE_FINISH) {
            fsm->finish_us = ts;
            fsm->state = SPRINT_DONE;
            accept(fsm, ts);
            return true;
        }
        return false;

    case SPRINT_DONE:
    default:
        return false;
    }
}

void sprint_rearm(sprint_fsm_t *fsm)
{
    fsm->state = SPRINT_IDLE;
}

uint64_t sprint_elapsed_us(const sprint_fsm_t *fsm)
{
    if (fsm->state != SPRINT_DONE) {
        return 0;
    }
    return fsm->finish_us - fsm->start_us;
}
