#ifndef __IMU__
#define __IMU__

void init_imu();
void imu_getaccel(float *accx, float *accy, float *accz );
void imu_calibrate();

#endif
