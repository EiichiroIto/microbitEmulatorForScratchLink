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

void imu_getaccel(float *paccx, float *paccy, float *paccz )
{
  M5.Imu.update();
  imuData = M5.Imu.getImuData();
  if (paccx) {
    *paccx = imuData.accel.x - accx_center;
  }
  if (paccy) {
    *paccy = imuData.accel.y - accy_center;
  }
  if (paccz) {
    *paccz = imuData.accel.z - accz_center;
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
