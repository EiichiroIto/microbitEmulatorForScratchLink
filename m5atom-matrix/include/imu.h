#ifndef __IMU__
#define __IMU__

void init_imu();
void imu_getaccel(uint16_t *accx, uint16_t *accy, uint16_t *accz );
void imu_calibrate();

#endif
