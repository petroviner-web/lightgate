/*
 * Host-side unit tests for sprint_fsm. Build and run without any hardware:
 *   cl /nologo /W4 /I ..\include test_sprint_fsm.c ..\src\sprint_fsm.c
 *   test_sprint_fsm.exe
 */

#include <stdio.h>
#include "sprint_fsm.h"

static int failures = 0;

#define CHECK(cond)                                                    \
    do {                                                               \
        if (!(cond)) {                                                 \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);     \
            failures++;                                                \
        }                                                              \
    } while (0)

#define LOCKOUT_US 300000u

static void test_full_run_measures_elapsed_time(void)
{
    sprint_fsm_t f;
    sprint_init(&f, LOCKOUT_US);

    CHECK(!sprint_on_gate(&f, GATE_START, 1000000));
    CHECK(sprint_on_gate(&f, GATE_FINISH, 1000000 + 4250000));
    CHECK(sprint_elapsed_us(&f) == 4250000);
}

static void test_finish_before_start_is_ignored(void)
{
    sprint_fsm_t f;
    sprint_init(&f, LOCKOUT_US);

    CHECK(!sprint_on_gate(&f, GATE_FINISH, 5000000));
    CHECK(f.state == SPRINT_IDLE);
    CHECK(sprint_elapsed_us(&f) == 0);
}

static void test_bounce_within_lockout_is_ignored(void)
{
    sprint_fsm_t f;
    sprint_init(&f, LOCKOUT_US);

    CHECK(!sprint_on_gate(&f, GATE_START, 1000000));
    /* second start edge 50 ms later (bounce) must not restart the run */
    CHECK(!sprint_on_gate(&f, GATE_START, 1050000));
    CHECK(f.start_us == 1000000);
    CHECK(sprint_on_gate(&f, GATE_FINISH, 2000000));
    CHECK(sprint_elapsed_us(&f) == 1000000);
}

static void test_events_after_done_are_ignored_until_rearm(void)
{
    sprint_fsm_t f;
    sprint_init(&f, LOCKOUT_US);

    sprint_on_gate(&f, GATE_START, 1000000);
    CHECK(sprint_on_gate(&f, GATE_FINISH, 3000000));

    CHECK(!sprint_on_gate(&f, GATE_START, 10000000));
    CHECK(!sprint_on_gate(&f, GATE_FINISH, 11000000));
    CHECK(f.state == SPRINT_DONE);

    sprint_rearm(&f);
    CHECK(f.state == SPRINT_IDLE);
}

static void test_second_run_after_rearm(void)
{
    sprint_fsm_t f;
    sprint_init(&f, LOCKOUT_US);

    sprint_on_gate(&f, GATE_START, 1000000);
    sprint_on_gate(&f, GATE_FINISH, 2000000);
    sprint_rearm(&f);

    CHECK(!sprint_on_gate(&f, GATE_START, 10000000));
    CHECK(sprint_on_gate(&f, GATE_FINISH, 10500000));
    CHECK(sprint_elapsed_us(&f) == 500000);
}

int main(void)
{
    test_full_run_measures_elapsed_time();
    test_finish_before_start_is_ignored();
    test_bounce_within_lockout_is_ignored();
    test_events_after_done_are_ignored_until_rearm();
    test_second_run_after_rearm();

    if (failures == 0) {
        printf("OK: all sprint_fsm tests passed\n");
        return 0;
    }
    printf("%d check(s) failed\n", failures);
    return 1;
}
