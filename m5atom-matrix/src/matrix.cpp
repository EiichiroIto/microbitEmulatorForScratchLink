#include <M5Unified.h>
#include <FastLED.h>
#include "matrix.h"

static CRGB Leds[MATRIX_NUM_LEDS];
const CRGB LedOn = CRGB(20,20,20);
const CRGB LedOff = CRGB::Black;
static bool Scrolling = false;
static unsigned long scroll_msec = 0;

static void clear();
static void shiftLeft(uint32_t font, int count);
static void shiftRight(uint32_t font, int count);
static void setupScroll(const char *str);
static void generateFrame();
static bool stepNext();
static void matrix_debug();

void init_matrix()
{
  FastLED.addLeds<NEOPIXEL, MATRIX_PIN>(Leds, MATRIX_NUM_LEDS);
  clear();
}

void matrix_clear()
{
  clear();
  FastLED.show();
}

void matrix_show(uint32_t value)
{
  for (int i = 0, mask = 1 << 24; i < MATRIX_NUM_LEDS; i ++, mask >>= 1) {
    if (value & mask) {
      Leds[i] = LedOn;
    } else {
      Leds[i] = LedOff;
    }
  }
  FastLED.show();
}

void matrix_scroll(const char *str)
{
  if (!*str) {
    Scrolling = false;
    matrix_clear();
    return;
  }
  setupScroll(str);
  scroll_msec = 0;
  matrix_scroll_step();
}

void matrix_scroll_step()
{
  if (!Scrolling) {
    return;
  }
  unsigned long now = millis();
  if (now - scroll_msec < scroll_interval) {
    return;
  }
  scroll_msec = now;
  generateFrame();
  FastLED.show();
  //matrix_debug();
  Scrolling = stepNext();
}

static uint32_t Fonts[] =  {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 8658952, 10813440, 11512810, 15514222, 26808627, 13185613, 8650752, 4464900, 8523912, 332096, 145536, 136, 14336, 256, 1118480,
  13191756, 4591758, 29438494, 31527500, 6638562, 33060926, 2243118, 32575760, 15252014, 15251592, 262400, 131208, 2236546, 459200, 8521864, 15243268,
  0, 13204050, 29979228, 15221262, 29968988, 32010782, 32010768, 15224366, 19495506, 29630748, 32574028, 19554962, 17318430, 18732593, 18667121, 13191756,
  29979152, 13191558, 29979217, 15216732, 32641156, 19483212, 18400580, 18405233, 19477074, 18157700, 31597086, 0, 17043521, 0, 4521984, 31,
  8519680, 477775, 17330780, 475662, 2177614, 13201934, 6582536, 15284300, 17330770, 8397064, 2099276, 17457810, 8659206, 906801, 936530, 412236,
  936848, 477634, 475664, 204952, 8665351, 608847, 574788, 575163, 602514, 567448, 987422, 0, 8659208, 0, 12384, 0};

static char scroll_text[SCROLL_MAXLEN] = "";
static const char *scroll_str = NULL;
static char next = 0;
static int scroll_index = 0;
static uint32_t cur_font = 0;
static uint32_t next_font = 0;

static void clear()
{
  for (int i = 0; i < MATRIX_NUM_LEDS; i ++) {
    Leds[i] = LedOff;
  }
}

static void shiftLeft(uint32_t font, int count)
{
  uint32_t rmask0, rmask;
	rmask0 = 1 << (24 - count);
  for (int x = 0; x < 5 - count; x ++) {
    rmask = rmask0;
    for (int y = 0; y < 5; y ++) {
      if (font & rmask) {
        Leds[x + y * 5] = LedOn;
      }
      rmask >>= 5;
    }
    rmask0 >>= 1;
  }
}

static void shiftRight(uint32_t font, int count)
{
  uint32_t rmask0, rmask;
	rmask0 = 1 << 24;
  for (int x = count; x < 5; x ++) {
    rmask = rmask0;
    for (int y = 0; y < 5; y ++) {
      if (font & rmask) {
        Leds[x + y * 5] = LedOn;
      }
      rmask >>= 5;
    }
    rmask0 >>= 1;
  }
}

static void setupScroll(const char *str)
{
  strncpy(scroll_text, str, sizeof scroll_text);
  scroll_str = scroll_text;
  cur_font = 0;
  next = *scroll_str++;
  next_font = Fonts[next];
  scroll_index = 0;
  Scrolling = true;
  //Serial.println(cur_font);
  //Serial.println(next_font);
}

static void generateFrame()
{
  clear();
  if (scroll_index == 5) {
    shiftLeft(next_font, 1);
  } else {
    shiftLeft(cur_font, 2 + scroll_index);
    shiftRight(next_font, 4 - scroll_index);
  }
}

static bool stepNext()
{
  if (scroll_index < 5) {
    scroll_index ++;
    return true;
  }
  scroll_index = 0;
  if (!next) {
    return false;
  }
  cur_font = next_font;
  next = *scroll_str++;
  next_font = Fonts[next];
  return true;
}

static void matrix_debug()
{
  for (int i = 0; i < MATRIX_NUM_LEDS; i ++) {
    if (Leds[i] == LedOff) {
      Serial.print("0");
    } else {
      Serial.print("1");
    }
  }
  Serial.println();
}