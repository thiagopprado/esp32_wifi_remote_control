#include "ir_emitter.h"

#include <stdbool.h>
#include <string.h>

// ESP
#include "esp_err.h"
#include "esp_log.h"

#include "driver/ledc.h"
#include "driver/timer.h"

/** IR LED GPIO */
#define IR_GPIO                 15

#define IR_PWM_TIMER            LEDC_TIMER_1
#define IR_PWM_CH               LEDC_CHANNEL_0

/** Modulation frequency */
#define IR_CARRIER_FREQ         38000

/** Controller timer (period = 800 / 80MHz = 10us) */
#define IR_CTL_TIMER_GROUP      TIMER_GROUP_0
#define IR_CTL_TIMER_IDX        TIMER_0
#define IR_CTL_TIMER_DIVIDER    800

/** NEC timings (in multiples of 10us)*/
#define IR_NEC_START_HIGH_TIME  450
#define IR_NEC_START_LOW_TIME   450
#define IR_NEC_BIT_HIGH_TIME    56
#define IR_NEC_BIT_0_LOW_TIME   56
#define IR_NEC_BIT_1_LOW_TIME   169

/** SKY timings (in multiples of 10us)*/
#define IR_SKY_START_TIME       600
#define IR_SKY_BIT_0_TIME       60
#define IR_SKY_BIT_1_TIME       120

#define IR_SIGNAL_MAX_SIZE          256

typedef struct {
    bool in_progress;
    bool state;
    uint16_t index;
    uint16_t timer_counter;
    uint16_t size;
} ir_trx_t;

static ir_trx_t ir_trx = { 0 };
static uint16_t ir_signal[IR_SIGNAL_MAX_SIZE] = { 0 };

static bool timer_callback(void* arg)
{
    if (ir_trx.timer_counter == 0) {
        if (ir_trx.state == true) {
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, IR_PWM_CH, 50);
        } else {
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, IR_PWM_CH, 0);
        }

        ledc_update_duty(LEDC_HIGH_SPEED_MODE, IR_PWM_CH);

        ir_trx.state = !ir_trx.state;
    }

    if (ir_trx.timer_counter == ir_signal[ir_trx.index]) {
        ir_trx.timer_counter = 0;
        ir_trx.index++;

        if (ir_trx.index >= ir_trx.size) {
            ir_trx.in_progress = false;

            ledc_set_duty(LEDC_HIGH_SPEED_MODE, IR_PWM_CH, 0);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, IR_PWM_CH);

            timer_pause(IR_CTL_TIMER_GROUP, IR_CTL_TIMER_IDX);
        }
    } else {
        ir_trx.timer_counter++;
    }

    return false;
}

void ir_emitter_setup(void) {
    // PWM
    ledc_timer_config_t pwm_timer_config = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = IR_PWM_TIMER,
        .freq_hz = IR_CARRIER_FREQ,
        .clk_cfg = LEDC_USE_APB_CLK,
    };

    ledc_timer_config(&pwm_timer_config);

    ledc_channel_config_t ch_config = {
        .gpio_num = IR_GPIO,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = IR_PWM_CH,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = IR_PWM_TIMER,
        .duty = 0,
    };

    ledc_channel_config(&ch_config);

    // IR controller timer
    timer_config_t timer_config = {
        .alarm_en = TIMER_ALARM_EN,
        .counter_en = TIMER_PAUSE,
        .intr_type = TIMER_INTR_LEVEL,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = TIMER_AUTORELOAD_EN,
        .divider = IR_CTL_TIMER_DIVIDER,
    };

    timer_init(IR_CTL_TIMER_GROUP, IR_CTL_TIMER_IDX, &timer_config);
    timer_set_counter_value(IR_CTL_TIMER_GROUP, IR_CTL_TIMER_IDX, 0);
    timer_set_alarm_value(IR_CTL_TIMER_GROUP, IR_CTL_TIMER_IDX, 1);
    timer_enable_intr(IR_CTL_TIMER_GROUP, IR_CTL_TIMER_IDX);
    timer_isr_callback_add(IR_CTL_TIMER_GROUP, IR_CTL_TIMER_IDX, timer_callback, NULL, 0);
}

void ir_emitter_nec(uint32_t value)
{
    if (ir_trx.in_progress == true) {
        return;
    }

    timer_pause(IR_CTL_TIMER_GROUP, IR_CTL_TIMER_IDX);

    ir_trx.state = true;
    ir_trx.index = 0;
    ir_trx.size = 0;
    ir_trx.timer_counter = 0;

    // Start
    ir_signal[ir_trx.size++] = IR_NEC_START_HIGH_TIME;
    ir_signal[ir_trx.size++] = IR_NEC_START_LOW_TIME;

    // Bits
    for (int i = 31; i >= 0; i--) {
        ir_signal[ir_trx.size++] = IR_NEC_BIT_HIGH_TIME;

        if ((value & (1 << i)) == 0) {
            // 0
            ir_signal[ir_trx.size++] = IR_NEC_BIT_0_LOW_TIME;
        } else {
            // 1
            ir_signal[ir_trx.size++] = IR_NEC_BIT_1_LOW_TIME;
        }
    }

    // End
    ir_signal[ir_trx.size++] = IR_NEC_BIT_HIGH_TIME;
    ir_signal[ir_trx.size++] = IR_NEC_BIT_0_LOW_TIME;

    ir_trx.in_progress = true;
    timer_start(IR_CTL_TIMER_GROUP, IR_CTL_TIMER_IDX);
}

void ir_emitter_sky(uint16_t value)
{
    if (ir_trx.in_progress == true) {
        return;
    }

    timer_pause(IR_CTL_TIMER_GROUP, IR_CTL_TIMER_IDX);

    ir_trx.state = true;
    ir_trx.index = 0;
    ir_trx.size = 0;
    ir_trx.timer_counter = 0;

    ir_signal[ir_trx.size++] = IR_SKY_START_TIME;

    ir_signal[ir_trx.size++] = IR_SKY_BIT_1_TIME;
    ir_signal[ir_trx.size++] = IR_SKY_BIT_1_TIME;
    ir_signal[ir_trx.size++] = IR_SKY_BIT_1_TIME;
    ir_signal[ir_trx.size++] = IR_SKY_BIT_0_TIME;
    ir_signal[ir_trx.size++] = IR_SKY_BIT_0_TIME;

    for (int i = 0; i < 12; i++) {
        if ((value & (1 << i)) != 0) {
            ir_signal[ir_trx.size++] = IR_SKY_BIT_1_TIME;
        } else {
            ir_signal[ir_trx.size++] = IR_SKY_BIT_0_TIME;
        }
    }

    ir_signal[ir_trx.size++] = IR_SKY_BIT_0_TIME;

    ir_trx.in_progress = true;
    timer_start(IR_CTL_TIMER_GROUP, IR_CTL_TIMER_IDX);

}

void ir_emitter_raw(const uint16_t raw_signal[], uint16_t size)
{
    if (ir_trx.in_progress == true) {
        return;
    }

    timer_pause(IR_CTL_TIMER_GROUP, IR_CTL_TIMER_IDX);

    ir_trx.state = true;
    ir_trx.index = 0;
    ir_trx.size = 0;
    ir_trx.timer_counter = 0;

    memcpy(ir_signal, raw_signal, size * sizeof(uint16_t));
    ir_trx.size = size;

    ir_trx.in_progress = true;
    timer_start(IR_CTL_TIMER_GROUP, IR_CTL_TIMER_IDX);
}
