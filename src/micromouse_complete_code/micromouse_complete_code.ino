/**
 * MICROMOUSE COMPETITION - COMPLETE CODE
 * Algorithm: Flood Fill
 * Hardware: ESP32, MPU6050, 3x VL53L0X, DRV8833, 2x DC Motors with Encoders
 * 
 * Pin Configuration:
 * - I2C: SDA=21, SCL=22 (MPU6050 + 3 LIDARs)
 * - Motor1: IN1=18, IN2=19, EncA=32, EncB=33
 * - Motor2: IN3=23, IN4=5, EncA=25, EncB=26
 * - LIDAR XSHUT: Left=27, Front=14, Right=12
 * - MPU6050: INT=4, AD0=GND (Address 0x68)
 */

#include <Wire.h>
#include "Adafruit_VL53L0X.h"

// ============================================================================
// CONFIGURATION & CONSTANTS
// ============================================================================

// Maze dimensions
#define MAZE_SIZE 16
#define CELL_SIZE 168        // Cell interior size in mm (16.8cm)
#define WALL_THRESHOLD 100   // Distance in mm to detect wall presence

// Motor pins
#define MOTOR1_IN1 18
#define MOTOR1_IN2 19
#define MOTOR2_IN3 23
#define MOTOR2_IN4 5

// Encoder pins
#define ENC1_A 32
#define ENC1_B 33
#define ENC2_A 25
#define ENC2_B 26

// LIDAR XSHUT pins (for address configuration)
#define LIDAR_LEFT_XSHUT 27
#define LIDAR_FRONT_XSHUT 14
#define LIDAR_RIGHT_XSHUT 12

// MPU6050
#define MPU_ADDR 0x68
#define MPU_INT_PIN 4

// LIDAR I2C addresses (default 0x29, we'll reassign)
#define LIDAR_LEFT_ADDR 0x30
#define LIDAR_FRONT_ADDR 0x31
#define LIDAR_RIGHT_ADDR 0x32

// Movement parameters
#define ENCODER_TICKS_PER_CELL 1000  // Adjust based on your encoder resolution
#define BASE_SPEED 180               // PWM value 0-255
#define TURN_SPEED 150               // Slower for precise turns

// PID constants for straight movement
#define KP_STRAIGHT 2.0
#define KD_STRAIGHT 0.5

// Directions
#define NORTH 0
#define EAST  1
#define SOUTH 2
#define WEST  3

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Sensors
Adafruit_VL53L0X lidarLeft = Adafruit_VL53L0X();
Adafruit_VL53L0X lidarFront = Adafruit_VL53L0X();
Adafruit_VL53L0X lidarRight = Adafruit_VL53L0X();

// MPU6050 variables
float gyroZ_offset = 0;
float currentHeading = 0;
unsigned long lastGyroTime;

// Encoder variables
volatile long encoder1Count = 0;
volatile long encoder2Count = 0;

// Robot state
int robotX = 0;        // Current X position in maze
int robotY = 0;        // Current Y position in maze
int robotDir = NORTH;  // Current direction facing

// Maze map: walls[y][x] stores walls as bit flags
// Bit 0 = North wall, Bit 1 = East wall, Bit 2 = South wall, Bit 3 = West wall
uint8_t walls[MAZE_SIZE][MAZE_SIZE];

// Flood fill distances
uint8_t floodValues[MAZE_SIZE][MAZE_SIZE];

// Goal coordinates (center 4 cells)
int goalX[] = {7, 8, 7, 8};
int goalY[] = {7, 7, 8, 8};

bool mazeExplored = false;
bool atGoal = false;

// ============================================================================
// INTERRUPT SERVICE ROUTINES
// ============================================================================

void IRAM_ATTR handleEncoder1() {
  if (digitalRead(ENC1_B) == HIGH) {
    encoder1Count++;
  } else {
    encoder1Count--;
  }
}

void IRAM_ATTR handleEncoder2() {
  if (digitalRead(ENC2_B) == HIGH) {
    encoder2Count++;
  } else {
    encoder2Count--;
  }
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== MICROMOUSE INITIALIZING ===");
  
  // Initialize I2C
  Wire.begin(21, 22);
  Wire.setClock(400000); // 400kHz I2C
  
  // Initialize motor pins
  pinMode(MOTOR1_IN1, OUTPUT);
  pinMode(MOTOR1_IN2, OUTPUT);
  pinMode(MOTOR2_IN3, OUTPUT);
  pinMode(MOTOR2_IN4, OUTPUT);
  stopMotors();
  
  // Initialize encoder pins
  pinMode(ENC1_A, INPUT);
  pinMode(ENC1_B, INPUT);
  pinMode(ENC2_A, INPUT);
  pinMode(ENC2_B, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENC1_A), handleEncoder1, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC2_A), handleEncoder2, RISING);
  
  // Initialize LIDAR XSHUT pins
  pinMode(LIDAR_LEFT_XSHUT, OUTPUT);
  pinMode(LIDAR_FRONT_XSHUT, OUTPUT);
  pinMode(LIDAR_RIGHT_XSHUT, OUTPUT);
  
  // Initialize sensors
  initMPU6050();
  initLIDARs();
  
  // Initialize maze
  initMaze();
  
  Serial.println("=== INITIALIZATION COMPLETE ===");
  Serial.println("Place robot at START position (corner)");
  Serial.println("Send 'g' to begin maze solving...");
  
  // Wait for start command
  while (true) {
    if (Serial.available() && Serial.read() == 'g') {
      break;
    }
    delay(100);
  }
  
  Serial.println("\n=== STARTING MAZE SOLVING ===");
  delay(2000); // Give time to remove hand
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  if (!atGoal) {
    // EXPLORATION MODE: Find the goal
    exploreStep();
  } else if (!mazeExplored) {
    // First time reaching goal - mark as explored
    mazeExplored = true;
    Serial.println("\n*** GOAL REACHED! ***");
    Serial.println("Returning to start for speed run...");
    delay(3000);
    
    // Navigate back to start
    navigateToStart();
    
    Serial.println("\n*** BACK AT START ***");
    Serial.println("Ready for speed run!");
    delay(2000);
  } else {
    // SPEED RUN MODE: Execute optimal path
    speedRun();
    
    // After speed run, stop
    Serial.println("\n*** SPEED RUN COMPLETE ***");
    while (true) {
      delay(1000);
    }
  }
}

// ============================================================================
// MAZE INITIALIZATION
// ============================================================================

void initMaze() {
  Serial.println("Initializing maze...");
  
  // Initialize walls: Assume all walls exist initially
  for (int y = 0; y < MAZE_SIZE; y++) {
    for (int x = 0; x < MAZE_SIZE; x++) {
      walls[y][x] = 0;
      
      // Add outer boundary walls
      if (y == 0) walls[y][x] |= (1 << SOUTH);
      if (y == MAZE_SIZE - 1) walls[y][x] |= (1 << NORTH);
      if (x == 0) walls[y][x] |= (1 << WEST);
      if (x == MAZE_SIZE - 1) walls[y][x] |= (1 << EAST);
      
      floodValues[y][x] = 255; // Max distance
    }
  }
  
  // Set start position (0,0) with known walls
  // Start square is bounded on three sides
  walls[0][0] |= (1 << SOUTH);  // South wall
  walls[0][0] |= (1 << WEST);   // West wall
  // North is open (entrance)
  
  // Set goal cells to distance 0
  for (int i = 0; i < 4; i++) {
    floodValues[goalY[i]][goalX[i]] = 0;
  }
  
  // Calculate initial flood fill
  floodFill();
  
  Serial.println("Maze initialized!");
}

// ============================================================================
// SENSOR INITIALIZATION
// ============================================================================

void initMPU6050() {
  Serial.println("Initializing MPU6050...");
  
  // Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  
  // Calibrate gyroscope
  Serial.println("Calibrating gyroscope - KEEP ROBOT STILL!");
  long sum = 0;
  int samples = 2000;
  for (int i = 0; i < samples; i++) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x47); // GYRO_ZOUT_H
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 2, true);
    sum += (int16_t)(Wire.read() << 8 | Wire.read());
    delay(2);
  }
  gyroZ_offset = (float)sum / (float)samples;
  
  currentHeading = 0;
  lastGyroTime = micros();
  
  Serial.println("MPU6050 ready!");
}

void initLIDARs() {
  Serial.println("Initializing LIDARs...");
  
  // Reset all sensors
  digitalWrite(LIDAR_LEFT_XSHUT, LOW);
  digitalWrite(LIDAR_FRONT_XSHUT, LOW);
  digitalWrite(LIDAR_RIGHT_XSHUT, LOW);
  delay(10);
  
  // Initialize LEFT LIDAR
  digitalWrite(LIDAR_LEFT_XSHUT, HIGH);
  delay(10);
  if (!lidarLeft.begin(LIDAR_LEFT_ADDR)) {
    Serial.println("ERROR: Left LIDAR failed!");
    while (1);
  }
  Serial.println("Left LIDAR OK");
  
  // Initialize FRONT LIDAR
  digitalWrite(LIDAR_FRONT_XSHUT, HIGH);
  delay(10);
  if (!lidarFront.begin(LIDAR_FRONT_ADDR)) {
    Serial.println("ERROR: Front LIDAR failed!");
    while (1);
  }
  Serial.println("Front LIDAR OK");
  
  // Initialize RIGHT LIDAR
  digitalWrite(LIDAR_RIGHT_XSHUT, HIGH);
  delay(10);
  if (!lidarRight.begin(LIDAR_RIGHT_ADDR)) {
    Serial.println("ERROR: Right LIDAR failed!");
    while (1);
  }
  Serial.println("Right LIDAR OK");
  
  Serial.println("All LIDARs initialized!");
}

// ============================================================================
// SENSOR READING
// ============================================================================

void updateGyro() {
  unsigned long currentTime = micros();
  float dt = (currentTime - lastGyroTime) / 1000000.0;
  lastGyroTime = currentTime;
  
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x47);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);
  int16_t gyroZ_raw = Wire.read() << 8 | Wire.read();
  
  float gyroZ_vel = (gyroZ_raw - gyroZ_offset) / 131.0;
  
  // Deadzone filter
  if (abs(gyroZ_vel) > 0.5) {
    currentHeading += gyroZ_vel * dt;
  }
  
  // Normalize to -180 to +180
  while (currentHeading > 180) currentHeading -= 360;
  while (currentHeading < -180) currentHeading += 360;
}

int readLeftDistance() {
  VL53L0X_RangingMeasurementData_t measure;
  lidarLeft.rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    return measure.RangeMilliMeter;
  }
  return 8000; // Out of range
}

int readFrontDistance() {
  VL53L0X_RangingMeasurementData_t measure;
  lidarFront.rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    return measure.RangeMilliMeter;
  }
  return 8000;
}

int readRightDistance() {
  VL53L0X_RangingMeasurementData_t measure;
  lidarRight.rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    return measure.RangeMilliMeter;
  }
  return 8000;
}

// ============================================================================
// MOTOR CONTROL
// ============================================================================

void setMotorSpeed(int motor, int speed) {
  // motor: 1 or 2
  // speed: -255 to +255 (negative = reverse)
  
  int in1, in2;
  if (motor == 1) {
    in1 = MOTOR1_IN1;
    in2 = MOTOR1_IN2;
  } else {
    in1 = MOTOR2_IN3;
    in2 = MOTOR2_IN4;
  }
  
  if (speed > 0) {
    analogWrite(in1, speed);
    digitalWrite(in2, LOW);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);
    analogWrite(in2, -speed);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

void stopMotors() {
  digitalWrite(MOTOR1_IN1, LOW);
  digitalWrite(MOTOR1_IN2, LOW);
  digitalWrite(MOTOR2_IN3, LOW);
  digitalWrite(MOTOR2_IN4, LOW);
}

// ============================================================================
// MOVEMENT FUNCTIONS
// ============================================================================

void moveForwardOneCell() {
  Serial.println("Moving forward one cell...");
  
  encoder1Count = 0;
  encoder2Count = 0;
  
  float targetHeading = getTargetHeading();
  float lastError = 0;
  
  while (true) {
    updateGyro();
    
    long avgCount = (encoder1Count + encoder2Count) / 2;
    
    if (avgCount >= ENCODER_TICKS_PER_CELL) {
      break;
    }
    
    // PID for straight movement
    float headingError = targetHeading - currentHeading;
    
    // Normalize error
    while (headingError > 180) headingError -= 360;
    while (headingError < -180) headingError += 360;
    
    float derivative = headingError - lastError;
    lastError = headingError;
    
    float correction = KP_STRAIGHT * headingError + KD_STRAIGHT * derivative;
    
    int leftSpeed = BASE_SPEED + correction;
    int rightSpeed = BASE_SPEED - correction;
    
    // Limit speeds
    leftSpeed = constrain(leftSpeed, 0, 255);
    rightSpeed = constrain(rightSpeed, 0, 255);
    
    setMotorSpeed(1, leftSpeed);
    setMotorSpeed(2, rightSpeed);
    
    delay(10);
  }
  
  stopMotors();
  delay(100);
  
  // Update position
  updatePosition();
  
  Serial.print("Position: (");
  Serial.print(robotX);
  Serial.print(", ");
  Serial.print(robotY);
  Serial.println(")");
}

void turnLeft() {
  Serial.println("Turning left...");
  turn(-90);
  robotDir = (robotDir + 3) % 4;
}

void turnRight() {
  Serial.println("Turning right...");
  turn(90);
  robotDir = (robotDir + 1) % 4;
}

void turnAround() {
  Serial.println("Turning around...");
  turn(180);
  robotDir = (robotDir + 2) % 4;
}

void turn(int degrees) {
  // degrees: positive = right, negative = left
  
  float targetHeading = currentHeading + degrees;
  
  // Normalize target
  while (targetHeading > 180) targetHeading -= 360;
  while (targetHeading < -180) targetHeading += 360;
  
  // Turn in place
  int turnDir = (degrees > 0) ? 1 : -1;
  
  while (true) {
    updateGyro();
    
    float error = targetHeading - currentHeading;
    while (error > 180) error -= 360;
    while (error < -180) error += 360;
    
    if (abs(error) < 2.0) {
      break;
    }
    
    int speed = TURN_SPEED;
    if (abs(error) < 20) {
      speed = 100; // Slow down near target
    }
    
    setMotorSpeed(1, turnDir * speed);
    setMotorSpeed(2, -turnDir * speed);
    
    delay(10);
  }
  
  stopMotors();
  delay(200);
  
  // Snap heading to cardinal direction
  snapHeading();
}

void snapHeading() {
  // Snap currentHeading to nearest 90 degree angle
  int cardinal = round(currentHeading / 90.0) * 90;
  currentHeading = cardinal;
  
  if (currentHeading == 180) currentHeading = -180; // Normalize
}

float getTargetHeading() {
  // Return expected heading based on direction
  switch (robotDir) {
    case NORTH: return 0;
    case EAST:  return 90;
    case SOUTH: return 180;
    case WEST:  return -90;
    default: return 0;
  }
}

void updatePosition() {
  // Update robot position based on direction
  switch (robotDir) {
    case NORTH: robotY++; break;
    case EAST:  robotX++; break;
    case SOUTH: robotY--; break;
    case WEST:  robotX--; break;
  }
}

// ============================================================================
// WALL DETECTION
// ============================================================================

void detectWalls() {
  int leftDist = readLeftDistance();
  int frontDist = readFrontDistance();
  int rightDist = readRightDistance();
  
  Serial.print("Distances - L:");
  Serial.print(leftDist);
  Serial.print(" F:");
  Serial.print(frontDist);
  Serial.print(" R:");
  Serial.println(rightDist);
  
  int x = robotX;
  int y = robotY;
  
  // Clear previously detected walls for this position
  // (Keep boundary walls)
  if (y > 0 && y < MAZE_SIZE - 1 && x > 0 && x < MAZE_SIZE - 1) {
    walls[y][x] = 0;
  }
  
  // Add boundary walls back
  if (y == 0) walls[y][x] |= (1 << SOUTH);
  if (y == MAZE_SIZE - 1) walls[y][x] |= (1 << NORTH);
  if (x == 0) walls[y][x] |= (1 << WEST);
  if (x == MAZE_SIZE - 1) walls[y][x] |= (1 << EAST);
  
  // Detect walls based on current direction
  bool leftWall = (leftDist < WALL_THRESHOLD);
  bool frontWall = (frontDist < WALL_THRESHOLD);
  bool rightWall = (rightDist < WALL_THRESHOLD);
  
  int leftDir = (robotDir + 3) % 4;  // Left of current direction
  int frontDir = robotDir;
  int rightDir = (robotDir + 1) % 4;
  
  if (frontWall) {
    walls[y][x] |= (1 << frontDir);
    updateNeighborWall(x, y, frontDir);
  }
  if (leftWall) {
    walls[y][x] |= (1 << leftDir);
    updateNeighborWall(x, y, leftDir);
  }
  if (rightWall) {
    walls[y][x] |= (1 << rightDir);
    updateNeighborWall(x, y, rightDir);
  }
}

void updateNeighborWall(int x, int y, int dir) {
  // Update the neighbor's wall too (walls are shared)
  int nx = x, ny = y;
  int oppositeDir;
  
  switch (dir) {
    case NORTH:
      ny++;
      oppositeDir = SOUTH;
      break;
    case EAST:
      nx++;
      oppositeDir = WEST;
      break;
    case SOUTH:
      ny--;
      oppositeDir = NORTH;
      break;
    case WEST:
      nx--;
      oppositeDir = EAST;
      break;
  }
  
  if (nx >= 0 && nx < MAZE_SIZE && ny >= 0 && ny < MAZE_SIZE) {
    walls[ny][nx] |= (1 << oppositeDir);
  }
}

// ============================================================================
// FLOOD FILL ALGORITHM
// ============================================================================

void floodFill() {
  // Reset all non-goal distances
  for (int y = 0; y < MAZE_SIZE; y++) {
    for (int x = 0; x < MAZE_SIZE; x++) {
      bool isGoal = false;
      for (int i = 0; i < 4; i++) {
        if (x == goalX[i] && y == goalY[i]) {
          isGoal = true;
          break;
        }
      }
      if (!isGoal) {
        floodValues[y][x] = 255;
      }
    }
  }
  
  // Iteratively flood from goal
  bool changed = true;
  while (changed) {
    changed = false;
    
    for (int y = 0; y < MAZE_SIZE; y++) {
      for (int x = 0; x < MAZE_SIZE; x++) {
        if (floodValues[y][x] == 0) continue; // Skip goal cells
        
        uint8_t minNeighbor = 255;
        
        // Check all four neighbors
        if (!(walls[y][x] & (1 << NORTH)) && y < MAZE_SIZE - 1) {
          minNeighbor = min(minNeighbor, floodValues[y + 1][x]);
        }
        if (!(walls[y][x] & (1 << EAST)) && x < MAZE_SIZE - 1) {
          minNeighbor = min(minNeighbor, floodValues[y][x + 1]);
        }
        if (!(walls[y][x] & (1 << SOUTH)) && y > 0) {
          minNeighbor = min(minNeighbor, floodValues[y - 1][x]);
        }
        if (!(walls[y][x] & (1 << WEST)) && x > 0) {
          minNeighbor = min(minNeighbor, floodValues[y][x - 1]);
        }
        
        if (minNeighbor < 255) {
          uint8_t newValue = minNeighbor + 1;
          if (newValue != floodValues[y][x]) {
            floodValues[y][x] = newValue;
            changed = true;
          }
        }
      }
    }
  }
}

int getNextDirection() {
  // Return direction to move based on lowest neighbor flood value
  int x = robotX;
  int y = robotY;
  uint8_t currentValue = floodValues[y][x];
  
  int bestDir = -1;
  uint8_t bestValue = currentValue;
  
  // Check all four directions
  for (int dir = 0; dir < 4; dir++) {
    if (walls[y][x] & (1 << dir)) continue; // Wall present
    
    int nx = x, ny = y;
    switch (dir) {
      case NORTH: ny++; break;
      case EAST:  nx++; break;
      case SOUTH: ny--; break;
      case WEST:  nx--; break;
    }
    
    if (nx < 0 || nx >= MAZE_SIZE || ny < 0 || ny >= MAZE_SIZE) continue;
    
    if (floodValues[ny][nx] < bestValue) {
      bestValue = floodValues[ny][nx];
      bestDir = dir;
    }
  }
  
  return bestDir;
}

// ============================================================================
// NAVIGATION
// ============================================================================

void exploreStep() {
  // Detect walls at current position
  detectWalls();
  
  // Check if at goal
  for (int i = 0; i < 4; i++) {
    if (robotX == goalX[i] && robotY == goalY[i]) {
      atGoal = true;
      return;
    }
  }
  
  // Recalculate flood fill with new wall information
  floodFill();
  
  // Get next best direction
  int nextDir = getNextDirection();
  
  if (nextDir == -1) {
    Serial.println("ERROR: No valid move!");
    while (1);
  }
  
  // Turn to face next direction
  turnToDirection(nextDir);
  
  // Move forward
  moveForwardOneCell();
  
  delay(100);
}

void turnToDirection(int targetDir) {
  int turns = (targetDir - robotDir + 4) % 4;
  
  switch (turns) {
    case 0:
      // Already facing correct direction
      break;
    case 1:
      turnRight();
      break;
    case 2:
      turnAround();
      break;
    case 3:
      turnLeft();
      break;
  }
}

void navigateToStart() {
  // Change goal to start position
  int tempGoalX[] = {0};
  int tempGoalY[] = {0};
  
  // Backup original goal
  int origGoalX[4], origGoalY[4];
  for (int i = 0; i < 4; i++) {
    origGoalX[i] = goalX[i];
    origGoalY[i] = goalY[i];
  }
  
  // Set temporary goal
  floodValues[0][0] = 0;
  for (int i = 0; i < 4; i++) {
    floodValues[origGoalY[i]][origGoalX[i]] = 255;
  }
  
  floodFill();
  
  // Navigate to start
  while (robotX != 0 || robotY != 0) {
    int nextDir = getNextDirection();
    if (nextDir == -1) break;
    
    turnToDirection(nextDir);
    moveForwardOneCell();
    delay(100);
  }
  
  // Restore original goal
  for (int i = 0; i < 4; i++) {
    goalX[i] = origGoalX[i];
    goalY[i] = origGoalY[i];
    floodValues[goalY[i]][goalX[i]] = 0;
  }
  floodValues[0][0] = 255;
  
  floodFill();
  atGoal = false;
}

void speedRun() {
  Serial.println("\n=== EXECUTING SPEED RUN ===");
  
  // Run to goal using known maze
  while (!atGoal) {
    // Check if at goal
    for (int i = 0; i < 4; i++) {
      if (robotX == goalX[i] && robotY == goalY[i]) {
        atGoal = true;
        break;
      }
    }
    
    if (atGoal) break;
    
    // Get next direction (no wall detection needed - map is known)
    int nextDir = getNextDirection();
    
    if (nextDir == -1) {
      Serial.println("ERROR: Path not found!");
      break;
    }
    
    // Turn and move
    turnToDirection(nextDir);
    moveForwardOneCell();
  }
  
  Serial.println("*** GOAL REACHED IN SPEED RUN! ***");
}

// ============================================================================
// END OF CODE
// ============================================================================
