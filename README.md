# Anemometer Portable

A portable anemometer built with an Optocoupler sensor for wind speed measurement and HMC5883L magnetometer for wind direction, controlled by an Arduino UNO R3 microcontroller with automatic MicroSD data logging.

> Undergraduate thesis project — Atmospheric and Planetary Science, ITERA 2026

---

## Description

This project presents the hardware design, firmware, and documentation for a low-cost portable anemometer. The device uses an Optocoupler sensor to count rotor pulses from a 3-cup anemometer and an HMC5883L magnetometer to detect wind direction via a rotating wind vane with an embedded magnet. All measurements are timestamped using a DS3231 RTC module and logged to a MicroSD card every 5 seconds.

The instrument was calibrated against a Davis AWS standard anemometer, achieving a post-calibration uncertainty of ±0.3046 m/s and a wind direction accuracy of CSI = 1.00 across 8 cardinal directions using the Lavdas method.

---

## Specifications

### System Overview

| Parameter | Value |
|---|---|
| Type | 3-cup portable anemometer with wind vane |
| Microcontroller | Arduino UNO R3 (ATMega328P, 16 MHz) |
| Power Supply | 2× Li-ion 18650 battery |
| Data Storage | MicroSD card (TXT format, FAT32) |
| Communication | I2C (sensors + RTC), SPI (MicroSD) |

### Wind Speed — Optocoupler Sensor

| Parameter | Value |
|---|---|
| Sensor | Optocoupler LM393 |
| Encoder | Disk encoder, 16 slots, D = 60 mm |
| Cup type | Hemisphere, D = 40 mm |
| Arm length | 60 mm |
| Bearing | 608zz |
| Calibrated speed range | 1.2–5.5 m/s |
| Minimum threshold | 0.43 m/s |
| Uncertainty (95%) | ±0.3046 m/s |
| Calibration equation | y = 10.51x + 0.43 |
| Pearson r (post-calibration) | 0.9556 |
| RMSE (post-calibration) | 0.1523 m/s |
| Reference standard | Davis AWS anemometer |
| Operating voltage | 3.3–5V |

### Wind Direction — HMC5883L Magnetometer

| Parameter | Value |
|---|---|
| Sensor | HMC5883L 3-axis magnetometer |
| Measurement range | ±1 to ±8 gauss |
| Angular resolution | 1–2° |
| Operating voltage | 2.7–6.5V |
| Interface | I2C |
| Directions supported | 8 (N, NE, E, SE, S, SW, W, NW) |
| CSI (Lavdas method) | 1.00 |
| POD | 1.00 |
| FAR | 0.00 |

### Data Logging

| Parameter | Value |
|---|---|
| Sampling rate | 1 second |
| Output interval | Every 5 seconds (average of 5 samples) |
| SD write interval | Every 1 minute (12 data points per write) |
| RTC module | DS3231 |
| Output format | `DD/MM/YYYY HH:MM:SS  speed(m/s)  direction(°)` |

### Physical Dimensions

| Part | Dimension |
|---|---|
| Housing 1 (main box) | 96 × 84 × 105 mm |
| Housing 2 (top box) | 96 × 84 × 48 mm |
| Support pipe | H = 123 mm |
| Wind vane blade | 102 × 75 mm |
| Wind vane arm | L = 150 mm |
| Magnet holder | D = 5 mm |
| Estimated total print time | ~12–30 hours (depends on printer settings) |

---

## Bill of Materials

| No. | Component | Qty | Notes |
|---|---|---|---|
| 1 | Arduino UNO R3 | 1 | ATMega328P, 16 MHz |
| 2 | Optocoupler LM393 | 1 | Digital output, 3.3–5V |
| 3 | HMC5883L Magnetometer | 1 | I2C, 2.7–6.5V, 1–2° accuracy |
| 4 | RTC DS3231 | 1 | I2C timekeeping module |
| 5 | MicroSD Adapter + SD Card | 1 | SPI interface, FAT32 |
| 6 | 18650 Battery + Holder | 2 | Rechargeable Li-ion |
| 7 | Jumper Wires | — | Male-to-male & male-to-female |
| 8 | Bearing 608zz | 2 | Cup axis + wind vane axis |
| 9 | Neodymium Magnet | 1 | For wind vane, D ±5 mm |
| 10 | 3D Printed Parts | — | See `hardware/stl/` |

---

## Wiring

```
Arduino UNO R3
├── Digital Pin 2  →  Optocoupler (OUT)
├── SDA (A4)       →  HMC5883L (SDA) + DS3231 (SDA)
├── SCL (A5)       →  HMC5883L (SCL) + DS3231 (SCL)
├── Pin 10         →  MicroSD (CS)
├── Pin 11         →  MicroSD (MOSI)
├── Pin 12         →  MicroSD (MISO)
├── Pin 13         →  MicroSD (SCK)
├── 5V             →  VCC all sensors
└── GND            →  GND all sensors
```

See full wiring diagram in [`hardware/schematic/`](hardware/schematic/)

---

## 3D Printing

STL files are available in [`hardware/stl/`](hardware/stl/)

> **Note:** Print time estimates below are approximate and may vary
> significantly depending on printer model, speed settings, and slicer
> configuration. Total estimated time ranges from 12–30 hours.

| File | Part | Est. Print Time |
|---|---|---|
| `housing_1.stl` | Main box (96×84×105 mm) | ~4 hrs |
| `housing_2.stl` | Top box (96×84×48 mm) | ~2 hrs |
| `backdoor.stl` | Back door | ~30 min |
| `top_cover.stl` | Top cover | ~30 min |
| `support_pipe.stl` | Support pipe (H=123 mm) | ~1 hr |
| `disk_encoder.stl` | 16-slot encoder disk (D=60 mm) | ~45 min |
| `cup_anemometer.stl` | 3× hemisphere cup (D=40 mm) | ~1.5 hrs |
| `wind_vane.stl` | Wind vane (arm 150 mm, blade 102×75 mm) | ~1 hr |
| `magnet_holder.stl` | Magnet holder (D=5 mm) | ~15 min |

Recommended print settings:
- Material: PLA or PETG
- Layer height: 0.2 mm
- Infill: 20–30%
- Support: required for `cup_anemometer.stl` and `wind_vane.stl`

---

## Assembly

### Mechanical

1. Install both bearing 608zz — one into the cup anemometer dock 
   and one into the magnet holder
2. Before attaching the support pipe, connect the disk encoder 
   to the bottom of the cup anemometer
3. Install the neodymium magnet into the inner wall of the 
   magnet holder
4. Insert the support pipe through the cup anemometer bearing, 
   the magnet holder bearing, and dock it into housing 2
5. Assemble the remaining parts following the 3D design — 
   the final result should match the device photo in `docs/images/`

### Electronics

1. Place the Optocoupler sensor inside housing 2, positioned 
   directly at the disk encoder rotation path
2. Route the HMC5883L cable through the support pipe before 
   final assembly — the sensor sits statically at the center 
   of the magnet holder while the magnet holder itself rotates 
   with the wind vane above it
3. Connect all remaining components according to the wiring 
   diagram
4. Place the assembled electronics inside housing 1
5. Ensure cables do not obstruct the disk encoder or the 
   Optocoupler gap

### Firmware Upload

1. Install Arduino IDE
2. Install all required libraries (see Dependencies)
3. Open `firmware/anemometer_main/anemometer_main.ino`
4. Select board: **Arduino UNO** and the correct port
5. Click **Upload**

---

## Dependencies

Install via Arduino IDE Library Manager:

| Library | Author |
|---|---|
| DS3232RTC | Jack Christensen |
| Streaming | Mikal Hart |
| Adafruit HMC5883 Unified | Adafruit |
| Adafruit Unified Sensor | Adafruit |

Built-in (no installation required): `SD`, `SPI`, `Wire`

---

## Getting Started

### First Time Setup

**1. Set RTC time**

Open Serial Monitor at baud rate **9600** and send a string in the following format:
```
YYMMDDHHmmss
```
Example — May 11, 2026, 08:30:00:
```
260511083000
```

**2. Prepare SD Card**

- Format the SD card to **FAT32**
- Insert into the MicroSD adapter slot
- `datalog.txt` is created automatically on first boot

**3. Power on**

- Ensure 18650 batteries are fully charged
- Power on the device — data logging starts automatically

### Reading Data

Data is stored in `datalog.txt` in the following format:
```
DD/MM/YYYY HH:MM:SS  velocity(m/s)  direction(°)
```

Example output:
```
11/05/2026 08:30:00  1.23456  270.3
11/05/2026 08:30:05  1.45678  268.7
11/05/2026 08:30:10  0.00000  271.0
```

A velocity of `0.00000` indicates wind speed below the minimum threshold (< 0.43 m/s).

---

## Working Principle

**Wind Speed:**
```
v_raw       = (π × d × pulses / n_slots) / Δt
v_corrected = (10.51 × v_raw) + 0.43
```

**Wind Direction:**
```
θ       = atan2(Y, X) → converted to azimuth 0–360°
azimuth = (360 - θ) % 360   ← corrected for CCW magnet rotation
```

---

## Repository Structure

```
anemometer-portable/
├── README.md
├── firmware/
│   └── anemometer_main/
│       └── anemometer_main.ino
├── hardware/
│   ├── stl/               <- 3D print files (.stl)
│   └── schematic/         <- Wiring diagram
└── docs/
    └── images/            <- Device photos
```

---

## Authors

**Nauval Nadier Nashrullah**
Atmospheric and Planetary Science, ITERA — 2026
NIM: 122290047

Advisors: Alfiah Rizky Diana Putri, S.T., M.Eng. & Alvin Pratama, S.Si., M.T.

---

## License

This project is licensed under the MIT License.

---
