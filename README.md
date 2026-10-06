# ESP32 Heart Sound Monitor

Real-time heart-sound streaming and visualization using an ESP32, an analog audio input, and a digital heart-sound detection input.

## Files

- `ESP32_Heart_Sound_Monitor.ino` — ESP32 firmware.
- `index.html` — browser-based heart sound visualizer.

## ESP32 setup

- Analog audio input: GPIO 34
- Digital microphone/beat output: GPIO 27
- Bluetooth device name: `ESP32-HEART`
- Serial baud rate: 115200
- Audio sample rate: 4000 Hz
- Audio block size: 128 samples

## Browser monitor

The browser page uses the Web Serial API and works with Chromium-based browsers such as Chrome and Edge.

1. Upload the Arduino sketch to the ESP32.
2. Connect the ESP32 to the computer using USB.
3. Close Arduino Serial Monitor/Serial Plotter.
4. Open `index.html` through a local web server or GitHub Pages.
5. Click **Connect ESP32 (USB)** and select the ESP32 serial port.

## Audio packet format

Each audio block is sent as:

`A5 5A | 128 | 128 audio bytes | checksum`

The checksum is the 8-bit sum of the 128 payload bytes.

The browser validates the header, payload length, and checksum before displaying and playing a block.

## Detection

The firmware detects S1 and S2 from the digital input. A valid S1-S2 interval is 100–500 ms, and BPM is calculated from valid S1-S2 pairs over a 5-second window.

## Important

The current web page and ESP32 firmware use the same packet format and 115200 baud rate. No packet-format change is required.
