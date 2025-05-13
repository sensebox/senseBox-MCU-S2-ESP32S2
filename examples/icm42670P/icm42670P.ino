#include "ICM42670P.h"     
#define I2C_PIN_SCL 42
#define I2C_PIN_SDA 45
ICM42670 icm = ICM42670(Wire1, 0); // Instantiate an ICM42670 with LSB address set to 0
void setup() {
  Wire1.begin(I2C_PIN_SDA, I2C_PIN_SCL);
  Serial.begin(115200);
  delay(1500);
  
  if (icm.begin() == 0)
  {
    // If MPU6050 fails, try ICM42670P
    Serial.println("ICM42670P Found!");
    icm.startAccel(100, 8); // Accel ODR = 100 Hz, Full Scale Range = 8G
    icm.startGyro(100, 500); // Gyro ODR = 100 Hz, Full Scale Range = 500 dps
  }
  else
  {
    Serial.println("acceleration sensor not found");
    delay(100000);
    return;
  }
  delay(100);
}
void loop() {
  inv_imu_sensor_event_t imu_event;

  // Get last event
  icm.getDataFromRegisters(imu_event);

  // Format data for Serial Plotter
  Serial.print("AccelX:");
  Serial.println((imu_event.accel[0]*9.81)/4096.0);
  Serial.print("AccelY:");
  Serial.println((imu_event.accel[1]*9.81)/4096.0);
  Serial.print("AccelZ:");
  Serial.println((imu_event.accel[2]*9.81)/4096.0);
  Serial.print("GyroX:");
  Serial.println((imu_event.gyro[0]*500.0)/32768.0);
  Serial.print("GyroY:");
  Serial.println((imu_event.gyro[1]*500.0)/32768.0);
  Serial.print("GyroZ:");
  Serial.println((imu_event.gyro[2]*500.0)/32768.0);
  Serial.print("Temperature:");
  Serial.println((imu_event.temperature/132.48)+25.0);

  // Run @ ODR 100Hz
  delay(100);
}