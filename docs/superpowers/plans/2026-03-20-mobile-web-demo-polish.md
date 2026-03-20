# Mobile Web Demo Polish Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Improve the existing Android Web Bluetooth cube demo so it feels clearer and more reliable on a phone screen without changing the firmware protocol.

**Architecture:** Keep the existing static `web-demo` structure and BLE behavior, but upgrade the page content, status handling, and mobile-first layout. Avoid introducing new dependencies so the deployment story stays simple for HTTPS static hosting.

**Tech Stack:** Static HTML, CSS, JavaScript, Web Bluetooth API, CSS 3D transforms.

---

## Chunk 1: Mobile-First Demo UX

### Task 1: Improve page layout, status messaging, and connection controls

**Files:**
- Modify: `D:\ArduinoProjects\MotionCube\web-demo\index.html`
- Modify: `D:\ArduinoProjects\MotionCube\web-demo\styles.css`
- Modify: `D:\ArduinoProjects\MotionCube\web-demo\app.js`

- [ ] **Step 1: Write the failing test**

Use manual observable behavior as the test target:
- current page has limited status feedback
- current page does not explain steps clearly enough for first-time phone use
- current page lacks an explicit disconnect control and clearer error surface

- [ ] **Step 2: Run test to verify it fails**

Open the existing page source and confirm:
- no visible step-by-step usage section
- no dedicated error/status banner
- no disconnect button
- limited mobile-focused layout guidance

- [ ] **Step 3: Write minimal implementation**

Update the existing files to:
- add a compact "how to use" section
- add a dedicated status/error area
- add connect and disconnect controls
- improve mobile layout spacing and touch target sizing
- refine cube presentation and connection state display
- preserve the same BLE service/characteristic protocol

- [ ] **Step 4: Run test to verify it passes**

Manual browser verification:
- page reads clearly on a phone-sized viewport
- connection state is easy to understand
- disconnect is available
- errors are visible without opening developer tools
- cube and numeric telemetry remain functional

- [ ] **Step 5: Commit**

Skip commit unless explicitly requested by the user.
