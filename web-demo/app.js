const BLE_SERVICE_UUID = "19b10000-e8f2-537e-4f6c-d104768a1214";
const BLE_ATTITUDE_CHARACTERISTIC_UUID = "19b10001-e8f2-537e-4f6c-d104768a1214";

const connectButton = document.getElementById("connectButton");
const disconnectButton = document.getElementById("disconnectButton");
const statusBanner = document.getElementById("statusBanner");
const statusValue = document.getElementById("statusValue");
const statusHint = document.getElementById("statusHint");
const rollValue = document.getElementById("rollValue");
const pitchValue = document.getElementById("pitchValue");
const packetValue = document.getElementById("packetValue");
const errorCard = document.getElementById("errorCard");
const errorValue = document.getElementById("errorValue");
const sculptureRig = document.getElementById("sculptureRig");

let bluetoothDevice = null;
let attitudeCharacteristic = null;

function setStatus(state, message, hint) {
  statusBanner.dataset.state = state;
  statusValue.textContent = message;
  statusHint.textContent = hint;
}

function setError(message) {
  if (!message) {
    errorCard.hidden = true;
    errorValue.textContent = "No issues.";
    return;
  }

  errorCard.hidden = false;
  errorValue.textContent = message;
}

function setButtonState({ connecting = false, connected = false } = {}) {
  connectButton.disabled = connecting || connected;
  disconnectButton.disabled = !connected;

  if (connected) {
    connectButton.textContent = "Connected";
  } else if (connecting) {
    connectButton.textContent = "Connecting...";
  } else {
    connectButton.textContent = "Connect BLE";
  }
}

function updateCube(rollDeg, pitchDeg) {
  rollValue.textContent = `${rollDeg.toFixed(2)} deg`;
  pitchValue.textContent = `${pitchDeg.toFixed(2)} deg`;
  packetValue.textContent = new Date().toLocaleTimeString();
  const displayPitch = -14 + (pitchDeg * 0.78);
  const displayRoll = 18 + (rollDeg * 0.72);
  sculptureRig.style.transform =
    `translateY(12px) rotateX(${displayPitch}deg) rotateY(${displayRoll}deg)`;
}

function parseAttitudePayload(text) {
  const [rollText, pitchText] = text.trim().split(",");
  const rollDeg = Number.parseFloat(rollText);
  const pitchDeg = Number.parseFloat(pitchText);

  if (Number.isNaN(rollDeg) || Number.isNaN(pitchDeg)) {
    throw new Error(`Unexpected payload: ${text}`);
  }

  return { rollDeg, pitchDeg };
}

function handleAttitudeNotification(event) {
  const decoder = new TextDecoder("utf-8");
  const text = decoder.decode(event.target.value.buffer);
  const { rollDeg, pitchDeg } = parseAttitudePayload(text);
  updateCube(rollDeg, pitchDeg);
}

function handleDisconnected() {
  setStatus("idle", "Disconnected", "You can reconnect to MotionCube at any time.");
  setButtonState({ connecting: false, connected: false });
  attitudeCharacteristic = null;
}

async function disconnectBle() {
  if (bluetoothDevice?.gatt?.connected) {
    bluetoothDevice.gatt.disconnect();
  } else {
    handleDisconnected();
  }
}

function validateEnvironment() {
  if (!window.isSecureContext) {
    throw new Error("This demo must be opened from an HTTPS page.");
  }

  if (!navigator.bluetooth) {
    throw new Error("Web Bluetooth is not available in this browser.");
  }
}

async function connectBle() {
  validateEnvironment();
  setError("");
  setStatus(
    "connecting",
    "Requesting device",
    "Choose MotionCube from the Bluetooth device picker.",
  );
  setButtonState({ connecting: true, connected: false });

  bluetoothDevice = await navigator.bluetooth.requestDevice({
    filters: [
      { namePrefix: "MotionCube" },
      { services: [BLE_SERVICE_UUID] },
    ],
    optionalServices: [BLE_SERVICE_UUID],
  });

  bluetoothDevice.addEventListener(
    "gattserverdisconnected",
    handleDisconnected,
    { once: false },
  );

  setStatus(
    "connecting",
    "Connecting",
    "Establishing the BLE link and subscribing to live attitude updates.",
  );
  const server = await bluetoothDevice.gatt.connect();
  const service = await server.getPrimaryService(BLE_SERVICE_UUID);
  attitudeCharacteristic = await service.getCharacteristic(
    BLE_ATTITUDE_CHARACTERISTIC_UUID,
  );

  await attitudeCharacteristic.startNotifications();
  attitudeCharacteristic.addEventListener(
    "characteristicvaluechanged",
    handleAttitudeNotification,
  );

  const initialValue = await attitudeCharacteristic.readValue();
  handleAttitudeNotification({ target: { value: initialValue } });

  setStatus(
    "streaming",
    "Streaming",
    "Tilt the board and the nightingale-and-rose sculpture should move with the incoming roll and pitch.",
  );
  setButtonState({ connecting: false, connected: true });
}

connectButton.addEventListener("click", async () => {
  try {
    await connectBle();
  } catch (error) {
    console.error(error);
    setStatus(
      "error",
      "Connection failed",
      "Check HTTPS, browser Bluetooth support, and whether MotionCube is advertising.",
    );
    setError(error.message || "Connection failed");
    setButtonState({ connecting: false, connected: false });
  }
});

disconnectButton.addEventListener("click", async () => {
  await disconnectBle();
});

setStatus(
  "idle",
  "Idle",
  "Open this page in Chrome on Android over HTTPS, then connect to MotionCube.",
);
setButtonState({ connecting: false, connected: false });
