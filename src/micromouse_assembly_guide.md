# Micromouse Robot Assembly & Programming Guide

## Project Overview
This guide provides detailed instructions for assembling a Micromouse robot using the Flood Fill algorithm to solve mazes autonomously.

---

## Table of Contents
1. Hardware Components
2. Assembly Instructions
3. Wiring Connections
4. Algorithm Explanation
5. Complete Code Implementation
6. Testing Procedures
7. Troubleshooting

---

## 1. Hardware Components

### Main Components:
- **ESP32 Development Board** (microcontroller)
- **ESP32 Extension Board** (for easier pin access)
- **2x DC Motors with Encoders** (for movement and position feedback)
- **MPU6050** (6-axis gyroscope/accelerometer for orientation)
- **3x VL53L0X LIDAR Sensors** (time-of-flight distance sensors: left, front, right)
- **DRV8833 H-Bridge** (dual motor driver)
- **Buck-Boost Converter** (voltage regulation)
- **Battery Pack** (power supply)
- **Robot Base Chassis** (with wheels and caster ball)
- **Wiring Materials** (jumper wires, connectors)

### Pin Configuration Summary:
```
I2C Bus (SDA=21, SCL=22):
- MPU6050
- 3x VL53L0X LIDARs (multiplexed via XSHUT pins)

Motor Control:
- Motor 1: IN1=18, IN2=19
- Motor 2: IN3=23, IN4=5

Encoders:
- Motor 1: EncoderA=32, EncoderB=33
- Motor 2: EncoderA=25, EncoderB=26

LIDAR XSHUT Pins (for address configuration):
- Left LIDAR: XSHUT=27
- Front LIDAR: XSHUT=14
- Right LIDAR: XSHUT=12

LIDAR GPIO Pins (optional interrupt):
- Left: GPIO1=34
- Front: GPIO2=35
- Right: GPIO3=36

MPU6050:
- Interrupt: INT=4
- Address: 0x68 (AD0 to GND)

DRV8833:
- Sleep/Enable: EEP to 3.3V (always enabled)
```

---

## 2. Assembly Instructions

### Step 1: Prepare the Base Chassis
**Purpose:** Create stable platform for all components

1. **Install wheels on DC motors**
   - Press-fit or screw wheels onto motor shafts
   - Ensure wheels are secure and rotate freely
   
2. **Mount motors to chassis**
   - Position motors on left and right sides
   - Use motor brackets and screws
   - Ensure motors are parallel and wheels touch ground evenly
   
3. **Install caster ball**
   - Mount at rear center (or front) for stability
   - Adjust height so robot sits level

**Reason:** Proper mechanical alignment prevents drift and ensures accurate dead reckoning.

### Step 2: Mount Electronic Components
**Purpose:** Organize electronics for easy access and maintenance

1. **Mount ESP32 Extension Board**
   - Place centrally on chassis
   - Use standoffs to prevent shorts
   - Orient so USB port is accessible
   
2. **Install DRV8833 H-Bridge**
   - Mount near motors to minimize wire length
   - Ensure heat dissipation area is clear
   
3. **Position Sensors**
   - **Front LIDAR:** Mount at front center, facing forward
   - **Left LIDAR:** Mount on left side, facing 90° left
   - **Right LIDAR:** Mount on right side, facing 90° right
   - **MPU6050:** Mount flat and centered for accurate readings
   - Keep sensors away from vibration sources

**Reason:** Proper sensor placement ensures accurate maze detection. LIDAR sensors must be perpendicular to walls.

### Step 3: Power System Installation
**Purpose:** Provide stable power to all components

1. **Install Battery Pack**
   - Mount securely (consider weight distribution)
   - Use velcro or mounting brackets for easy removal
   
2. **Connect Buck-Boost Converter**
   - Input: Connect to battery pack
   - Output: Regulate to 5V for ESP32 and sensors
   - Mount away from motors to reduce electrical noise
   
3. **Wire DRV8833 Power**
   - Motor voltage input: Direct from battery (if 7.4V) or regulated
   - Logic voltage: 3.3V from ESP32
   - Connect EEP pin to 3.3V to enable driver

**Reason:** ESP32 requires 5V input, motors may need higher voltage. Buck-boost ensures stable power regardless of battery level.

### Step 4: Wiring Connections
**Purpose:** Connect all components following pin configuration

#### I2C Bus Wiring (SDA=21, SCL=22):
**Reason:** I2C allows multiple devices on same bus. Pull-up resistors often built into sensors.

1. **Connect all I2C devices in parallel:**
   ```
   ESP32 Pin 21 (SDA) → MPU6050 SDA → All 3 LIDAR SDA
   ESP32 Pin 22 (SCL) → MPU6050 SCL → All 3 LIDAR SCL
   ```

2. **LIDAR Address Configuration (CRITICAL):**
   **Reason:** All VL53L0X sensors default to 0x29. Must assign unique addresses.
   
   - Connect XSHUT pins:
     ```
     Left LIDAR XSHUT → Pin 27
     Front LIDAR XSHUT → Pin 14
     Right LIDAR XSHUT → Pin 12
     ```
   - Code will initialize sensors sequentially by:
     1. Holding all XSHUT LOW (sensors disabled)
     2. Enable one sensor, assign new address
     3. Repeat for others

#### Motor Control Wiring:
**Reason:** H-bridge enables bidirectional motor control with PWM speed regulation.

```
Motor 1 (Left):
  ESP32 Pin 18 → DRV8833 IN1
  ESP32 Pin 19 → DRV8833 IN2
  DRV8833 OUT1 → Motor 1 Terminal 1
  DRV8833 OUT2 → Motor 1 Terminal 2

Motor 2 (Right):
  ESP32 Pin 23 → DRV8833 IN3
  ESP32 Pin 5 → DRV8833 IN4
  DRV8833 OUT3 → Motor 2 Terminal 1
  DRV8833 OUT4 → Motor 2 Terminal 2
```

#### Encoder Wiring:
**Reason:** Encoders provide wheel rotation feedback for precise distance measurement.

```
Motor 1 Encoder:
  Encoder A → ESP32 Pin 32
  Encoder B → ESP32 Pin 33
  VCC → 5V
  GND → GND

Motor 2 Encoder:
  Encoder A → ESP32 Pin 25
  Encoder B → ESP32 Pin 26
  VCC → 5V
  GND → GND
```

**Note:** If encoders drift at 0, add 10kΩ pull-up resistors to encoder pins.

#### MPU6050 Wiring:
**Reason:** Gyroscope provides heading information to maintain straight movement and accurate turns.

```
MPU6050 VCC → 3.3V
MPU6050 GND → GND
MPU6050 SDA → Pin 21 (with other I2C devices)
MPU6050 SCL → Pin 22 (with other I2C devices)
MPU6050 INT → Pin 4 (optional, for interrupts)
MPU6050 AD0 → GND (sets address to 0x68)
```

#### Power Distribution:
```
Battery Pack → Buck-Boost Converter → 5V Rail
5V Rail → ESP32 VIN
5V Rail → Encoders VCC
3.3V (ESP32) → MPU6050 VCC, LIDAR VCC, DRV8833 Logic
Battery/Regulated → DRV8833 Motor Power (VM)
All GND → Common Ground
```

### Step 5: Cable Management
**Purpose:** Prevent interference and mechanical issues

1. Use zip ties or cable clips
2. Keep power wires away from signal wires
3. Ensure no wires contact moving parts (wheels, motors)
4. Leave slack for movement but prevent tangling

**Reason:** Clean wiring prevents shorts, reduces EMI, and improves reliability.

---

## 3. Algorithm Explanation: Flood Fill

### Why Flood Fill?
The Flood Fill algorithm is optimal for Micromouse because:

1. **Guaranteed Solution:** Always finds the goal if path exists
2. **Optimal Path:** Finds shortest path through iterative exploration
3. **Adaptive:** Updates maze knowledge dynamically
4. **Memory Efficient:** Only stores distance values per cell
5. **Competition Proven:** Industry standard for Micromouse

### How Flood Fill Works:

#### Conceptual Overview:
Imagine filling the maze with water from the goal. Each cell's "flood value" represents distance to goal. Robot always moves to neighboring cell with lowest value.

#### Detailed Steps:

1. **Initialization:**
   - Set goal cells (center 4 cells) to distance 0
   - All other cells start at maximum distance (255)
   - Assume all walls exist except outer boundary

2. **Flood Fill Update:**
   ```
   For each cell (starting from goal):
     neighbors = accessible cells without walls
     cell_distance = min(neighbor_distances) + 1
   Repeat until all distances stabilize
   ```

3. **Navigation:**
   ```
   While not at goal:
     1. Read walls with LIDARs
     2. Update maze map with discovered walls
     3. Recalculate flood fill values
     4. Move to neighbor with lowest distance
     5. Update current position
   ```

4. **Optimization:**
   - First run: Explore and map maze
   - Subsequent runs: Use known map for fastest path
   - Robot returns to start automatically for speed runs

### Maze Coordinate System:
```
North (N) = 0
East (E)  = 1
South (S) = 2
West (W)  = 3

Turn Right: heading = (heading + 1) % 4
Turn Left:  heading = (heading + 3) % 4
Turn Back:  heading = (heading + 2) % 4
```

### Movement Strategy:
1. **Cell-by-Cell Movement:** Robot always centers in cells
2. **Wall Detection:** Check walls before entering each cell
3. **Heading Correction:** Use gyroscope to maintain 0°, 90°, 180°, 270°
4. **Distance Tracking:** Use encoders to measure exact travel distance

---

## 4. Complete Code Implementation

### Code Structure:
The complete code includes:
1. **Sensor Initialization:** MPU6050, VL53L0X LIDARs with address assignment
2. **Motor Control:** PID-controlled movement with encoders
3. **Maze Representation:** 16x16 grid with wall tracking
4. **Flood Fill Algorithm:** Distance calculation and pathfinding
5. **State Machine:** Coordinated movement through maze

### Key Features:
- **Precise Movement:** PID control ensures straight lines and accurate turns
- **Wall Detection:** Three LIDARs detect walls in all directions
- **Gyroscope Integration:** Maintains heading through turns
- **Encoder Feedback:** Measures exact distance traveled
- **Automatic Centering:** Aligns robot in cell center before turns
- **Speed Optimization:** Smooth acceleration/deceleration

---

## 5. Testing Procedures

### Phase 1: Individual Component Testing
1. **Motors:** Test forward, backward, speed control
2. **Encoders:** Verify count accuracy
3. **Gyroscope:** Check angle tracking, calibration
4. **LIDARs:** Confirm all three sensors work, measure distances

### Phase 2: Movement Calibration
1. **Straight Line:** Drive exact 168mm (one cell)
2. **Turn Accuracy:** Verify 90° turns
3. **Cell Centering:** Ensure robot stops at cell center
4. **Speed Ramping:** Test smooth acceleration

### Phase 3: Maze Testing
1. **Simple Maze:** Test 3x3 known maze
2. **Wall Detection:** Verify correct wall identification
3. **Flood Fill:** Confirm algorithm finds goal
4. **Speed Run:** Test optimized path execution

---

## 6. Troubleshooting

### Robot Drifts While Moving Straight:
- Check: Motor speeds equal? Gyroscope calibrated?
- Fix: Adjust PID constants, ensure calibration at rest

### Encoders Show Zero Count:
- Check: Pull-up resistors on pins 32,33,25,26?
- Fix: Add 10kΩ resistors to 3.3V

### LIDAR Sensors Don't Initialize:
- Check: I2C connections? XSHUT pins wired correctly?
- Fix: Verify I2C scanner detects devices, check address assignment

### Robot Turns Incorrect Angle:
- Check: Gyroscope drift? Turn speed too fast?
- Fix: Recalibrate MPU6050, reduce turn speed

### Battery Dies Quickly:
- Check: Buck-boost efficiency? Motor current draw?
- Fix: Use higher capacity battery, reduce motor speed

### Robot Gets Lost in Maze:
- Check: Encoder accuracy? Wall detection threshold?
- Fix: Calibrate CELL_DISTANCE, adjust WALL_THRESHOLD

---

## 7. Performance Optimization

### Speed Tuning:
- Start with slow speeds (100-150 PWM)
- Gradually increase after verifying accuracy
- Competition speeds: 200-255 PWM

### PID Tuning:
- Start with proportional only (P)
- Add derivative (D) to reduce oscillation
- Add integral (I) if steady-state error exists

### Weight Distribution:
- Place battery low and centered
- Keep heavy components near center of rotation
- Minimize moment of inertia

---

## Success Criteria

Your Micromouse is ready when:
1. ✓ Moves straight for multiple cells without drift
2. ✓ Turns exactly 90° consistently
3. ✓ Detects walls accurately with LIDARs
4. ✓ Completes simple maze in exploration mode
5. ✓ Executes optimized speed run under 60 seconds

Good luck with your Micromouse competition!
