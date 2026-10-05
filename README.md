# Portable STM32 Data Logger

A battery-powered data logger with an OLED screen and buttons, designed from scratch: SPICE-simulated analog front-end, custom KiCad PCB, and bare-metal STM32 firmware.

**Goal:** learn the full embedded engineering workflow (analog design, schematic and PCB, firmware, testing) on one complete project, with an eye on EU/Polish compliance (CE, EMC, RoHS) for later.

---

## 1. Specification (v0.1, draft)

| Area | Choice | Notes |
|---|---|---|
| MCU | STM32G431 (LQFP48 or LQFP64) | Cortex-M4, good ADCs, built-in op-amps, hand-solderable |
| Dev board | NUCLEO-G431RB (~15 EUR) | Firmware work starts here before the PCB exists |
| Inputs | 2x voltage (0-10 V), 1x NTC thermistor (10k) | Front-end: protection, divider, RC filter, buffer |
| Display | 0.96" or 1.3" OLED, 128x64, I2C or SPI (SSD1306 / SH1106) | Start with a ready-made module on a pin header |
| Buttons | 4x (Up, Down, Select, Back) | Hardware RC filter + firmware debounce |
| Storage | microSD (SPI to start, SDMMC later) | FAT filesystem (FatFs), CSV log files |
| PC link | USB-C: charging, power, and USB data (CDC serial or mass storage) | ESD protection on the USB lines |
| Power | 1x Li-ion/LiPo cell (3.7 V) with USB-C charging | Power-path charger, 3.3 V rail, battery voltage monitoring |
| Target runtime | 20+ hours at a 1 Hz logging rate | To be confirmed with a power budget |

### Battery design notes
- Use a **protected** single-cell LiPo or 18650 (built-in protection circuit) for the first revision.
- **Charger:** a power-path IC (e.g. BQ24074 or similar) so the device can run from USB while charging.
- **3.3 V rail:** a low-Iq buck-boost (e.g. TPS63001 / TPS63802 class) uses the full battery range (about 3.0 to 4.2 V). A plain LDO is simpler but wastes the last part of the battery capacity. Decide this during the SPICE phase.
- **Battery voltage** is measured through a high-impedance divider into an ADC pin.
- **Safety:** never charge or short unprotected cells. Fire risk is real, so handle LiPo carefully.

---

## 2. Project phases

### Phase 0: Setup (week 1)
- [ ] Create a Git repository with this README
- [ ] Install STM32CubeIDE or set up CMake + arm-none-eabi-gcc + OpenOCD
- [ ] Install KiCad (latest stable) and LTspice
- [ ] Order a NUCLEO-G431RB and an OLED module
- [ ] Finalize the spec (channels, accuracy, sample rate)

### Phase 1: Firmware on the dev board (weeks 2-6)
- [ ] Blink an LED, set up the toolchain and debugger
- [ ] UART printf logging
- [ ] ADC sampling with DMA and timer triggering
- [ ] Thermistor conversion (Steinhart-Hart or beta equation)
- [ ] OLED driver (I2C or SPI) and a simple text UI
- [ ] Button handling (interrupts or polling + debounce)
- [ ] Menu state machine
- [ ] SD card logging with FatFs
- [ ] USB CDC or mass storage
- [ ] Unit tests for pure logic (unit conversion, menu state machine) on the PC

### Phase 2: Analog and power simulation (weeks 3-6, in parallel)
- [ ] Input divider and protection (ESD, overvoltage clamp) in LTspice
- [ ] RC anti-aliasing filter; check against the sample rate
- [ ] Thermistor circuit: sensitivity and linearity
- [ ] Power budget spreadsheet (every part's current in each mode)
- [ ] Choose LDO vs buck-boost based on the budget
- [ ] Battery gauge divider and its leakage current

### Phase 3: Schematic and PCB (weeks 6-10)
- [ ] Read the STM32G4 hardware design guide (AN5093 or the current equivalent)
- [ ] Schematic: power, MCU, analog inputs, USB, SD, display header, buttons
- [ ] ERC clean, with a second pair of eyes (post it for review on an electronics forum)
- [ ] PCB layout: 2 layers or 4 layers, separate analog and digital areas, solid ground
- [ ] DRC clean against the manufacturer's rules (JLCPCB, PCBWay, or a Polish fab)
- [ ] BOM, Gerbers, 3D check, and a test point for every power rail

### Phase 4: Build and bring-up (weeks 10+)
- [ ] Order PCB and parts
- [ ] Power-up in stages: no MCU fitted first, check the rails, then fit the MCU
- [ ] Flash the firmware and fix the hardware differences
- [ ] Measure: accuracy, noise, battery runtime
- [ ] Write the lessons learned and plan revision B

---

## 3. Repository layout (suggested)

```
/docs         spec, power budget, datasheets (links), design notes
/firmware     STM32 source code, tests
/hardware
  /kicad      schematic and PCB
  /spice      LTspice simulations
/manufacturing  Gerbers, BOM, assembly notes
```

---

## 4. Learning resources

- STM32G4 reference manual (RM0440) and the datasheet
- ST application note on STM32 hardware development (AN5093 for G4)
- KiCad official getting started guide
- LTspice tutorials from Analog Devices
- *Making Embedded Systems* by Elecia White (firmware design)
- *The Art of Electronics* by Horowitz and Hill (reference for the analog side)

---

## 5. Compliance notes (for later)

Not required for a personal prototype, but worth learning along the way:
- **CE marking**, **EMC Directive**, **RoHS** (and **RED** if wireless is added later)
- Battery-related rules: a Li-ion cell must meet transport standard UN 38.3, and a product with a battery has extra rules (e.g. the EU Battery Regulation)
- Design habits that help later: ESD protection on connectors, filtered power inputs, a clean ground plane

---

## 6. Decisions log

| Date | Decision | Reason |
|---|---|---|
| 2026-10-05 | STM32 over ESP32 | More common in professional embedded work |
| 2026-10-05 | MCU (STM32G4) for project 1, MPU later | Avoids DDR and BGA complexity on the first board |
| 2026-10-05 | OLED display first, TFT maybe in revision B | Simpler to drive, less memory needed |
| 2026-10-05 | Battery power added (single-cell Li-ion) | Makes the device truly portable |
