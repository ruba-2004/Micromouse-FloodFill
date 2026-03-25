# Micromouse Calibration & Tuning Guide

## Overview
This guide helps you calibrate your Micromouse robot to ensure accurate movement and maze solving.

---

## 1. ENCODER CALIBRATION

### Purpose
Determine the exact number of encoder ticks needed to move one cell (168mm).

### Procedure

1. **Setup:**
   - Place robot on smooth, level surface
   - Measure and mark exactly 168mm (one cell distance)
   - Upload test code below

2. **Test Code:**
```cpp
void setup() {
  Serial.begin(115200);
  // Initialize motors and encoders (same as main code)
  pinMode(MOTOR1_IN1, OUTPUT);
  pinMode(MOTOR1_IN2, OUTPUT);
  pinMode(MOTOR2_IN3, OUTPUT);
  pinMode(MOTOR2_IN4, OUTPUT);
  
  pinMode(ENC1_A, INPUT);
  pinMode(ENC1_B, INPUT);
  pinMode(ENC2_A, INPUT);
  pinMode(ENC2_B, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(ENC1_A), handleEncoder1, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC2_A), handleEncoder2, RISING);
}

void loop() {
  Serial.println("Press 's' to start test...");
  while (!Serial.available());
  Serial.read();
  
  encoder1Count = 0;
  encoder2Count = 0;
  
  // Move forward at constant speed
  setMotorSpeed(1, 180);
  setMotorSpeed(2, 180);
  
  delay(2000); // Adjust time to travel ~168mm
  
  stopMotors();
  
  Serial.print("Encoder 1: ");
  Serial.println(encoder1Count);
  Serial.print("Encoder 2: ");
  Serial.println(encoder2Count);
  Serial.print("Average: ");
  Serial.println((encoder1Count + encoder2Count) / 2);
  
  delay(3000);
}
```

3. **Calculation:**
   - Run test multiple times
   - Measure actual distance traveled
   - Calculate: `ENCODER_TICKS_PER_CELL = (measured_ticks * 168) / actual_distance_mm`
   - Update constant in main code

4. **Verification:**
   - Test with updated value
   - Robot should travel exactly 168mm
   - Repeat if needed

### Expected Results
- Typical values: 800-1500 ticks per cell (depends on encoder resolution and wheel diameter)
- Both encoders should be within 5% of each other
- If difference > 10%, check wiring or mechanical alignment

---

## 2. GYROSCOPE CALIBRATION

### Purpose
Eliminate gyroscope drift for accurate heading tracking.

### Automatic Calibration
The main code includes automatic calibration in `setup()`:
```cpp
void initMPU6050() {
  // Calibrates automatically on startup
  Serial.println("Calibrating gyroscope - KEEP ROBOT STILL!");
  // Takes 2000 samples over 4 seconds
}
```

### Critical Requirements
1. **Robot must be completely still** during calibration
2. **Place on stable surface** (not hand-held)
3. **No vibrations** from nearby equipment
4. **Temperature stable** (don't calibrate right after moving from cold to warm)

### Verification Test
```cpp
void testGyro() {
  // After calibration, let robot sit still for 60 seconds
  unsigned long startTime = millis();
  currentHeading = 0;
  
  while (millis() - startTime < 60000) {
    updateGyro();
    delay(100);
  }
  
  Serial.print("Drift after 60 seconds: ");
  Serial.print(currentHeading);
  Serial.println(" degrees");
}
```

### Acceptable Drift
- **Good:** < 2° per minute
- **Acceptable:** < 5° per minute
- **Needs recalibration:** > 5° per minute

### If drift is excessive:
1. Increase deadzone value (currently 0.5):
   ```cpp
   if (abs(gyroZ_vel) > 1.0) { // Increase from 0.5 to 1.0
   ```
2. Ensure MPU6050 is securely mounted (no vibration)
3. Check I2C connections
4. Try recalibrating in final operating environment

---

## 3. TURN ANGLE CALIBRATION

### Purpose
Ensure robot turns exactly 90°, 180°, 270°.

### Test Code
```cpp
void testTurns() {
  Serial.println("Testing 90° right turn...");
  currentHeading = 0;
  delay(1000);
  
  turn(90);
  
  Serial.print("Expected: 90°, Actual: ");
  Serial.println(currentHeading);
  
  delay(3000);
  
  Serial.println("Testing 90° left turn...");
  turn(-90);
  
  Serial.print("Expected: 0°, Actual: ");
  Serial.println(currentHeading);
}
```

### Procedure
1. Place robot on smooth surface
2. Mark starting position with tape
3. Run turn test
4. Measure actual angle with protractor or grid

### Adjustments
If turns are inaccurate:

**Under-turning (e.g., 85° instead of 90°):**
- Reduce TURN_SPEED slightly (current: 150)
- Increase minimum speed threshold in turn function

**Over-turning (e.g., 95° instead of 90°):**
- Increase TURN_SPEED
- Add more aggressive stopping (longer brake time)

**Oscillation around target:**
- Robot overshoots, corrects, overshoots again
- Reduce threshold in turn function:
  ```cpp
  if (abs(error) < 1.0) { // Reduce from 2.0 to 1.0
  ```

### Advanced Turn Tuning
```cpp
void turn(int degrees) {
  float targetHeading = currentHeading + degrees;
  // Normalize...
  
  int turnDir = (degrees > 0) ? 1 : -1;
  
  while (true) {
    updateGyro();
    float error = targetHeading - currentHeading;
    // Normalize error...
    
    if (abs(error) < 1.5) { // Fine-tune this threshold
      break;
    }
    
    // Proportional speed control
    int speed = TURN_SPEED;
    if (abs(error) < 30) {
      // Slow down approach
      speed = map(abs(error), 0, 30, 80, TURN_SPEED); // Fine-tune these values
    }
    
    setMotorSpeed(1, turnDir * speed);
    setMotorSpeed(2, -turnDir * speed);
    
    delay(10);
  }
  
  stopMotors();
  delay(200); // Settling time - adjust if needed
  
  snapHeading();
}
```

---

## 4. LIDAR WALL DETECTION CALIBRATION

### Purpose
Set correct threshold distance for wall detection.

### Test Procedure
1. **Measure cell dimensions:**
   - Interior cell: 168mm x 168mm
   - Wall thickness: 12mm
   - Distance from center to wall: 84mm

2. **Test Code:**
```cpp
void testLIDARs() {
  while (true) {
    int left = readLeftDistance();
    int front = readFrontDistance();
    int right = readRightDistance();
    
    Serial.print("L: ");
    Serial.print(left);
    Serial.print("mm  F: ");
    Serial.print(front);
    Serial.print("mm  R: ");
    Serial.print(right);
    Serial.println("mm");
    
    delay(500);
  }
}
```

3. **Collect Data:**
   - **With wall present:** Place robot centered in cell with walls
     - Expected: ~70-90mm (center to wall minus sensor offset)
   - **Without wall:** Place in open area
     - Expected: >200mm or out of range

4. **Set Threshold:**
   ```cpp
   #define WALL_THRESHOLD 100  // Adjust based on measurements
   ```
   - Should be between "wall present" and "wall absent" readings
   - Typical range: 90-120mm

### Troubleshooting LIDAR Issues

**All sensors read 0 or 8000:**
- Check: I2C connections, XSHUT wiring
- Fix: Verify address assignment in `initLIDARs()`

**Readings fluctuate wildly:**
- Check: Sensor mounting (must be stable)
- Fix: Ensure sensors don't vibrate, add damping

**One sensor doesn't work:**
- Check: XSHUT pin correct? Address unique?
- Fix: Verify pin assignments match hardware

**Readings affected by wall color:**
- Black walls absorb IR light → shorter readings
- White walls reflect well → accurate readings
- Adjust WALL_THRESHOLD per competition maze

---

## 5. PID TUNING FOR STRAIGHT MOVEMENT

### Purpose
Robot drives straight without veering left or right.

### Current PID Constants
```cpp
#define KP_STRAIGHT 2.0
#define KD_STRAIGHT 0.5
```

### Tuning Process

1. **Start with P-only control:**
   ```cpp
   #define KP_STRAIGHT 1.0
   #define KD_STRAIGHT 0.0
   ```

2. **Test straight movement:**
   - Robot should try to correct heading errors
   - If oscillates (S-curve), reduce KP
   - If doesn't correct, increase KP

3. **Add D-term to reduce oscillation:**
   ```cpp
   #define KD_STRAIGHT 0.3
   ```
   - Increase if still oscillating
   - Reduce if robot becomes sluggish

4. **Fine-tune for smooth movement:**
   - Good values typically: KP=1.5-3.0, KD=0.3-1.0
   - Higher speeds may need different constants

### Test Code
```cpp
void testStraight() {
  encoder1Count = 0;
  encoder2Count = 0;
  currentHeading = 0;
  
  while ((encoder1Count + encoder2Count) / 2 < ENCODER_TICKS_PER_CELL * 5) {
    updateGyro();
    
    // PID code here...
    
    Serial.print("Heading: ");
    Serial.print(currentHeading);
    Serial.print("  Enc1: ");
    Serial.print(encoder1Count);
    Serial.print("  Enc2: ");
    Serial.println(encoder2Count);
    
    delay(10);
  }
  
  stopMotors();
  
  Serial.print("Final heading: ");
  Serial.println(currentHeading);
  Serial.print("Encoder difference: ");
  Serial.println(abs(encoder1Count - encoder2Count));
}
```

### Success Criteria
- **Heading error:** < 2° after 5 cells
- **Encoder difference:** < 10% between motors
- **Path:** Robot travels in straight line (mark with tape)

---

## 6. SPEED OPTIMIZATION

### Phase 1: Low Speed Testing (BASE_SPEED = 100-150)
- **Purpose:** Verify accuracy at safe speeds
- **Use for:** Initial testing, algorithm verification

### Phase 2: Medium Speed (BASE_SPEED = 150-200)
- **Purpose:** Balance speed and accuracy
- **Use for:** Competition exploration runs

### Phase 3: High Speed (BASE_SPEED = 200-255)
- **Purpose:** Maximum speed for speed runs
- **Requirements:**
  - Excellent PID tuning
  - Precise mechanical alignment
  - Low-friction wheels
  - Strong battery

### Speed Tuning Tips

1. **Acceleration/Deceleration:**
   Add ramping to prevent wheel slip:
   ```cpp
   void moveForwardOneCell() {
     // Ramp up speed
     for (int speed = 0; speed < BASE_SPEED; speed += 10) {
       setMotorSpeed(1, speed);
       setMotorSpeed(2, speed);
       delay(20);
     }
     
     // Main movement with PID...
     
     // Ramp down before stopping
     for (int speed = BASE_SPEED; speed > 0; speed -= 20) {
       setMotorSpeed(1, speed);
       setMotorSpeed(2, speed);
       delay(10);
     }
   }
   ```

2. **Corner Cutting:**
   Advanced: Don't fully stop at each cell
   ```cpp
   // Instead of: turn → stop → move
   // Do: slow down → turn → accelerate
   ```

3. **Battery Monitoring:**
   Add voltage check:
   ```cpp
   float batteryVoltage = analogRead(BATTERY_PIN) * (3.3 / 4095.0) * VOLTAGE_DIVIDER;
   if (batteryVoltage < 6.5) {
     Serial.println("WARNING: Low battery!");
   }
   ```

---

## 7. FINAL SYSTEM CHECK

### Pre-Competition Checklist

**Hardware:**
- [ ] All screws tight, nothing loose
- [ ] Wheels spin freely, no rubbing
- [ ] Battery fully charged
- [ ] All sensors securely mounted
- [ ] Wiring secure, no shorts
- [ ] Robot fits in 20cm x 20cm x 20cm

**Software:**
- [ ] ENCODER_TICKS_PER_CELL calibrated
- [ ] WALL_THRESHOLD set correctly
- [ ] PID constants tuned
- [ ] Turn angles accurate (±2°)
- [ ] Straight movement verified

**Functional Tests:**
- [ ] Robot moves exactly 168mm per cell
- [ ] Turns exactly 90° each direction
- [ ] Detects walls correctly
- [ ] Solves simple 3x3 maze
- [ ] Returns to start after goal
- [ ] Executes speed run

### Test Maze Suggestions

**Test 1: Simple Path**
```
Start -> 3 cells forward -> turn right -> 2 cells -> Goal
```

**Test 2: Multiple Turns**
```
Start -> left -> forward -> right -> forward -> left -> Goal
```

**Test 3: Dead End**
```
Start -> forward -> dead end -> turn around -> Goal
```

### Competition Day Setup

1. **30 minutes before:** 
   - Charge battery to 100%
   - Power on, verify all sensors
   - Calibrate gyroscope in competition area

2. **During inspection:**
   - Test on actual maze floor (friction check)
   - Verify lighting doesn't affect LIDARs
   - Quick turn test to verify calibration

3. **Before run:**
   - Reset robot to start position
   - Ensure serial monitor disconnected (if wireless)
   - Send start command ('g')

---

## 8. TROUBLESHOOTING COMMON ISSUES

### Robot Veers to One Side
**Causes:**
- Motors different speeds
- Wheel diameter different
- Encoder miscalibrated
- Poor mechanical alignment

**Solutions:**
1. Check encoder counts are equal
2. Adjust motor speed offsets:
   ```cpp
   #define MOTOR1_OFFSET 1.0  // Multiply motor 1 speed
   #define MOTOR2_OFFSET 0.98 // Slightly slower motor 2
   ```
3. Replace wheels if diameter differs

### Robot Overshoots Cells
**Causes:**
- ENCODER_TICKS_PER_CELL too low
- Momentum from high speed
- Floor too slippery

**Solutions:**
1. Recalibrate encoder constant
2. Add deceleration ramp
3. Reduce BASE_SPEED

### Robot Gets Lost in Maze
**Causes:**
- Wall detection inaccurate
- Position tracking off
- Flood fill error

**Solutions:**
1. Verify WALL_THRESHOLD
2. Add position verification using walls
3. Print flood values to debug

### LIDARs Give Inconsistent Readings
**Causes:**
- Vibration during movement
- I2C noise
- Insufficient power

**Solutions:**
1. Mount sensors rigidly
2. Add capacitors to power lines
3. Use shielded I2C wires

---

## 9. PERFORMANCE BENCHMARKS

### Target Times (16x16 maze)

**First Run (Exploration):**
- Beginner: < 5 minutes
- Intermediate: < 3 minutes  
- Advanced: < 90 seconds

**Speed Run (Known Map):**
- Beginner: < 60 seconds
- Intermediate: < 30 seconds
- Advanced: < 15 seconds
- Expert: < 10 seconds

### Optimization Goals

1. **Reduce exploration time:**
   - Faster base speed
   - Minimize stops
   - Efficient flood fill updates

2. **Reduce speed run time:**
   - Maximum safe speed
   - Corner cutting
   - Smooth acceleration curves

3. **Improve reliability:**
   - Robust wall detection
   - Error recovery
   - Position verification

---

## Summary

Proper calibration is crucial for Micromouse success. Take time to:
1. Accurately calibrate encoders
2. Minimize gyroscope drift
3. Tune PID for straight, stable movement
4. Set correct wall detection threshold
5. Test thoroughly in maze-like environments

Remember: **Accuracy first, speed second!** Get the robot working reliably at low speed before attempting speed runs.

Good luck!
