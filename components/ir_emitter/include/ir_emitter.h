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

void ir_emitter_setup(void);
void ir_emitter_nec(uint32_t value);
void ir_emitter_raw(uint16_t raw_signal[], uint16_t size);

#endif /** IR_EMITTER_H */