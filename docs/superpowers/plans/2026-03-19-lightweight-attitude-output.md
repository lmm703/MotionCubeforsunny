# Lightweight Attitude Output Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a minimal complementary-filter-based roll and pitch output mode while preserving the existing raw CSV mode behind compile-time switches.

**Architecture:** Keep the MPU6050 driver focused on raw register access and perform all attitude math in `MotionCube.ino` to minimize churn. Add small application-layer helpers for scaling, complementary filter state, and CSV formatting so the existing sampling loop stays easy to follow.

**Tech Stack:** Arduino framework, Wire, MPU6050 raw register access, basic trigonometric functions from Arduino math support.

---

## Chunk 1: Application-Layer Attitude Output

### Task 1: Add compile-time output mode switches and attitude helpers

**Files:**
- Modify: `D:\ArduinoProjects\MotionCube\MotionCube.ino`

- [ ] **Step 1: Write the failing test**

There is no automated test harness in this minimal Arduino sketch project. Use device-observable behavior as the verification target for this change:
- raw CSV mode still prints `millis,ax,ay,az,gx,gy,gz,temp`
- attitude mode prints `millis,roll,pitch`
- roll and pitch react to board motion and remain bounded while stationary

- [ ] **Step 2: Run test to verify it fails**

Run: compile and flash the existing sketch, then observe that no roll/pitch output mode exists.
Expected: the sketch only prints raw CSV samples and cannot output complementary-filter roll/pitch values.

- [ ] **Step 3: Write minimal implementation**

In `MotionCube.ino`:
- add `constexpr` compile-time switches for raw and attitude CSV output
- add constants for accelerometer and gyroscope scaling and complementary filter alpha
- add a small `AttitudeEstimate` struct storing `rollDeg`, `pitchDeg`, and initialization state
- add helpers to compute accelerometer tilt, gyro rate in deg/s, and complementary filter update
- keep existing sampling interval and re-use the same raw sensor sample for both output modes

- [ ] **Step 4: Run test to verify it passes**

Run: compile and flash the updated sketch, then observe:
- raw mode still matches previous output format when enabled
- attitude mode prints `millis,roll,pitch`
- moving the board changes roll/pitch in expected directions

- [ ] **Step 5: Commit**

Skip commit in this workspace unless explicitly requested by the user.

### Task 2: Document the new formulas and configuration

**Files:**
- Modify: `D:\ArduinoProjects\MotionCube\README.md`

- [ ] **Step 1: Write the failing test**

Current README does not describe complementary filter output, formulas, or tunable parameters.

- [ ] **Step 2: Run test to verify it fails**

Run: read `README.md`.
Expected: no section describing roll/pitch output mode, complementary filter equations, or the meaning of `alpha`.

- [ ] **Step 3: Write minimal implementation**

Update the README to explain:
- compile-time raw/attitude output switches
- attitude CSV format
- accelerometer tilt equations
- gyroscope integration equations
- complementary filter equation and default parameters
- current limitation that yaw is not implemented

- [ ] **Step 4: Run test to verify it passes**

Run: read `README.md`.
Expected: the new behavior and math are documented clearly enough for future tuning and phone-side integration.

- [ ] **Step 5: Commit**

Skip commit in this workspace unless explicitly requested by the user.
