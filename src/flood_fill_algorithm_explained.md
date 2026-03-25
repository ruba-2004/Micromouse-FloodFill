# Flood Fill Algorithm - Detailed Explanation for Micromouse

## Table of Contents
1. Why Flood Fill?
2. Algorithm Fundamentals
3. Step-by-Step Example
4. Implementation Details
5. Optimization Strategies
6. Comparison with Other Algorithms

---

## 1. Why Flood Fill?

### The Micromouse Challenge
The Micromouse robot must:
- Navigate an **unknown maze** (walls not known beforehand)
- Find the **goal** at the center
- Remember the maze layout
- Execute **fastest path** on subsequent runs

### Why Flood Fill is Ideal

**Advantages:**
1. **Guaranteed to find goal** - Always succeeds if path exists
2. **Dynamic updating** - Adapts as new walls are discovered
3. **Optimal path** - Naturally finds shortest route
4. **Memory efficient** - Only stores one number per cell
5. **Simple to implement** - Straightforward logic
6. **Industry standard** - Proven in competitions worldwide

**Comparison to Alternatives:**
- **Wall Following:** Fails if goal is in center (as per rules)
- **Depth-First Search:** Not optimal, wastes time exploring dead ends
- **A* Search:** More complex, requires heuristics, not necessary
- **Flood Fill:** Perfect balance of simplicity and optimality

---

## 2. Algorithm Fundamentals

### Core Concept: Water Analogy

Imagine the maze as a container. If you pour water into the goal cell:
1. Water fills the goal first (distance = 0)
2. Water spreads to adjacent cells (distance = 1)
3. Water continues spreading outward (distance = 2, 3, 4...)
4. Each cell's "flood value" = number of steps from goal

The robot simply "flows downhill" - always moving to the neighboring cell with the lowest flood value.

### Key Components

#### 1. Maze Representation
```cpp
uint8_t walls[16][16];  // Stores which walls exist
uint8_t floodValues[16][16];  // Distance to goal
```

**Wall Storage:**
Each cell stores walls as bit flags:
```
Bit 0 = North wall
Bit 1 = East wall
Bit 2 = South wall
Bit 3 = West wall

Example: walls[0][0] = 0b1101 = walls on North, South, West
         walls[0][0] = 0b0010 = wall only on East
```

#### 2. Coordinate System
```
      North (0)
        ↑
West(3) ← → East(1)
        ↓
      South(2)

Y increases going North
X increases going East

Start: (0,0) at South-West corner
Goal: (7,7), (8,7), (7,8), (8,8) in 16×16 maze
```

#### 3. Direction Encoding
```cpp
#define NORTH 0  // +Y direction
#define EAST  1  // +X direction
#define SOUTH 2  // -Y direction
#define WEST  3  // -X direction
```

---

## 3. Step-by-Step Example

### Simple 5×5 Maze Example

#### Initial State
```
Start at (0,0), Goal at (2,2)

Maze (unknown walls shown as ?):
┌─?─?─?─?─┐
? . ? . ? . ?
?─?─?─?─?─?
? . ? G ? . ?
?─?─?─?─?─?
? . ? . ? . ?
?─?─?─?─?─?
? . ? . ? . ?
?─?─?─?─?─?
? S ? . ? . ?
└─?─?─?─?─┘
```

#### Step 1: Initialize Flood Values
```
All cells start at distance 255 (unknown)
Goal cell set to 0

Flood Values:
┌───┬───┬───┬───┬───┐
│255│255│255│255│255│
├───┼───┼───┼───┼───┤
│255│255│255│255│255│
├───┼───┼───┼───┼───┤
│255│255│ 0 │255│255│  ← Goal at (2,2)
├───┼───┼───┼───┼───┤
│255│255│255│255│255│
├───┼───┼───┼───┼───┤
│255│255│255│255│255│
└───┴───┴───┴───┴───┘
  ↑ Start at (0,0)
```

#### Step 2: Run Initial Flood Fill
```
For each cell: distance = min(neighbor_distances) + 1

After first iteration:
┌───┬───┬───┬───┬───┐
│255│255│255│255│255│
├───┼───┼───┼───┼───┤
│255│ 1 │ 1 │ 1 │255│  ← Neighbors of goal = 1
├───┼───┼───┼───┼───┤
│255│ 1 │ 0 │ 1 │255│
├───┼───┼───┼───┼───┤
│255│255│255│255│255│
├───┼───┼───┼───┼───┤
│255│255│255│255│255│
└───┴───┴───┴───┴───┘

After second iteration:
┌───┬───┬───┬───┬───┐
│255│ 2 │ 2 │ 2 │255│
├───┼───┼───┼───┼───┤
│ 2 │ 1 │ 1 │ 1 │ 2 │
├───┼───┼───┼───┼───┤
│ 2 │ 1 │ 0 │ 1 │ 2 │
├───┼───┼───┼───┼───┤
│255│ 2 │ 2 │ 2 │255│
├───┼───┼───┼───┼───┤
│255│255│255│255│255│
└───┴───┴───┴───┴───┘

Continue until all reachable cells have values...
```

#### Step 3: Robot Explores (First Move)
```
Robot at (0,0) detects walls:
- South wall (boundary)
- West wall (boundary)
- North wall: NO
- East wall: NO

Update maze:
walls[0][0] |= (1 << SOUTH) | (1 << WEST)

Look at neighbors:
- North (0,1): floodValue = 5
- East (1,0): floodValue = 5
- South: WALL
- West: WALL

Choose minimum: Both North and East are equal (5)
Robot chooses North (or could choose East)
```

#### Step 4: Robot Moves North to (0,1)
```
Detect walls at (0,1):
- Left (West): WALL detected
- Front (North): NO WALL
- Right (East): NO WALL

Update maze with new wall information
Recalculate flood fill with updated walls

New flood values might change based on discovered walls
Robot continues moving toward lowest value
```

#### Step 5: Continue Until Goal
```
Robot keeps:
1. Detecting walls
2. Updating maze map
3. Recalculating flood fill
4. Moving to lowest neighbor
5. Repeat

Eventually reaches goal!
```

---

## 4. Implementation Details

### Flood Fill Calculation (Core Algorithm)

```cpp
void floodFill() {
  // 1. Initialize: Set all non-goal cells to maximum distance
  for (int y = 0; y < MAZE_SIZE; y++) {
    for (int x = 0; x < MAZE_SIZE; x++) {
      if (!isGoalCell(x, y)) {
        floodValues[y][x] = 255;
      }
    }
  }
  
  // 2. Iteratively flood from goal outward
  bool changed = true;
  while (changed) {
    changed = false;
    
    for (int y = 0; y < MAZE_SIZE; y++) {
      for (int x = 0; x < MAZE_SIZE; x++) {
        if (floodValues[y][x] == 0) continue; // Skip goal
        
        // Find minimum neighbor distance
        uint8_t minNeighbor = 255;
        
        // Check North
        if (!(walls[y][x] & (1 << NORTH)) && y < MAZE_SIZE-1) {
          minNeighbor = min(minNeighbor, floodValues[y+1][x]);
        }
        // Check East
        if (!(walls[y][x] & (1 << EAST)) && x < MAZE_SIZE-1) {
          minNeighbor = min(minNeighbor, floodValues[y][x+1]);
        }
        // Check South
        if (!(walls[y][x] & (1 << SOUTH)) && y > 0) {
          minNeighbor = min(minNeighbor, floodValues[y-1][x]);
        }
        // Check West
        if (!(walls[y][x] & (1 << WEST)) && x > 0) {
          minNeighbor = min(minNeighbor, floodValues[y][x-1]);
        }
        
        // Update this cell's value
        if (minNeighbor < 255) {
          uint8_t newValue = minNeighbor + 1;
          if (newValue != floodValues[y][x]) {
            floodValues[y][x] = newValue;
            changed = true;  // Continue iterating
          }
        }
      }
    }
  }
}
```

**How It Works:**
1. Start with goal = 0, all others = 255
2. Loop through all cells
3. Each cell = (minimum neighbor) + 1
4. Repeat until no values change
5. Result: Every cell knows its distance to goal

**Why Multiple Iterations?**
- First pass: Only goal's neighbors get updated
- Second pass: Neighbors of those neighbors update
- Continue: Values "ripple" outward from goal
- Stops when all reachable cells have correct distance

### Navigation Logic

```cpp
int getNextDirection() {
  int x = robotX;
  int y = robotY;
  
  int bestDir = -1;
  uint8_t bestValue = floodValues[y][x];  // Current cell value
  
  // Check all four directions
  for (int dir = 0; dir < 4; dir++) {
    // Is there a wall?
    if (walls[y][x] & (1 << dir)) continue;
    
    // Get neighbor coordinates
    int nx = x, ny = y;
    switch (dir) {
      case NORTH: ny++; break;
      case EAST:  nx++; break;
      case SOUTH: ny--; break;
      case WEST:  nx--; break;
    }
    
    // Out of bounds check
    if (nx < 0 || nx >= MAZE_SIZE || 
        ny < 0 || ny >= MAZE_SIZE) continue;
    
    // Is this neighbor better (lower value)?
    if (floodValues[ny][nx] < bestValue) {
      bestValue = floodValues[ny][nx];
      bestDir = dir;
    }
  }
  
  return bestDir;  // Direction to move (-1 if stuck)
}
```

**Greedy Approach:**
- Always move to neighbor with lowest flood value
- This is locally optimal AND globally optimal
- Works because flood fill guarantees shortest path

### Wall Detection and Update

```cpp
void detectWalls() {
  int leftDist = readLeftDistance();
  int frontDist = readFrontDistance();
  int rightDist = readRightDistance();
  
  // Determine which directions have walls
  bool frontWall = (frontDist < WALL_THRESHOLD);
  bool leftWall = (leftDist < WALL_THRESHOLD);
  bool rightWall = (rightDist < WALL_THRESHOLD);
  
  // Convert to absolute directions based on robot heading
  int frontDir = robotDir;
  int leftDir = (robotDir + 3) % 4;  // Turn left from current
  int rightDir = (robotDir + 1) % 4; // Turn right from current
  
  // Update wall map
  if (frontWall) {
    walls[robotY][robotX] |= (1 << frontDir);
    updateNeighborWall(robotX, robotY, frontDir);
  }
  if (leftWall) {
    walls[robotY][robotX] |= (1 << leftDir);
    updateNeighborWall(robotX, robotY, leftDir);
  }
  if (rightWall) {
    walls[robotY][robotX] |= (1 << rightDir);
    updateNeighborWall(robotX, robotY, rightDir);
  }
}
```

**Key Points:**
1. Walls are detected relative to robot (front/left/right)
2. Converted to absolute directions (N/E/S/W)
3. Both cells on either side of wall get updated
4. After wall update, recalculate flood fill

---

## 5. Optimization Strategies

### 1. Modified Flood Fill (Faster Convergence)

Instead of updating all cells every iteration, only update cells that need updating:

```cpp
void optimizedFloodFill() {
  // Use queue of cells to update
  queue<pair<int,int>> toUpdate;
  
  // Start with goal cells
  for (int i = 0; i < 4; i++) {
    toUpdate.push({goalX[i], goalY[i]});
  }
  
  while (!toUpdate.empty()) {
    auto [x, y] = toUpdate.front();
    toUpdate.pop();
    
    // Check all neighbors
    for (int dir = 0; dir < 4; dir++) {
      if (walls[y][x] & (1 << dir)) continue;
      
      int nx = getNeighborX(x, dir);
      int ny = getNeighborY(y, dir);
      
      if (!inBounds(nx, ny)) continue;
      
      // Should neighbor be updated?
      uint8_t newValue = floodValues[y][x] + 1;
      if (newValue < floodValues[ny][nx]) {
        floodValues[ny][nx] = newValue;
        toUpdate.push({nx, ny});  // Update its neighbors too
      }
    }
  }
}
```

**Benefit:** Much faster for large mazes (16×16)

### 2. Diagonal Movement (Advanced)

Allow diagonal moves when both adjacent cells are free:

```cpp
// Check diagonal: North-East
if (!(walls[y][x] & (1<<NORTH)) && 
    !(walls[y][x] & (1<<EAST)) &&
    !(walls[y+1][x] & (1<<EAST)) &&
    !(walls[y][x+1] & (1<<NORTH))) {
  // Diagonal is clear
  // Distance = sqrt(2) ≈ 1.4 cells
}
```

**Benefit:** Shorter paths in open areas

### 3. Speed Run Optimization

After mapping maze, optimize path:

```cpp
void speedRunOptimization() {
  // 1. Use flood fill to get basic path
  // 2. Identify straight sections
  // 3. Minimize turning points
  // 4. Use higher speeds in straightaways
  // 5. Slow down only for turns
}
```

**Advanced Techniques:**
- **Smooth turns:** Don't stop at each cell
- **Variable speed:** Fast in straights, slow in turns
- **Path smoothing:** Cut corners where safe

### 4. Intelligent Exploration

Don't just follow flood fill blindly during exploration:

```cpp
// Prefer unexplored areas
if (cellVisitCount[ny][nx] == 0) {
  priority += EXPLORATION_BONUS;
}

// Prefer directions that lead to unexplored regions
// This maps maze faster than pure flood fill
```

---

## 6. Comparison with Other Algorithms

### Wall Following (Left-Hand Rule)
```
Pros:
- Very simple to implement
- Guaranteed to return to start
- No memory needed

Cons:
- Cannot find center goal (fails competition)
- Not optimal path
- Gets stuck in loops

Verdict: ❌ Not suitable for Micromouse
```

### Depth-First Search (DFS)
```
Pros:
- Explores entire maze
- Finds goal eventually
- Simple recursion

Cons:
- Not optimal (explores dead ends fully)
- Requires stack (memory intensive)
- Slow exploration

Verdict: ⚠️ Works but inefficient
```

### Breadth-First Search (BFS)
```
Pros:
- Finds shortest path
- Systematic exploration
- Guaranteed optimal

Cons:
- Requires full maze knowledge (must explore everything)
- Memory intensive (queue of cells)
- Doesn't adapt well to dynamic discovery

Verdict: ⚠️ Good for offline planning, not online exploration
```

### A* Search
```
Pros:
- Optimal path
- Faster than BFS (with good heuristic)
- Widely used in robotics

Cons:
- More complex to implement
- Requires heuristic function
- Unnecessary for simple grid maze

Verdict: ⚠️ Overkill for Micromouse (flood fill is simpler)
```

### Flood Fill
```
Pros:
- ✅ Optimal path guaranteed
- ✅ Simple implementation
- ✅ Adapts to new wall discoveries
- ✅ Memory efficient (just distances)
- ✅ Industry standard for competitions
- ✅ Fast enough for real-time

Cons:
- Requires full recalculation on wall update (optimizable)

Verdict: ✅ BEST CHOICE for Micromouse
```

---

## 7. Common Pitfalls and Solutions

### Pitfall 1: Infinite Loop
**Problem:** Robot gets stuck going back and forth

**Cause:** All neighbors have same or higher flood value

**Solution:**
```cpp
// Add visited tracking
bool visited[16][16] = {false};

// In navigation:
if (visited[nx][ny]) {
  penalty += 100;  // Prefer unvisited cells
}
```

### Pitfall 2: Wrong Wall Detection
**Problem:** Robot adds non-existent walls

**Cause:** Incorrect WALL_THRESHOLD or sensor noise

**Solution:**
- Calibrate WALL_THRESHOLD properly (see calibration guide)
- Use multiple readings and average
- Verify walls before permanently adding

### Pitfall 3: Slow Flood Fill
**Problem:** Takes too long to recalculate

**Cause:** Inefficient algorithm for 16×16 maze

**Solution:**
- Use optimized queue-based flood fill
- Only recalculate when walls change
- Cache results when possible

### Pitfall 4: Position Tracking Error
**Problem:** Robot thinks it's at wrong cell

**Cause:** Encoder drift, missed turns

**Solution:**
- Use gyroscope to verify heading
- Use walls for position correction
- Reset encoder counts each cell

---

## 8. Summary

### Why Flood Fill Wins

1. **Simplicity:** Easy to understand and debug
2. **Optimality:** Always finds shortest path
3. **Adaptability:** Handles unknown mazes perfectly
4. **Efficiency:** Fast enough for real-time competition
5. **Proven:** Used by winning teams worldwide

### Algorithm Flow (Complete)

```
1. INITIALIZATION:
   - Set goal cells to distance 0
   - All other cells to distance 255
   - Assume all internal walls exist
   - Add boundary walls

2. INITIAL FLOOD FILL:
   - Calculate distances from goal
   - Creates initial path estimate

3. EXPLORATION LOOP:
   while (not at goal) {
     a. Detect walls with sensors
     b. Update maze map
     c. Recalculate flood fill
     d. Get best direction (lowest neighbor)
     e. Turn to face that direction
     f. Move forward one cell
     g. Update robot position
   }

4. RETURN TO START:
   - Change goal to start position (0,0)
   - Recalculate flood fill
   - Navigate back to start

5. SPEED RUN:
   - Use known maze (no wall detection)
   - Follow optimal path at high speed
   - Execute fastest possible run
```

### Key Takeaways

✅ Flood fill naturally finds shortest paths
✅ Works perfectly with incremental wall discovery
✅ Simple enough to implement reliably
✅ Fast enough for competition use
✅ Industry-proven algorithm

**Your robot will use flood fill to:**
- Explore the unknown maze efficiently
- Always know the shortest path to goal
- Adapt instantly to new wall discoveries
- Execute blazing-fast speed runs

Good luck in the competition!
