#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Wire.h>

#include "mpu6050.h"

namespace {

constexpr unsigned long kSerialBaud = 115200;
constexpr unsigned long kSampleIntervalMs = 50;
constexpr bool kEnableRawCsvOutput = false;
constexpr bool kEnableAttitudeCsvOutput = true;
constexpr bool kEnableBleAttitudeOutput = true;
constexpr float kAccelLsbPerG = 16384.0f;
constexpr float kGyroLsbPerDegPerSec = 131.0f;
constexpr float kComplementaryAlpha = 0.98f;
constexpr char kBleDeviceName[] = "MotionCube";
constexpr char kBleServiceUuid[] = "19b10000-e8f2-537e-4f6c-d104768a1214";
constexpr char kBleAttitudeCharacteristicUuid[] =
    "19b10001-e8f2-537e-4f6c-d104768a1214";

struct TiltAngles {
  float rollDeg;
  float pitchDeg;
};

struct AttitudeEstimate {
  float rollDeg = 0.0f;
  float pitchDeg = 0.0f;
  bool initialized = false;
};

MPU6050 g_mpu;
BLECharacteristic *g_attitudeCharacteristic = nullptr;
bool g_bleReady = false;
bool g_bleClientConnected = false;

class MotionCubeBleServerCallbacks : public BLEServerCallbacks {
 public:
  void onConnect(BLEServer *server) override {
    (void)server;
    g_bleClientConnected = true;
    Serial.println("BLE central connected.");
  }

  void onDisconnect(BLEServer *server) override {
    (void)server;
    g_bleClientConnected = false;
    Serial.println("BLE central disconnected, restarting advertising.");
    BLEDevice::startAdvertising();
  }
};

void waitForSerial(unsigned long timeoutMs) {
  const unsigned long start = millis();
  while (!Serial && (millis() - start) < timeoutMs) {
    delay(10);
  }
}

void printHexByte(uint8_t value) {
  if (value < 0x10) {
    Serial.print('0');
  }
  Serial.print(value, HEX);
}

void printOnOffLine(const char *label, bool enabled) {
  Serial.print(label);
  Serial.print(": ");
  Serial.println(enabled ? "ON" : "OFF");
}

void printRuntimeConfig() {
  Serial.println("Runtime configuration:");
  printOnOffLine("  Raw CSV output", kEnableRawCsvOutput);
  printOnOffLine("  Attitude CSV output", kEnableAttitudeCsvOutput);
  printOnOffLine("  BLE attitude output", kEnableBleAttitudeOutput);
  Serial.print("  Sample interval (ms): ");
  Serial.println(kSampleIntervalMs);
}

void haltWithError(const char *message) {
  Serial.print("ERROR: ");
  Serial.println(message);
  Serial.println("System halted.");

  while (true) {
    delay(1000);
  }
}

void scanI2CBus(TwoWire &wire) {
  Serial.println("Scanning I2C bus...");

  uint8_t devicesFound = 0;
  for (uint8_t address = 1; address < 127; ++address) {
    wire.beginTransmission(address);
    const uint8_t error = wire.endTransmission();

    if (error == 0) {
      Serial.print("  Found device at 0x");
      printHexByte(address);
      Serial.println();
      ++devicesFound;
    } else if (error == 4) {
      Serial.print("  Unknown error at 0x");
      printHexByte(address);
      Serial.println();
    }
  }

  if (devicesFound == 0) {
    Serial.println("  No I2C devices found.");
  } else {
    Serial.print("I2C scan complete, devices found: ");
    Serial.println(devicesFound);
  }
}

void printCsvHeader() {
  Serial.println("millis,ax,ay,az,gx,gy,gz,temp");
}

void printAttitudeCsvHeader() {
  Serial.println("millis,roll,pitch");
}

void printSampleCsv(unsigned long nowMs, const MPU6050::RawSample &sample) {
  Serial.print(nowMs);
  Serial.print(',');
  Serial.print(sample.accelX);
  Serial.print(',');
  Serial.print(sample.accelY);
  Serial.print(',');
  Serial.print(sample.accelZ);
  Serial.print(',');
  Serial.print(sample.gyroX);
  Serial.print(',');
  Serial.print(sample.gyroY);
  Serial.print(',');
  Serial.print(sample.gyroZ);
  Serial.print(',');
  Serial.println(sample.temperatureCelsius(), 2);
}

void printAttitudeCsv(unsigned long nowMs, const AttitudeEstimate &attitude) {
  Serial.print(nowMs);
  Serial.print(',');
  Serial.print(attitude.rollDeg, 2);
  Serial.print(',');
  Serial.println(attitude.pitchDeg, 2);
}

void publishBleAttitude(const AttitudeEstimate &attitude) {
  if (!kEnableBleAttitudeOutput || !g_bleReady || !g_bleClientConnected ||
      g_attitudeCharacteristic == nullptr) {
    return;
  }

  char payload[32] = {0};
  snprintf(payload,
           sizeof(payload),
           "%.2f,%.2f",
           attitude.rollDeg,
           attitude.pitchDeg);
  g_attitudeCharacteristic->setValue(reinterpret_cast<uint8_t *>(payload),
                                     strlen(payload));
  g_attitudeCharacteristic->notify();
}

TiltAngles computeAccelTilt(const MPU6050::RawSample &sample) {
  const float ax = static_cast<float>(sample.accelX) / kAccelLsbPerG;
  const float ay = static_cast<float>(sample.accelY) / kAccelLsbPerG;
  const float az = static_cast<float>(sample.accelZ) / kAccelLsbPerG;

  TiltAngles tilt{};
  tilt.rollDeg = atan2f(ay, az) * 180.0f / PI;
  tilt.pitchDeg = atan2f(-ax, sqrtf((ay * ay) + (az * az))) * 180.0f / PI;
  return tilt;
}

void updateAttitudeEstimate(AttitudeEstimate &attitude,
                            const MPU6050::RawSample &sample,
                            float dtSeconds) {
  const TiltAngles accelTilt = computeAccelTilt(sample);
  if (!attitude.initialized) {
    attitude.rollDeg = accelTilt.rollDeg;
    attitude.pitchDeg = accelTilt.pitchDeg;
    attitude.initialized = true;
    return;
  }

  const float gyroRollDegPerSec =
      static_cast<float>(sample.gyroX) / kGyroLsbPerDegPerSec;
  const float gyroPitchDegPerSec =
      static_cast<float>(sample.gyroY) / kGyroLsbPerDegPerSec;

  const float rollFromGyro = attitude.rollDeg + (gyroRollDegPerSec * dtSeconds);
  const float pitchFromGyro =
      attitude.pitchDeg + (gyroPitchDegPerSec * dtSeconds);

  attitude.rollDeg =
      (kComplementaryAlpha * rollFromGyro) +
      ((1.0f - kComplementaryAlpha) * accelTilt.rollDeg);
  attitude.pitchDeg =
      (kComplementaryAlpha * pitchFromGyro) +
      ((1.0f - kComplementaryAlpha) * accelTilt.pitchDeg);
}

void setupBleAttitudeService() {
  if (!kEnableBleAttitudeOutput) {
    Serial.println("BLE attitude output disabled.");
    return;
  }

  BLEDevice::init(kBleDeviceName);

  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new MotionCubeBleServerCallbacks());

  BLEService *service = server->createService(kBleServiceUuid);
  g_attitudeCharacteristic =
      service->createCharacteristic(kBleAttitudeCharacteristicUuid,
                                    BLECharacteristic::PROPERTY_READ |
                                        BLECharacteristic::PROPERTY_NOTIFY);
  g_attitudeCharacteristic->addDescriptor(new BLE2902());
  g_attitudeCharacteristic->setValue("0.00,0.00");

  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(kBleServiceUuid);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);
  advertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  g_bleReady = true;

  Serial.print("BLE advertising as ");
  Serial.println(kBleDeviceName);
  Serial.print("BLE service UUID: ");
  Serial.println(kBleServiceUuid);
}

}  // namespace

void setup() {
  Serial.begin(kSerialBaud);
  waitForSerial(2000);

  Serial.println();
  Serial.println("MotionCube booting...");
  printRuntimeConfig();

  Wire.begin();
  scanI2CBus(Wire);

  uint8_t whoAmI = 0;
  if (!g_mpu.readWhoAmI(whoAmI)) {
    haltWithError("Failed to read MPU6050 WHO_AM_I register.");
  }

  Serial.print("MPU6050 WHO_AM_I = 0x");
  printHexByte(whoAmI);
  Serial.println();

  const MPU6050::InitStatus initStatus = g_mpu.begin();
  if (initStatus != MPU6050::InitStatus::Ok) {
    Serial.print("ERROR: MPU6050 initialization failed: ");
    Serial.println(MPU6050::statusToString(initStatus));
    Serial.println("System halted.");

    while (true) {
      delay(1000);
    }
  }

  Serial.println("MPU6050 initialization complete.");
  setupBleAttitudeService();

  if (kEnableRawCsvOutput) {
    printCsvHeader();
  }

  if (kEnableAttitudeCsvOutput) {
    printAttitudeCsvHeader();
  }
}

void loop() {
  static unsigned long lastOutputMs = 0;
  static unsigned long lastFilterUpdateMs = 0;
  static AttitudeEstimate attitude{};

  const unsigned long nowMs = millis();
  if ((nowMs - lastOutputMs) < kSampleIntervalMs) {
    return;
  }

  lastOutputMs = nowMs;

  MPU6050::RawSample sample{};
  if (!g_mpu.readRawSample(sample)) {
    haltWithError("Failed to read raw sample from MPU6050.");
  }

  float dtSeconds = kSampleIntervalMs / 1000.0f;
  if (lastFilterUpdateMs != 0) {
    dtSeconds = static_cast<float>(nowMs - lastFilterUpdateMs) / 1000.0f;
  }
  lastFilterUpdateMs = nowMs;

  updateAttitudeEstimate(attitude, sample, dtSeconds);

  if (kEnableRawCsvOutput) {
    printSampleCsv(nowMs, sample);
  }

  if (kEnableAttitudeCsvOutput && attitude.initialized) {
    printAttitudeCsv(nowMs, attitude);
  }

  publishBleAttitude(attitude);
}
