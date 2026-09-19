#ifndef __MATRIX__
#define __MATRIX__

#define MATRIX_PIN 27
#define MATRIX_NUM_LEDS 25
#define SCROLL_MAXLEN 256

const int scroll_interval = 100;

const int matrix_angry = 0x11503F5;
const int matrix_sleepy = 0xD81C0;
const int matrix_happy = 0x5022E;
const int matrix_sad = 0x501D1;
const int matrix_doubt = 0x50155;

extern void init_matrix();
extern void matrix_clear();
extern void matrix_show(uint32_t value);
void matrix_scroll(const char *str);
void matrix_scroll_step();

#endif
