# ESP32 Binary LED Counter with ADC Speed Control and Watchdog

A mini embedded systems project built with ESP-IDF for ESP32-S3.

This project demonstrates:

- Bitwise operations
- Binary number visualization using LEDs
- ADC input with a potentiometer
- GPIO interrupts
- Button debounce
- FreeRTOS timing
- Task Watchdog Timer (TWDT)
- Intentional watchdog reset handling

---

# Features

- 4-bit binary LED counter
- Potentiometer-controlled counter speed
- GPIO interrupt button handling
- Interrupt debounce protection
- Watchdog monitoring
- Intentional system freeze simulation
- Automatic ESP32 restart after watchdog timeout

---

# Hardware Components

- ESP32-S3 development board
- 4 LEDs
- 4 resistors
- Potentiometer
- Push button
- Breadboard
- Jumper wires

---

# Pin Configuration

| Component | GPIO |
|---|---|
| LED 1 | GPIO15 |
| LED 2 | GPIO16 |
| LED 3 | GPIO17 |
| LED 4 | GPIO18 |
| Button | GPIO19 |
| Potentiometer Output | GPIO1 |

---

# Binary Counter Logic

The LEDs display numbers from `0` to `15` in binary format.

---

# Bitwise Operations

The project uses bit masks to determine which LEDs should be enabled.

```cpp
value & (1 << bit)
```

Example:

```text
5 = 0101
```

Checking bit 2:

```text
0101
&
0100
=
0100
```

Since the result is not zero, the LED is enabled.

---

# ADC Speed Control

The potentiometer controls the binary counter speed.

ADC range:

```text
0 - 4095
```

Mapped delay range:

```text
100 ms - 1000 ms
```

The ADC value is converted into a delay value using:

```cpp
mapAdcToDelayMs()
```

---

# Interrupt Debounce

Mechanical buttons generate noisy signals called bounce.

To avoid multiple interrupts from a single press, debounce logic is implemented:

```cpp
(now - lastInterruptTime) > pdMS_TO_TICKS(50)
```

The interrupt is accepted only if at least 50 ms passed since the previous valid interrupt.

---

# Watchdog Demonstration

The project uses the ESP-IDF Task Watchdog Timer.

The main task periodically resets the watchdog:

```cpp
esp_task_wdt_reset();
```

When the button is pressed:

```cpp
simulateSystemFreeze();
```

The program enters an infinite loop:

```cpp
while (true)
{
}
```

Since the watchdog is no longer reset, the ESP32 automatically restarts.

---

# Task Watchdog Configuration

The project uses:

```cpp
esp_task_wdt_reconfigure()
```

instead of:

```cpp
esp_task_wdt_init()
```

because the Task Watchdog may already be initialized by ESP-IDF.

---

# Main Concepts Practiced

- ESP-IDF GPIO driver
- ADC One-shot driver
- FreeRTOS task timing
- Interrupt Service Routines (ISR)
- Button debounce
- Bitwise operations
- Binary representation
- Watchdog monitoring
- Embedded C++ architecture
- Non-blocking embedded design

---

# Expected Behavior

1. The board starts and prints:

```text
App started
```

2. LEDs display a binary counter from `0` to `15`
3. Rotating the potentiometer changes the counter speed
4. Pressing the button triggers a system freeze
5. The watchdog detects the freeze
6. ESP32 automatically restarts
7. `App started` appears again in Serial Monitor

---

---

# Technologies Used

- ESP-IDF
- FreeRTOS
- PlatformIO
- Embedded C++

---

# Notes

The project intentionally avoids Arduino abstractions to practice lower-level embedded development concepts using ESP-IDF APIs.

The ISR is intentionally lightweight and only sets a flag. All heavy operations are handled in the main task context.

```
