# Android Web BLE Cube Demo Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add BLE attitude notifications to the XIAO ESP32S3 firmware and a static Android-friendly web page that renders a live cube from `roll,pitch`.

**Architecture:** Keep sensor and complementary-filter logic in `MotionCube.ino`, add a minimal BLE transport layer there, and create a dependency-free static web client under a separate demo directory. Use a plain-text `roll,pitch` payload to reduce risk and make debugging easy across firmware and browser layers.

**Tech Stack:** Arduino framework on ESP32, built-in ESP32 BLE library, static HTML/CSS/JavaScript, Web Bluetooth API, CSS 3D transforms.

---

## Chunk 1: Firmware BLE Transport

### Task 1: Add BLE advertising, characteristic notifications, and boot mode logging

**Files:**
- Modify: `D:\ArduinoProjects\MotionCube\MotionCube.ino`

- [ ] **Step 1: Write the failing test**

Use observable firmware behavior as the test target:
- the sketch should log its active serial/BLE modes at boot
- the board should advertise as `MotionCube`
- the firmware should expose a readable/notifiable attitude characteristic
- the characteristic should emit `roll,pitch` text while the board is tilted

- [ ] **Step 2: Run test to verify it fails**

Run the current firmware and verify:
- no BLE device named `MotionCube` is advertised
- no BLE characteristic exists for browser subscription
- boot log does not identify output modes or BLE status

- [ ] **Step 3: Write minimal implementation**

In `MotionCube.ino`:
- include the ESP32 BLE headers
- define custom service/characteristic UUID constants
- add BLE setup helpers
- add a helper to publish `roll,pitch` text notifications
- print active output modes at startup
- keep serial output compile-time switches intact

- [ ] **Step 4: Run test to verify it passes**

Run:
- `arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32S3 D:\ArduinoProjects\MotionCube`

Expected:
- compile succeeds

Manual hardware verification:
- board advertises `MotionCube`
- boot log shows BLE enabled and current serial output modes
- live angle notifications are available to a BLE central

- [ ] **Step 5: Commit**

Skip commit unless explicitly requested by the user.

## Chunk 2: Android Web Viewer

### Task 2: Create a static Web Bluetooth cube page

**Files:**
- Create: `D:\ArduinoProjects\MotionCube\web-demo\index.html`
- Create: `D:\ArduinoProjects\MotionCube\web-demo\styles.css`
- Create: `D:\ArduinoProjects\MotionCube\web-demo\app.js`

- [ ] **Step 1: Write the failing test**

Use observable browser behavior as the test target:
- there is currently no web page that can connect to `MotionCube`
- there is currently no cube rendering driven by BLE notifications

- [ ] **Step 2: Run test to verify it fails**

Inspect the repo and confirm there is no existing Android web BLE demo page.

- [ ] **Step 3: Write minimal implementation**

Create a static page that:
- provides a `Connect BLE` button
- uses Web Bluetooth to request the custom service
- subscribes to the attitude characteristic
- parses `roll,pitch` text
- updates displayed numeric values
- rotates a CSS cube

- [ ] **Step 4: Run test to verify it passes**

Manual browser verification on Android Chrome over HTTPS:
- page loads
- `Connect BLE` opens the chooser
- selecting `MotionCube` connects
- live values update
- cube rotates with device tilt

- [ ] **Step 5: Commit**

Skip commit unless explicitly requested by the user.

## Chunk 3: Documentation

### Task 3: Document BLE demo setup and browser constraints

**Files:**
- Modify: `D:\ArduinoProjects\MotionCube\README.md`

- [ ] **Step 1: Write the failing test**

Current README does not describe:
- BLE demo mode
- web page usage
- the HTTPS requirement for Web Bluetooth

- [ ] **Step 2: Run test to verify it fails**

Read `README.md` and confirm that the Android BLE cube demo flow is not documented.

- [ ] **Step 3: Write minimal implementation**

Update the README with:
- BLE device name and payload format
- the web demo directory contents
- the need to serve the page over HTTPS
- Android Chrome connection steps
- what to expect visually

- [ ] **Step 4: Run test to verify it passes**

Read `README.md` and confirm it now explains how to reproduce the demo from firmware upload through phone connection.

- [ ] **Step 5: Commit**

Skip commit unless explicitly requested by the user.
