#include "mpu6050.h"

namespace {

int16_t joinBytes(uint8_t highByte, uint8_t lowByte) {
  return static_cast<int16_t>((static_cast<uint16_t>(highByte) << 8) | lowByte);
}

}  // namespace

MPU6050::MPU6050(TwoWire &wire, uint8_t address) : wire_(&wire), address_(address) {}

MPU6050::InitStatus MPU6050::begin() {
  uint8_t whoAmI = 0;
  if (!readWhoAmI(whoAmI)) {
    return InitStatus::WhoAmIReadFailed;
  }

  if (whoAmI != kExpectedWhoAmI) {
    return InitStatus::UnexpectedWhoAmI;
  }

  // Wake the chip and force known measurement ranges for predictable output.
  if (!writeRegister(kRegisterPwrMgmt1, 0x00)) {
    return InitStatus::WakeFailed;
  }

  if (!writeRegister(kRegisterAccelConfig, 0x00)) {
    return InitStatus::AccelConfigFailed;
  }

  if (!writeRegister(kRegisterGyroConfig, 0x00)) {
    return InitStatus::GyroConfigFailed;
  }

  delay(100);
  return InitStatus::Ok;
}

bool MPU6050::readWhoAmI(uint8_t &whoAmI) {
  return readRegister(kRegisterWhoAmI, whoAmI);
}

bool MPU6050::readRawSample(RawSample &sample) {
  uint8_t buffer[14] = {0};
  if (!readRegisters(kRegisterAccelXoutH, buffer, sizeof(buffer))) {
    return false;
  }

  sample.accelX = joinBytes(buffer[0], buffer[1]);
  sample.accelY = joinBytes(buffer[2], buffer[3]);
  sample.accelZ = joinBytes(buffer[4], buffer[5]);
  sample.temperatureRaw = joinBytes(buffer[6], buffer[7]);
  sample.gyroX = joinBytes(buffer[8], buffer[9]);
  sample.gyroY = joinBytes(buffer[10], buffer[11]);
  sample.gyroZ = joinBytes(buffer[12], buffer[13]);

  return true;
}

float MPU6050::RawSample::temperatureCelsius() const {
  return (static_cast<float>(temperatureRaw) / 340.0f) + 36.53f;
}

const char *MPU6050::statusToString(InitStatus status) {
  switch (status) {
    case InitStatus::Ok:
      return "ok";
    case InitStatus::WhoAmIReadFailed:
      return "unable to read WHO_AM_I register";
    case InitStatus::UnexpectedWhoAmI:
      return "WHO_AM_I mismatch (expected 0x68)";
    case InitStatus::WakeFailed:
      return "failed to clear sleep bit in PWR_MGMT_1";
    case InitStatus::AccelConfigFailed:
      return "failed to set ACCEL_CONFIG";
    case InitStatus::GyroConfigFailed:
      return "failed to set GYRO_CONFIG";
    default:
      return "unknown initialization error";
  }
}

bool MPU6050::writeRegister(uint8_t reg, uint8_t value) {
  wire_->beginTransmission(address_);
  wire_->write(reg);
  wire_->write(value);
  return wire_->endTransmission() == 0;
}

bool MPU6050::readRegister(uint8_t reg, uint8_t &value) {
  return readRegisters(reg, &value, 1);
}

bool MPU6050::readRegisters(uint8_t startReg, uint8_t *buffer, size_t length) {
  wire_->beginTransmission(address_);
  wire_->write(startReg);
  if (wire_->endTransmission(false) != 0) {
    return false;
  }

  const size_t requested = wire_->requestFrom(static_cast<int>(address_),
                                              static_cast<int>(length),
                                              static_cast<int>(true));
  if (requested != length) {
    return false;
  }

  for (size_t i = 0; i < length; ++i) {
    if (!wire_->available()) {
      return false;
    }
    buffer[i] = static_cast<uint8_t>(wire_->read());
  }

  return true;
}
