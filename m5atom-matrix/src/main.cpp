#include <M5Unified.h>
#include "scratchlink.h"
#include "matrix.h"

#define ScratchLinkName "matrix"

void setup()
{
  delay(100);
  M5.begin();
  Serial.begin(115200);
  Serial.println("Waiting 3 seconds...");
  delay(3000);

  M5.setLogDisplayIndex(0);
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_INFO);
  M5.Log.setEnableColor(m5::log_target_serial, false);
  M5_LOGI("Initializing ...");

  init_scratchlink(ScratchLinkName);
  init_matrix();

  M5_LOGI("Starting ...");
  matrix_scroll("Start");
}

unsigned long scratchlink_msec = 0;
bool button_pressed = false;
m5::imu_data_t imuData;

void loop()
{
  M5.update();
  button_pressed |= M5.BtnA.isPressed();

  unsigned long now = millis();
  if (now - scratchlink_msec > 1000) {
    scratchlink_msec = now;
    M5.Imu.update();
    imuData = M5.Imu.getImuData();
    float v;
    uint16_t accx, accy;
    v = imuData.accel.x;
    v = v < -1.0f ? -1.0f : v;
    v = v > 1.0f ? 1.0f : v;
    v *= 32767.0f;
    if (v < 0) {
      accx = (uint16_t) (v + 65536);
    } else {
      accx = (uint16_t) v;
    }
    v = imuData.accel.y;
    v = v < -1.0f ? -1.0f : v;
    v = v > 1.0f ? 1.0f : v;
    v *= 32767.0f;
    if (v < 0) {
      accy = (uint16_t) (v + 65536);
    } else {
      accy = (uint16_t) v;
    }
    scratchlink_update(accx, accy, button_pressed, false, false, false, false, 0);
    button_pressed = false;
  }
  matrix_scroll_step();
  delay(10);
}

void scratchlink_callback(byte command, byte *payload, unsigned int length)
{
  M5_LOGD("ScratchLink: Command=%d, Payload=%s", command, payload);
  if (command == ScratchLink_Say) {
    String text;
    for (int i = 0; i < length; i ++) {
      text += (char) payload[i];
    }
    text += '\0';
    matrix_scroll(text.c_str());
    M5_LOGI("say received");
  } else if (command == ScratchLink_Led) {
    uint32_t value = 0;
    value |= (int) payload[4];
    value |= (int) payload[3] << 5;
    value |= (int) payload[2] << 10;
    value |= (int) payload[1] << 15;
    value |= (int) payload[0] << 20;
    matrix_show(value);
    M5_LOGI("led received=%X", value);
  }
}
