#include <M5Unified.h>
#include "avatar.h"
#include "scratchlink.h"

#define ScratchLinkName "avatar"

void setup()
{
  delay(100);
  M5.begin();
  Serial.begin(115200);
  Serial.println("Waiting 3 seconds...");
  delay(3000);

  M5.setLogDisplayIndex(0);
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_INFO);
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_INFO);
  M5.Log.setEnableColor(m5::log_target_serial, false);
  M5_LOGI("Initializing ...");

  init_scratchlink(ScratchLinkName);

  M5_LOGI("Starting ...");
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_NONE);
  delay(2000);

  init_avatar();
}

unsigned long scratchlink_msec = 0;
bool button_pressed = false;

void loop()
{
  M5.update();
  button_pressed |= M5.BtnA.isPressed();

  unsigned long now = millis();
  if (now - scratchlink_msec > 1000) {
    scratchlink_msec = now;
    scratchlink_update(0, 0, button_pressed, false, false, false, false, 0);
    button_pressed = false;
  }
  delay(10);
}

void scratchlink_callback(byte command, byte *payload, unsigned int length)
{
  M5_LOGD("ScratchLink: Command=%d, Payload=%s", command, payload);
  if (command == ScratchLink_Say) {
    M5_LOGI("say received");
    String text;
    for (int i = 0; i < length; i ++) {
      text += (char) payload[i];
    }
    text += '\0';
    avatar_say(text.c_str());
  } else if (command == ScratchLink_Led) {
    int value = 0;
    value |= (int) payload[4];
    value |= (int) payload[3] << 5;
    value |= (int) payload[2] << 10;
    value |= (int) payload[1] << 15;
    value |= (int) payload[0] << 20;
    M5_LOGI("led received=%X", value);
    int face = Expression_Neutral;
    if (value == 0x11503F5) {
      face = Expression_Angry;
    }
    if (value == 0xD81C0) {
      face = Expression_Sleepy;
    }
    if (value == 0x5022E) {
      face = Expression_Happy;
    }
    if (value == 0x501D1) {
      face = Expression_Sad;
    }
    if (value == 0x50155) {
      face = Expression_Doubt;
    }
    avatar_expression(face);
  }
}
