#include <M5Unified.h>
#include "imu.h"

m5::imu_data_t imuData;

static float accx_center = 0;
static float accy_center = 0;
static float accz_center = 0;

void init_imu()
{
  M5.Imu.begin();
}

void imu_getaccel(uint16_t *paccx, uint16_t *paccy, uint16_t *paccz )
{
  M5.Imu.update();
  imuData = M5.Imu.getImuData();
  float v;

  if (paccx) {
    v = imuData.accel.x - accx_center;
    v = v < -1.0f ? -1.0f : v;
    v = v > 1.0f ? 1.0f : v;
    v *= 32767.0f;
    if (v < 0) {
      *paccx = (uint16_t) (v + 65536);
    } else {
      *paccx = (uint16_t) v;
    }
  }
  if (paccy) {
    v = imuData.accel.y - accy_center;
    v = v < -1.0f ? -1.0f : v;
    v = v > 1.0f ? 1.0f : v;
    v *= 32767.0f;
    if (v < 0) {
      *paccy = (uint16_t) (v + 65536);
    } else {
      *paccy = (uint16_t) v;
    }
  }
  if (paccz) {
    v = imuData.accel.z - accz_center;
    v = v < -1.0f ? -1.0f : v;
    v = v > 1.0f ? 1.0f : v;
    v *= 32767.0f;
    if (v < 0) {
      *paccz = (uint16_t) (v + 65536);
    } else {
      *paccz = (uint16_t) v;
    }
  }
}

void imu_calibrate()
{
  float accx, accy, accz;

  accx = accy = accz = 0.0f;
  for (int i = 0; i < 100; i ++) {
    M5.Imu.update();
    imuData = M5.Imu.getImuData();
    accx += imuData.accel.x;
    accy += imuData.accel.y;
    accz += imuData.accel.z;
    delay(10);
  }
  accx_center = accx / 100.0f;
  accy_center = accy / 100.0f;
  accz_center = accz / 100.0f;
}
