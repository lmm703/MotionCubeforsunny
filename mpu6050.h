#ifndef MPU6050_H
#define MPU6050_H

#include <Arduino.h>
#include <Wire.h>

class MPU6050 {
 public:
  static constexpr uint8_t kDefaultAddress = 0x68;

  enum class InitStatus : uint8_t {
    Ok = 0,
    WhoAmIReadFailed,
    UnexpectedWhoAmI,
    WakeFailed,
    AccelConfigFailed,
    GyroConfigFailed,
  };

  struct RawSample {
    int16_t accelX;
    int16_t accelY;
    int16_t accelZ;
    int16_t temperatureRaw;
    int16_t gyroX;
    int16_t gyroY;
    int16_t gyroZ;

    float temperatureCelsius() const;
  };

  explicit MPU6050(TwoWire &wire = Wire, uint8_t address = kDefaultAddress);

  InitStatus begin();
  bool readWhoAmI(uint8_t &whoAmI);
  bool readRawSample(RawSample &sample);

  static const char *statusToString(InitStatus status);

 private:
  static constexpr uint8_t kRegisterWhoAmI = 0x75;
  static constexpr uint8_t kRegisterPwrMgmt1 = 0x6B;
  static constexpr uint8_t kRegisterAccelConfig = 0x1C;
  static constexpr uint8_t kRegisterGyroConfig = 0x1B;
  static constexpr uint8_t kRegisterAccelXoutH = 0x3B;
  static constexpr uint8_t kExpectedWhoAmI = 0x68;

  bool writeRegister(uint8_t reg, uint8_t value);
  bool readRegister(uint8_t reg, uint8_t &value);
  bool readRegisters(uint8_t startReg, uint8_t *buffer, size_t length);

  TwoWire *wire_;
  uint8_t address_;
};

#endif  // MPU6050_H
