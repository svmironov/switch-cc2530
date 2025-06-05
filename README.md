# ZigBee 2-gnag switch fimware

**ZigBee Home Automation Sample Application based on TI Z-Stack 3.0.2**

This project implements a custom ZigBee device using ZCL (ZigBee Cluster Library) with two independent switches supporting On/Off and Level Control functionality.  
It is designed for embedded development and hardware based on TI CC2530 SoC.

---

## Features

- ✅ Two independent endpoints (`APP_ENDPOINT_1` and `APP_ENDPOINT_2`)
- ✅ On/Off control with short press
- ✅ Level Control (brightness up/down) with long press
- ✅ Binding support (both unicast and group bindings)
- ✅ Long press detection with timers (`osal_start_timerEx`)
- ✅ Reconnection handling for End Devices (`bdb_ZedAttemptRecoverNwk()`)

---

## Hardware

- **MCU**: Texas Instruments CC2530
- **Stack**: Z-Stack 3.0.2 (Home Automation Profile)

## Button Mapping and Pins

| Button | Function | GPIO Pin |
|--------|----------|----------|
| SW1    | Start network commissioning (Commissioning Mode) | P0.1 |
| SW3    | Endpoint 1: ON (short press), Brightness Up (long press) | P0.3 |
| SW4    | Endpoint 1: OFF (short press), Brightness Down (long press) | P2.0 |
| SW5    | Endpoint 2: ON (short press), Brightness Up (long press) | P2.1 |
| SW6    | Endpoint 2: OFF (short press), Brightness Down (long press) | P2.2 |

> ⚠ **Note:** GPIO pin mapping may vary depending on your specific hardware configuration. Update `hal_board_cfg.h` accordingly.

---

## Build Instructions

1. Install **IAR Embedded Workbench for 8051**, version 8.20.2 (recommended for Z-Stack 3.0.2).
2. Install **TI Z-Stack 3.0.2**.
3. Import the project into IAR and configure include paths to Z-Stack libraries.
4. Build and flash the firmware to CC2530 using a programmer (e.g., CC Debugger).

## License

This project is provided as a sample application based on TI Z-Stack 3.0.2 and may be used for educational and development purposes.

---

**Note:** Before building, verify that all Z-Stack 3.0.2 dependencies are correctly integrated into the project environment.
