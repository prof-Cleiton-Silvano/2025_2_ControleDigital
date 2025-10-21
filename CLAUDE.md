# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP32 firmware for digital control systems using PlatformIO, Arduino framework, and FreeRTOS. The project implements a task-based architecture with hardware abstraction layers (HAL) and inter-task communication via FreeRTOS queues.

## Build and Development Commands

### Compile
```bash
pio run
```

### Upload to ESP32
```bash
pio run --target upload
```

### Serial Monitor
```bash
pio device monitor
```
Monitor runs at 115200 baud (configured in platformio.ini).

### Clean Build
```bash
pio run --target clean
```

## Architecture Overview

### Initialization Flow
1. `setup()` in `src/main.cpp` calls `hal::initBoard()` to initialize all hardware peripherals
2. Individual FreeRTOS tasks are created via factory functions in the `tasks` namespace
3. `loop()` remains idle (`vTaskDelay(portMAX_DELAY)`) - all work is delegated to the FreeRTOS scheduler

### Module Structure

**HAL Layer** (`src/hal/board.*` and `include/hal/board.h`):
- Provides hardware abstraction for GPIO pins and peripherals
- Currently manages: builtin LED, 3 user buttons (GPIO 32, 33, 25 with pull-up), 2 limit switch buttons (GPIO 26, 27 with pull-down)
- All hardware access should go through HAL functions, never directly in tasks
- Button reading functions return `true` when pressed (accounting for pull-up/pull-down logic)

**Task Layer** (`src/tasks/*` and `include/tasks/*`):
- Each task is implemented as a separate module with header in `include/tasks/` and implementation in `src/tasks/`
- Tasks expose a `start*Task(UBaseType_t priority)` factory function
- Existing tasks:
  - `blink_task`: Simple LED blink example (low priority)
  - `display_task`: Manages I2C LCD (16x2, address 0x27) with FreeRTOS queue for incoming messages
  - `touch_task`: Reads capacitive touch sensor (T0/GPIO4) and sends state to display via queue
  - `stepper_task`: Controls stepper motor via TB6600 driver with queue-based commands (high priority for precise timing)

### Inter-Task Communication

**Display Task Queue** (`xDisplayQueue`):
- Queue size: 16 messages
- Message type: `DisplayMessage` struct
- Commands: `WriteChar` (write character at col/row) or `Clear` (clear screen)
- Other tasks send messages via `sendDisplayMessage()` function

**Stepper Task Queue** (`xStepperQueue`):
- Queue size: 8 messages
- Message type: `StepperMessage` struct
- Fields: `steps` (number of steps), `intervalUs` (microseconds between steps), `direction` (Clockwise/CounterClockwise)
- Other tasks send commands via `sendStepperMessage()` function
- Example: `StepperMessage msg{1600, 500, StepperDirection::Clockwise};` moves 1600 steps at 500us intervals

This queue-based pattern should be used for any task requiring asynchronous input from multiple sources.

## Hardware Configuration

### I2C LCD Display
- **Pins**: SDA = GPIO 21, SCL = GPIO 22 (ESP32 defaults)
- **Address**: 0x27 (if display doesn't work, try 0x3F)
- **Size**: 16 columns x 2 rows
- Pull-up resistors are internal (no external resistors needed)

### Buttons
- **User Buttons**: GPIO 32, 33, 25 (INPUT_PULLUP, read LOW when pressed)
- **Limit Switches**: GPIO 26, 27 (INPUT_PULLDOWN, read HIGH when activated)

### Touch Sensor
- **Pin**: T0 (GPIO 4)
- **Threshold**: Values < 40 indicate touch (tune as needed for your hardware)

### Stepper Motor (TB6600 Driver)
- **PUL/STEP Pin**: GPIO 18 (pulse signal for each step)
- **DIR Pin**: GPIO 19 (direction control: HIGH = clockwise, LOW = counter-clockwise)
- **ENA Pin**: GPIO 23 (enable control: LOW = motor enabled, HIGH = motor disabled)
- **Minimum Pulse Interval**: 50 microseconds (enforced in software to prevent motor stalling)
- **Pulse Width**: 5 microseconds (TB6600 requires minimum 2.5us)

### LED
- **Builtin LED**: GPIO 2 (defined via `-DLED_BUILTIN=2` in platformio.ini)

## Development Conventions

### Creating New Tasks
1. Add header file in `include/tasks/` with a `start*Task(UBaseType_t priority)` declaration
2. Implement task function in `src/tasks/` as static/anonymous namespace function
3. Task function signature: `void taskName(void* params)`
4. Call `xTaskCreate()` in the `start*Task()` factory function with appropriate stack size
5. Register task startup in `src/main.cpp` within `setup()`

### Hardware Access
- **Never** access GPIO pins directly from tasks using `pinMode()`, `digitalWrite()`, etc.
- Always add abstraction functions to `hal/board.*` and expose them in the header
- Define hardware constants (pin numbers, timings, addresses) in HAL headers for reusability

### Task Priorities
- Use `tskIDLE_PRIORITY + N` for priority assignment
- Low priority tasks (display, blink, touch): `tskIDLE_PRIORITY + 1`
- High priority tasks (stepper motor): `tskIDLE_PRIORITY + 3` for precise timing
- Critical tasks (if needed): `tskIDLE_PRIORITY + 4` or higher
- The stepper task uses high priority to ensure accurate step timing and prevent motor stalling

### Stack Sizes
- Simple tasks (like blink): 2048 words
- Tasks with I2C/LCD operations: 4096 words
- Adjust based on local variable usage and call depth

### Debugging
- Core debug level is set to 1 via `-DCORE_DEBUG_LEVEL=1` in platformio.ini
- Use `Serial.print()` for debugging, but ensure serial is initialized if not using display

## Dependencies

External libraries (managed via PlatformIO):
- `marcoschwartz/LiquidCrystal_I2C@^1.1.4`: I2C LCD driver

## Planned Extensions

Future modules should follow the established patterns:
- Sensor reading tasks with message queues to publish samples
- Control tasks that process sensor data and actuate outputs via HAL
- Communication tasks (UART/WiFi/BLE) for telemetry and remote monitoring
- Each module should be isolated, testable, and communicate via queues or shared resources protected by mutexes
