#ifndef IR_EMITTER_H
#define IR_EMITTER_H

#include <stdint.h>

/** Samsung NEC codes */
#define IR_NEC_SAMSUNG_ENTER    0xE0E016E9
#define IR_NEC_SAMSUNG_ESC      0xE0E01AE5
#define IR_NEC_SAMSUNG_UP       0xE0E006F9
#define IR_NEC_SAMSUNG_DOWN     0xE0E08679
#define IR_NEC_SAMSUNG_LEFT     0xE0E0A659
#define IR_NEC_SAMSUNG_RIGHT    0xE0E046B9

/** SKY codes */
#define IR_SKY_ON       0x0701
#define IR_SKY_OFF      0x0F81
#define IR_SKY_GUIDE    0x0014
#define IR_SKY_LIST     0x0454
#define IR_SKY_EXIT     0x0F64
#define IR_SKY_UP       0x0D84
#define IR_SKY_LEFT     0x0BC4
#define IR_SKY_RIGHT    0x0B24
#define IR_SKY_DOWN     0x0344
#define IR_SKY_CONFIRM  0x07A4
#define IR_SKY_MENU     0x07A4
#define IR_SKY_LAST_CH  0x03F0
#define IR_SKY_CH_UP    0x05B0
#define IR_SKY_CH_DOWN  0x0D70
#define IR_SKY_DASH     0x0E48
#define IR_SKY_ENTER    0x01C8
#define IR_SKY_1        0x0880
#define IR_SKY_2        0x0440
#define IR_SKY_3        0x0CC0
#define IR_SKY_4        0x0C20
#define IR_SKY_5        0x02A0
#define IR_SKY_6        0x0A60
#define IR_SKY_7        0x06E0
#define IR_SKY_8        0x0610
#define IR_SKY_9        0x0E90
#define IR_SKY_0        0x0688

void ir_emitter_setup(void);
void ir_emitter_nec(uint32_t value);
void ir_emitter_sky(uint16_t value);
void ir_emitter_raw(const uint16_t raw_signal[], uint16_t size);

#endif /** IR_EMITTER_H */