# Arduino FastLED Spectrum Analyzer

This repository contains Arduino sketches for an LED spectrum analyzer using `FastLED` and an `MSGEQ7` audio spectrum chip.

## Included files
- `arduino_code_spectrum_analyzer.ino` - main project sketch (current workspace file).
- `Festival.ino` - alternative/legacy sketch with multiple effect modes (from prior context).

## Hardware setup
- LED strip: WS2813 (or similar WS281x), connected to `DATA_PIN` (default pin 22 in `Festival.ino`)
- `MSGEQ7` frequency decoder:
  - `STROBE_PIN` to pin 4
  - `RESET_PIN` to pin 5
  - output bands to analog pins A0/A1
  - audio signal ground and 5V to board

## Software requirements
- Arduino IDE
- FastLED library (tested with v3.2.6; newer versions generally compatible)

## Installation
1. Open `arduino_code_spectrum_analyzer.ino` in Arduino IDE.
2. Verify connections in code match your wiring:
   - data pin
   - strobe and reset pins
   - number of columns/rows and LED count
3. Upload to supported board (e.g., Arduino Nano/Mega/Uno with enough RAM)

## Basic behavior
- Reads 7 bands from MSGEQ7.
- Maps audio amplitude to LED matrix levels.
- Supports several display effects (rainbow, single-color, dots, columns) depending on mode in sketch.
- Uses `FastLED.show()` for frame updates.

## Customization
- Adjust `BRIGHTNESS` to reduce current draw.
- Adjust `NOISECOMP` to filter background noise.
- Adjust `BOOST` to control sensitivity.
- Adjust `COLUMN` and `ROWS` to your LED matrix layout.

## Tips
- Use a stable 5V power supply rated for your LED count (20mA per LED at full white).
- Keep brightness low (e.g., 50) for long-term reliability.
- Add a button or serial command interface if you want effect switching at runtime.

## Notes
- The provided code is intended for learning and small experiment setups. Further memory and performance optimization may be required for large matrices.
- The code currently uses `delay(30)` in loop; non-blocking timing (`millis()`) is recommended for more responsive control.
