#include "ir_emitter.h"

#include <stdint.h>
#include <stdbool.h>

#include "driver/ledc.h"
#include "driver/timer.h"

/** IR LED GPIO */
#define IR_GPIO                 25

#define IR_PWM_TIMER            LEDC_TIMER_1
#define IR_PWM_CH               LEDC_CHANNEL_0

/** Modulation frequency */
#define IR_CARRIER_FREQ         38000

/** Controller timer (frequency = 80MHz / 800) */
#define IR_CTL_TIMER_GROUP      TIMER_GROUP_0
#define IR_CTL_TIMER_IDX        TIMER_0
#define IR_CTL_TIMER_DIVIDER    800

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
