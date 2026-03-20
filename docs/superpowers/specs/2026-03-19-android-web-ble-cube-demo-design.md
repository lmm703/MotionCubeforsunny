# Android Web BLE Cube Demo Design

## Goal

Build the fastest end-to-end demonstration of MotionCube orientation data on an Android phone by:

- keeping the current MPU6050 complementary-filter firmware
- adding BLE transmission of `roll` and `pitch`
- adding a small phone-friendly web page that connects over Web Bluetooth
- rotating a simple cube in the browser based on incoming data

This version is optimized for speed of delivery and demo value, not for final production architecture.

## Scope

Included in this round:

- XIAO ESP32S3 BLE peripheral mode
- periodic BLE notifications carrying `roll,pitch`
- a lightweight Android web client
- a simple 3D cube viewer
- connection status and live angle display in the page
- retaining serial output for local debugging

Explicitly excluded:

- DMP
- BLE control/config protocol beyond connection and notifications
- yaw stabilization
- native Android app
- calibration UI
- reconnect persistence across page reloads

## Firmware Design

The current application already computes `roll` and `pitch` in `MotionCube.ino`. To minimize churn, BLE is added at the application layer rather than inside the MPU6050 driver.

### Responsibilities

- `MotionCube.ino`
  - initializes BLE after IMU startup
  - advertises a simple custom service
  - pushes `roll,pitch` text notifications every sample cycle
  - keeps raw and/or attitude serial output behind compile-time switches
  - prints active output modes at boot for easier verification

- `mpu6050.h/.cpp`
  - unchanged sensor driver responsibilities

### BLE Shape

- Device name: `MotionCube`
- One custom service UUID
- One notify/read characteristic UUID
- Payload format: UTF-8 text line without JSON, example:
  - `-4.32,-52.18`

This text format is intentionally easy to debug from both firmware and browser code.

### Timing

- BLE notification cadence matches the current `50 ms` sample interval
- effective target update rate: about `20 Hz`

That is enough for a convincing first cube demo while keeping the implementation simple.

## Web Client Design

The web client is a static page intended to run in Chrome on Android under HTTPS.

### Responsibilities

- shows a `Connect BLE` button
- requests the `MotionCube` BLE device or custom service
- subscribes to notifications from the attitude characteristic
- parses incoming `roll,pitch`
- rotates a CSS 3D cube
- displays current angles and connection state

### Rendering Choice

Use CSS 3D rather than Three.js in this round.

Why:

- fewer moving parts
- zero external dependencies
- easier to host as a single static page
- fast enough to visually validate the BLE data path

## Data Mapping

The browser receives `roll` and `pitch` in degrees and maps them directly to cube transforms.

Initial mapping:

- cube X rotation follows `pitch`
- cube Y rotation follows `roll`

If the visual direction feels inverted during device testing, signs can be swapped in the page without changing the firmware transport format.

## Error Handling

Firmware:

- if BLE setup fails, print a clear serial error and continue halting behavior only for IMU failures
- if no central device is connected, keep advertising
- if connected, continue notifying on each sample cycle

Web page:

- show connection state
- show a readable error if Bluetooth is unavailable or permission is denied
- allow reconnecting without a page reload

## Testing Strategy

Manual verification is appropriate for this round because the project is a hardware-plus-browser demo.

Firmware verification:

- compile successfully for `XIAO_ESP32S3`
- boot log shows active output mode
- BLE device `MotionCube` appears in Android Chrome
- notifications contain live angle changes when the board is tilted

Web verification:

- page loads on Android Chrome over HTTPS
- tapping `Connect BLE` opens the device picker
- selecting `MotionCube` connects successfully
- live `roll/pitch` text updates
- cube rotates as the board is tilted

## Follow-On Path

After this fast demo version works, the next iteration can upgrade toward a more durable architecture:

- binary BLE payloads instead of text
- reconnect handling
- optional yaw experiments
- native app or more robust cross-platform client
- eventual switch from cube rendering to character rendering
