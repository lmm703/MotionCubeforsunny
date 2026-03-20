# Nightingale And Rose Scene Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the cube viewer with a moonlit "Nightingale and Rose" scene while keeping the existing BLE orientation pipeline intact.

**Architecture:** Reuse the current `web-demo` page structure and BLE logic, but swap the visual object from a cube to a layered sculpture rig composed of HTML elements styled with CSS gradients, shadows, and transforms. Keep interaction logic minimal by continuing to map incoming `roll/pitch` directly to a single rotating scene container.

**Tech Stack:** Static HTML, CSS, JavaScript, Web Bluetooth API, CSS transforms.

---

## Chunk 1: Scene Replacement

### Task 1: Replace the cube markup and styling with the rose-and-bird scene

**Files:**
- Modify: `D:\ArduinoProjects\MotionCube\web-demo\index.html`
- Modify: `D:\ArduinoProjects\MotionCube\web-demo\styles.css`
- Modify: `D:\ArduinoProjects\MotionCube\web-demo\app.js`

- [ ] **Step 1: Write the failing test**

Use observable behavior as the test target:
- the page currently renders a cube instead of the intended art scene
- the visual object does not resemble a rose or a perched bird

- [ ] **Step 2: Run test to verify it fails**

Inspect the current page and confirm the viewer still shows a cube.

- [ ] **Step 3: Write minimal implementation**

Update the web demo to:
- replace cube markup with a rose, bird, moon, and pedestal composition
- style the composition with layered petal depth and stronger atmosphere
- keep telemetry and BLE controls
- rotate the new sculpture rig from the same `roll/pitch` values

- [ ] **Step 4: Run test to verify it passes**

Manual verification:
- page still loads
- BLE controls remain usable
- telemetry still updates
- the new scene rotates with board movement

- [ ] **Step 5: Commit**

Skip commit unless explicitly requested by the user.
