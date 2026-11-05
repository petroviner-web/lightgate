#include "hal_pico.h"

#include "hardware/gpio.h"
#include "pico/time.h"

static hal_gate_cb_t s_cb = 0;
static unsigned int s_start_pin;
static unsigned int s_finish_pin;

/* The Pico SDK allows one IRQ callback per core, so both pins share this handler. */
static void gpio_irq_handler(unsigned int gpio, uint32_t events)
{
    (void)events;
    const uint64_t ts = time_us_64();   /* capture the timestamp first */

    if (s_cb == 0) {
        return;
    }
    if (gpio == s_start_pin) {
        s_cb(GATE_START, ts);
    } else if (gpio == s_finish_pin) {
        s_cb(GATE_FINISH, ts);
    }
}

static void configure_input(unsigned int pin)
{
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);   /* sensor output pulls the line LOW when the beam is broken */
}

void hal_gates_init(unsigned int start_pin, unsigned int finish_pin, hal_gate_cb_t cb)
{
    s_cb = cb;
    s_start_pin = start_pin;
    s_finish_pin = finish_pin;

    configure_input(start_pin);
    configure_input(finish_pin);

    gpio_set_irq_enabled_with_callback(start_pin, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(finish_pin, GPIO_IRQ_EDGE_FALL, true);
}
