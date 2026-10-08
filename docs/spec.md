# Specification: Portable STM32 Data Logger

Version: v0.1 (draft) | Date: 2026-10-08 | Status: to be reviewed by the project owner

---

## 1. What this document is

This file answers one question: **what exactly must the device do, in numbers?**

Every later design step takes its values from here:

| Later step | Which spec values it uses |
|---|---|
| SPICE: input divider and protection | Input range, overvoltage tolerance |
| SPICE: filter | Sample rate, signal bandwidth |
| SPICE: thermistor circuit | Temperature range, accuracy |
| Power budget and regulator choice | Runtime target, battery capacity |
| Schematic (choice of parts) | All of the above |
| Firmware | Sample rate, log format, UI, storage |
| Testing at the end | Every number is a pass/fail check |

Without these numbers you cannot tell whether a design is good, only that it is "something". With them, every choice is a measurable decision (for example: "does this divider keep the error below 1%?").

## 2. How the values were chosen

Nothing here is final. The values were chosen from three sources:

1. **Typical use:** what a small portable logger would reasonably be used for (battery voltages, sensor outputs, ambient temperature).
2. **What the chosen parts can do:** the STM32G431 ADC is 12-bit with a 3.3 V reference, which limits what resolution is realistic without extra hardware.
3. **Keeping the first board simple:** a first design that works is worth more than an ambitious one that does not.

Change any value that does not match what you want to build. If a value changes, update the "Why" and "Affects" columns too, and add a line to the change log at the bottom.

---

## 3. Requirements

### 3.1 Measurement channels

| ID | Requirement | Value | Why this value | Affects |
|---|---|---|---|---|
| M1 | Voltage channels | 2 | Enough to learn the analog front-end twice (once to design, once to confirm it is repeatable), and to log e.g. a battery and a sensor output at the same time | Number of dividers, filters and ADC pins |
| M2 | Voltage input range | 0 to 10 V | Covers most 5 V, 9 V and 12 V-class low-voltage signals up to 10 V. Larger ranges need more careful safety design | Divider ratio (about 0.3 so 10 V becomes about 3.0 V, leaving headroom below 3.3 V) |
| M3 | Input overvoltage tolerance | Survive up to 30 V and reverse polarity without damage | A mistake with a 12 V or 24 V source should not destroy the board | Protection parts: series resistor, clamp diodes, higher-rated divider resistors |
| M4 | Temperature channel | 1x NTC thermistor, 10 kOhm at 25 C, beta about 3950 | Cheap, common, and large signal change per degree, so it works well with a plain ADC | Thermistor circuit and conversion formula in firmware |
| M5 | Temperature range | -20 C to +80 C | Covers ambient and warm electronics. The NTC is accurate in this range with a plain divider | Choice of reference resistor in the thermistor divider |

### 3.2 Accuracy and resolution

| ID | Requirement | Value | Why this value | Affects |
|---|---|---|---|---|
| A1 | ADC resolution | 12-bit (4096 steps), 3.3 V reference | Native to the STM32G431. One step at the pin is about 0.8 mV | Fixes the best possible resolution |
| A2 | Resolution at the 10 V input | about 2.7 mV per step (before averaging) | 0.8 mV divided by the divider ratio of 0.3 | Whether it is worth adding an external ADC later (stretch goal) |
| A3 | Voltage accuracy | plus/minus 1% of reading | A realistic target using 0.1% to 1% resistors and the internal reference. Better needs calibration | Resistor tolerance, reference choice |
| A4 | Temperature accuracy | plus/minus 1 C (stretch: plus/minus 0.5 C with calibration) | Limited mostly by the thermistor tolerance, not the ADC. The ADC gives roughly 0.02 C per step near 25 C (to be verified in SPICE) | Thermistor grade, calibration step |
| A5 | Noise reduction | Oversample and average (for example 16 samples) | Averaging N samples reduces random noise by about the square root of N, and adds effective resolution | Firmware design, DMA buffer size |

### 3.3 Sampling and logging

| ID | Requirement | Value | Why this value | Affects |
|---|---|---|---|---|
| S1 | Logging rate | 1 sample per second per channel (configurable later) | Plenty for slow signals like temperature and battery voltage, and keeps the power low | Power budget, SD file size |
| S2 | Internal sampling rate | 1 kHz (then averaged to the 1 Hz logged value) | Fast enough to average out noise and mains hum at 50 Hz (Poland), slow enough to keep the processor load small | Timer and DMA setup, filter cutoff |
| S3 | Anti-aliasing filter | RC low-pass, cutoff about 100 Hz to 200 Hz, to be tuned in SPICE | Nyquist: sampling at 1 kHz means anything above 500 Hz would fold back as false readings, so it is filtered out first | Filter resistor and capacitor values |
| S4 | Log format | CSV text file: timestamp, ch1 (V), ch2 (V), temperature (C) | Opens directly in a spreadsheet or Python | Firmware file writing, SD card use |
| S5 | Timestamp | Seconds since start of the log (a real-time clock is a later option) | Simplest to implement first | Whether an external RTC and backup battery are needed |

### 3.4 User interface

| ID | Requirement | Value | Why this value | Affects |
|---|---|---|---|---|
| U1 | Display | 128x64 monochrome OLED, 0.96 inch, SSD1306 or SH1106 | Simple to drive, low memory use, cheap. A color display may follow in revision B | Display connector, I2C or SPI pins, power budget |
| U2 | Buttons | 4 (Up, Down, Select, Back) | The minimum for a usable menu | GPIO pins, debounce circuit and firmware |
| U3 | Display content | Live readings, start and stop of logging, battery level, SD status | What a user needs while logging | Menu state machine design |

### 3.5 Storage and PC connection

| ID | Requirement | Value | Why this value | Affects |
|---|---|---|---|---|
| D1 | Storage | microSD card, FAT filesystem | Cheap, large, and any PC can read it | SD socket, SPI pins (SDMMC later) |
| D2 | PC connection | USB-C: charging and power, plus USB data (serial first, mass storage later) | Standard modern connector. Serial is easy to start with | USB lines, ESD protection, USB-C resistors |

### 3.6 Power

| ID | Requirement | Value | Why this value | Affects |
|---|---|---|---|---|
| P1 | Battery | 1 cell Li-ion or LiPo, 3.7 V nominal, protected, capacity about 1000 mAh to 2000 mAh | Portable operation. A protected cell lowers the safety risk of the first revision | Charger and protection choice |
| P2 | Charging | Through USB-C, device usable while charging | Normal user expectation | Power-path charger IC |
| P3 | Runtime | At least 20 hours at the 1 Hz logging rate | A full day of logging. At 1000 mAh this means an average current of at most about 50 mA, and I aim for 25 mA or less to leave margin | Power budget, regulator type, display brightness and sleep modes |
| P4 | System voltage | 3.3 V regulated rail from a battery range of about 3.0 V to 4.2 V | The STM32, OLED and SD card all run at 3.3 V | LDO vs buck-boost decision (to be made after the power budget) |
| P5 | Battery monitoring | Battery voltage measured by the ADC through a high-resistance divider | Needed for the battery level display and low-battery shutdown | Divider values, leakage current in the budget |

### 3.7 Mechanical and general

| ID | Requirement | Value | Why this value | Affects |
|---|---|---|---|---|
| G1 | Board | 2-layer PCB if possible, hand-solderable parts (LQFP and 0603 or larger) | Cheap to make and possible to assemble and repair at home | Part and package choices |
| G2 | Test points | One for every power rail and for key analog nodes | Makes bring-up and debugging much easier | PCB layout |
| G3 | Programming | SWD header (and a USB bootloader later) | Needed to flash and debug the firmware | Header on the PCB |

---

## 4. Open questions (to decide during the project)

| Question | When to decide | How |
|---|---|---|
| LDO or buck-boost for the 3.3 V rail | After the power budget | Compare the runtime of both with the real current numbers |
| OLED over I2C or SPI | When the module arrives | Check the module, and choose the faster interface if it supports both |
| Is the internal 12-bit ADC accurate enough? | After SPICE and the first measurements | If not, consider an external 16-bit ADC in revision B |
| Is a real-time clock needed? | After the first firmware works | Only if absolute timestamps are wanted |
| Hardware filter for the buttons | During the schematic | Compare an RC filter with firmware-only debouncing |

## 5. Out of scope for version 1

Wireless (Wi-Fi or Bluetooth), color display, mains-powered operation, galvanic isolation, and measurements above 10 V. Wireless in particular would bring in the EU radio equipment rules (RED), which is better saved for a later project.

## 6. Change log

| Date | Change |
|---|---|
| 2026-10-08 | First draft (v0.1) |
