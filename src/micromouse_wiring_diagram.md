# Micromouse Detailed Wiring Diagram

## Complete Pin Connection Reference

---

## 1. ESP32 Pin Assignments

### I2C Bus (Shared Communication)
```
ESP32 Pin 21 (SDA) ──┬── MPU6050 SDA
                     ├── Left LIDAR SDA
                     ├── Front LIDAR SDA
                     └── Right LIDAR SDA

ESP32 Pin 22 (SCL) ──┬── MPU6050 SCL
                     ├── Left LIDAR SCL
                     ├── Front LIDAR SCL
                     └── Right LIDAR SCL
```

### Motor Control Outputs
```
Motor 1 (Left Motor):
  ESP32 Pin 18 (PWM) → DRV8833 IN1
  ESP32 Pin 19 (PWM) → DRV8833 IN2

Motor 2 (Right Motor):
  ESP32 Pin 23 (PWM) → DRV8833 IN3
  ESP32 Pin 5  (PWM) → DRV8833 IN4
```

### Encoder Inputs
```
Motor 1 Encoder:
  ESP32 Pin 32 (Input) ← Encoder A (Channel A)
  ESP32 Pin 33 (Input) ← Encoder B (Channel B)

Motor 2 Encoder:
  ESP32 Pin 25 (Input) ← Encoder A (Channel A)
  ESP32 Pin 26 (Input) ← Encoder B (Channel B)
```

### LIDAR Control Pins (XSHUT for Address Assignment)
```
ESP32 Pin 27 → Left LIDAR XSHUT
ESP32 Pin 14 → Front LIDAR XSHUT
ESP32 Pin 12 → Right LIDAR XSHUT
```

### Optional Pins (For Advanced Features)
```
ESP32 Pin 4  → MPU6050 INT (Interrupt, optional)
ESP32 Pin 34 → Left LIDAR GPIO1 (Interrupt, optional)
ESP32 Pin 35 → Front LIDAR GPIO2 (Interrupt, optional)
ESP32 Pin 36 → Right LIDAR GPIO3 (Interrupt, optional)
```

---

## 2. DRV8833 H-Bridge Connections

### Power Connections
```
VM (Motor Voltage)  ← Battery voltage (7.4V typical)
VCC (Logic Voltage) ← ESP32 3.3V
GND                 ← Common ground
EEP (Sleep/Enable)  ← ESP32 3.3V (always enabled)
```

### Motor Outputs
```
OUT1 → Motor 1 Terminal +
OUT2 → Motor 1 Terminal -
OUT3 → Motor 2 Terminal +
OUT4 → Motor 2 Terminal -
```

### Control Inputs
```
IN1 ← ESP32 Pin 18
IN2 ← ESP32 Pin 19
IN3 ← ESP32 Pin 23
IN4 ← ESP32 Pin 5
```

**Truth Table:**
```
IN1  IN2  →  Motor 1 Action
HIGH LOW  →  Forward
LOW  HIGH →  Reverse
LOW  LOW  →  Brake/Coast
HIGH HIGH →  Brake
```

---

## 3. MPU6050 Connections

```
VCC → ESP32 3.3V
GND → Common ground
SDA → ESP32 Pin 21 (with other I2C devices)
SCL → ESP32 Pin 22 (with other I2C devices)
INT → ESP32 Pin 4 (optional)
AD0 → GND (sets I2C address to 0x68)
```

**I2C Address:**
- AD0 = GND → Address 0x68 (used in code)
- AD0 = 3.3V → Address 0x69

---

## 4. VL53L0X LIDAR Sensors (×3)

### Left LIDAR
```
VIN   → ESP32 3.3V
GND   → Common ground
SDA   → ESP32 Pin 21 (shared I2C bus)
SCL   → ESP32 Pin 22 (shared I2C bus)
XSHUT → ESP32 Pin 27
GPIO1 → ESP32 Pin 34 (optional)
```
**Assigned Address:** 0x30

### Front LIDAR
```
VIN   → ESP32 3.3V
GND   → Common ground
SDA   → ESP32 Pin 21 (shared I2C bus)
SCL   → ESP32 Pin 22 (shared I2C bus)
XSHUT → ESP32 Pin 14
GPIO1 → ESP32 Pin 35 (optional)
```
**Assigned Address:** 0x31

### Right LIDAR
```
VIN   → ESP32 3.3V
GND   → Common ground
SDA   → ESP32 Pin 21 (shared I2C bus)
SCL   → ESP32 Pin 22 (shared I2C bus)
XSHUT → ESP32 Pin 12
GPIO1 → ESP32 Pin 36 (optional)
```
**Assigned Address:** 0x32

**Note:** All VL53L0X sensors default to address 0x29. The XSHUT pins are used during initialization to assign unique addresses.

---

## 5. Encoder Connections

### Motor 1 Encoder
```
VCC   → 5V (regulated)
GND   → Common ground
OUT A → ESP32 Pin 32
OUT B → ESP32 Pin 33
```

### Motor 2 Encoder
```
VCC   → 5V (regulated)
GND   → Common ground
OUT A → ESP32 Pin 25
OUT B → ESP32 Pin 26
```

**Important Notes:**
- Encoders typically need 5V supply
- If using 3.3V encoders, connect VCC to 3.3V
- GPIO 32, 33, 25, 26 do NOT have internal pull-ups
- If encoders show zero count, add 10kΩ pull-up resistors:
  ```
  ESP32 Pin 32 ──[10kΩ]── 3.3V
  ESP32 Pin 33 ──[10kΩ]── 3.3V
  ESP32 Pin 25 ──[10kΩ]── 3.3V
  ESP32 Pin 26 ──[10kΩ]── 3.3V
  ```

---

## 6. Power Distribution System

### Power Flow Diagram
```
Battery Pack (7.4V LiPo)
    │
    ├─────→ Buck-Boost Converter → 5V Rail
    │                                  │
    │                                  ├→ ESP32 VIN (5V)
    │                                  ├→ Encoder 1 VCC
    │                                  └→ Encoder 2 VCC
    │
    └─────→ DRV8833 VM (Motor Power)

ESP32 3.3V Output
    ├→ MPU6050 VCC
    ├→ Left LIDAR VCC
    ├→ Front LIDAR VCC
    ├→ Right LIDAR VCC
    └→ DRV8833 VCC (Logic)
    └→ DRV8833 EEP (Enable)
```

### Ground Connections (CRITICAL)
```
All grounds MUST be connected together:
- Battery negative
- Buck-boost ground
- ESP32 GND
- DRV8833 GND
- MPU6050 GND
- All 3 LIDAR GND
- Encoder 1 GND
- Encoder 2 GND
```

**Common Ground is Essential:** Without common ground, communication and motor control will fail.

---

## 7. Recommended Component Specifications

### Battery Pack
- **Type:** 2S LiPo (7.4V nominal) or 6× AA batteries (7.2-9V)
- **Capacity:** ≥1000mAh (higher for longer run time)
- **Discharge rate:** ≥2C
- **Connector:** XT60 or similar

### Buck-Boost Converter
- **Input:** 5-12V
- **Output:** 5V regulated
- **Current:** ≥2A
- **Features:** Short circuit protection, thermal shutdown

### DRV8833 Specifications
- **Motor voltage (VM):** 2.7-10.8V
- **Logic voltage (VCC):** 2.7-6.8V (use 3.3V)
- **Output current:** 1.5A per channel (peak 2A)
- **PWM frequency:** Up to 250kHz

### Motors
- **Voltage:** 6-9V (match to battery)
- **Current:** <1A stall
- **Type:** DC gear motor with encoders
- **Gear ratio:** 20:1 to 100:1 (higher = more torque, slower)

### Encoders
- **Type:** Quadrature (2-channel)
- **Resolution:** ≥100 PPR (pulses per revolution)
- **Voltage:** 3.3V or 5V compatible

---

## 8. Assembly Tips

### Soldering Guidelines
1. **Use solid core wire** for permanent connections
2. **Strain relief:** Hot glue wires near solder joints
3. **Label wires:** Use colored wire or heat shrink markers
4. **Test continuity:** Multimeter check before power-up

### Wire Management
1. **Keep power wires separate** from signal wires
2. **Twist I2C pairs:** SDA/SCL twisted reduces noise
3. **Short wires:** Minimize wire length to reduce resistance
4. **Cable routing:** Avoid wires near moving parts (wheels)

### Mounting Considerations
1. **MPU6050:** Mount flat, secure (no vibration)
2. **LIDARs:** Perpendicular to walls, at wall height (~2.5cm)
3. **Motors:** Parallel alignment critical
4. **Battery:** Low center of gravity, secure mount

---

## 9. Testing Procedure (Step-by-Step)

### Step 1: Power Test (NO CODE UPLOADED YET)
```
1. Connect only battery and buck-boost converter
2. Verify 5V output with multimeter
3. Check for heating or unusual smells
4. Disconnect immediately if any issues
```

### Step 2: ESP32 Test
```
1. Connect 5V to ESP32 VIN
2. Connect GND
3. Upload simple blink sketch
4. Verify LED blinks
```

### Step 3: Motor Test
```
1. Upload motor test code
2. Connect one motor to DRV8833
3. Verify motor spins both directions
4. Repeat for second motor
5. Check for smooth operation, no grinding
```

### Step 4: Encoder Test
```
1. Upload encoder test code
2. Manually spin wheel
3. Verify count increases/decreases
4. Check both channels respond
```

### Step 5: I2C Device Test
```
1. Upload I2C scanner code:
   #include <Wire.h>
   void setup() {
     Wire.begin(21, 22);
     Serial.begin(115200);
   }
   void loop() {
     for (byte addr = 1; addr < 127; addr++) {
       Wire.beginTransmission(addr);
       if (Wire.endTransmission() == 0) {
         Serial.print("Found: 0x");
         Serial.println(addr, HEX);
       }
     }
     delay(5000);
   }

2. Should detect:
   - 0x68 (MPU6050)
   - 0x29 (all LIDARs initially)
```

### Step 6: Individual Sensor Tests
```
1. Test MPU6050 (gyro code provided)
2. Test each LIDAR (lidar code provided)
3. Verify readings are reasonable
```

### Step 7: Full System Test
```
1. Upload complete Micromouse code
2. Place in test area
3. Send 'g' to start
4. Monitor serial output
5. Verify all systems working together
```

---

## 10. Wiring Diagram (ASCII Art)

```
                    ┌─────────────┐
                    │   BATTERY   │
                    │   (7.4V)    │
                    └─────┬───────┘
                          │
                ┌─────────┴─────────┐
                │                   │
                │                   │
        ┌───────▼────────┐   ┌──────▼──────┐
        │  Buck-Boost    │   │   DRV8833   │
        │  Converter     │   │   (VM)      │
        │  (5V output)   │   └──────┬──────┘
        └───────┬────────┘          │
                │                   │ (Motor Power)
                │                   │
        ┌───────▼────────┐          │
        │     ESP32      │          │
        │                │◄─────────┘ (Control: IN1-4)
        │  Pin 21 (SDA)──┼──┬──┬──┬─ (I2C Bus)
        │  Pin 22 (SCL)──┼──┼──┼──┼─ (I2C Bus)
        │                │  │  │  │
        │  Pin 18 ───────┼──┼──┼──┼─→ DRV8833 IN1
        │  Pin 19 ───────┼──┼──┼──┼─→ DRV8833 IN2
        │  Pin 23 ───────┼──┼──┼──┼─→ DRV8833 IN3
        │  Pin 5  ───────┼──┼──┼──┼─→ DRV8833 IN4
        │                │  │  │  │
        │  Pin 32 ◄──────┼──┼──┼──┼─ Encoder 1A
        │  Pin 33 ◄──────┼──┼──┼──┼─ Encoder 1B
        │  Pin 25 ◄──────┼──┼──┼──┼─ Encoder 2A
        │  Pin 26 ◄──────┼──┼──┼──┼─ Encoder 2B
        │                │  │  │  │
        │  Pin 27 ───────┼──┼──┼──┼─→ Left LIDAR XSHUT
        │  Pin 14 ───────┼──┼──┼──┼─→ Front LIDAR XSHUT
        │  Pin 12 ───────┼──┼──┼──┼─→ Right LIDAR XSHUT
        │                │  │  │  │
        │  3.3V ─────────┼──┼──┼──┼─→ Sensors VCC
        │  GND ──────────┼──┼──┼──┼─→ Common Ground
        └────────────────┘  │  │  │
                            │  │  │
                ┌───────────▼┐ │  │
                │  MPU6050    │ │  │
                └─────────────┘ │  │
                                │  │
                ┌───────────────▼┐ │
                │  Left LIDAR     │ │
                └─────────────────┘ │
                                    │
                ┌───────────────────▼┐
                │  Front LIDAR        │
                └─────────────────────┘
                
                ┌─────────────────────┐
                │  Right LIDAR        │
                └─────────────────────┘

        ┌───────────────┐     ┌───────────────┐
        │  Motor 1      │     │  Motor 2      │
        │  (Left)       │     │  (Right)      │
        │               │     │               │
        │  + ◄─ OUT1    │     │  + ◄─ OUT3    │
        │  - ◄─ OUT2    │     │  - ◄─ OUT4    │
        │               │     │               │
        │ ┌─────────┐   │     │ ┌─────────┐   │
        │ │Encoder 1│───┼─────┼─│Encoder 2│───┼─→ ESP32
        │ └─────────┘   │     │ └─────────┘   │
        └───────────────┘     └───────────────┘
```

---

## 11. Common Wiring Mistakes

### Mistake 1: Wrong I2C Pins
- **Symptom:** Sensors not detected
- **Fix:** ESP32 I2C is on pins 21 (SDA) and 22 (SCL), not default Arduino pins

### Mistake 2: Missing Common Ground
- **Symptom:** Erratic behavior, communication failures
- **Fix:** Connect all GND pins together

### Mistake 3: Motor Voltage to Logic Pins
- **Symptom:** ESP32 damaged
- **Fix:** DRV8833 VM connects to battery, VCC connects to 3.3V

### Mistake 4: Reversed Motor Connections
- **Symptom:** Robot moves backward when commanded forward
- **Fix:** Swap OUT1/OUT2 or OUT3/OUT4 connections

### Mistake 5: No Pull-up Resistors on Encoders
- **Symptom:** Encoder count stays at zero
- **Fix:** Add 10kΩ resistors from encoder pins to 3.3V

### Mistake 6: LIDAR XSHUT Not Connected
- **Symptom:** All LIDARs have same address, can't initialize
- **Fix:** Wire XSHUT pins to ESP32 pins 27, 14, 12

### Mistake 7: Insufficient Power Supply
- **Symptom:** ESP32 resets during motor operation
- **Fix:** Use adequate battery and buck-boost converter (≥2A)

---

## 12. Quick Reference Tables

### Color Code Recommendation
```
Red       → Power (+)
Black     → Ground (-)
Yellow    → Signal (SDA, Encoders, etc.)
Green     → Signal (SCL, Encoders, etc.)
Blue      → Control (IN1-4)
White     → Optional/Interrupt pins
```

### Pin Function Summary
| Pin | Function | Type | Device |
|-----|----------|------|--------|
| 21 | SDA | I2C | MPU6050 + 3× LIDAR |
| 22 | SCL | I2C | MPU6050 + 3× LIDAR |
| 18 | IN1 | PWM Out | Motor 1 Forward |
| 19 | IN2 | PWM Out | Motor 1 Reverse |
| 23 | IN3 | PWM Out | Motor 2 Forward |
| 5 | IN4 | PWM Out | Motor 2 Reverse |
| 32 | EncA | Input | Motor 1 Encoder A |
| 33 | EncB | Input | Motor 1 Encoder B |
| 25 | EncA | Input | Motor 2 Encoder A |
| 26 | EncB | Input | Motor 2 Encoder B |
| 27 | XSHUT | Output | Left LIDAR |
| 14 | XSHUT | Output | Front LIDAR |
| 12 | XSHUT | Output | Right LIDAR |
| 4 | INT | Input | MPU6050 (optional) |

---

## Need Help?

If you encounter wiring issues:
1. **Double-check all connections** against this diagram
2. **Use multimeter** to verify continuity and voltage
3. **Test components individually** before full assembly
4. **Check for shorts** between adjacent pins
5. **Verify power supply** is adequate for all components

Good luck with your build!
