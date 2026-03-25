#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "API.h"

#define MAZE_WIDTH 16
#define MAZE_HEIGHT 16
#define INFINITY 99999

static int current_x = 0;
static int current_y = 0;
static int direction = 0; // N=0, E=1, S=2, W=3
static int h_n[MAZE_WIDTH][MAZE_HEIGHT];
static int g_n[MAZE_WIDTH][MAZE_HEIGHT];
static int visited[MAZE_WIDTH][MAZE_HEIGHT] = {0};

typedef struct {
    int x, y;
} Node;

static Node path_stack[MAZE_WIDTH * MAZE_HEIGHT];
static int stack_size = 0;
static const int dx[] = {0, 1, 0, -1};
static const int dy[] = {1, 0, -1, 0};

void stack_push(Node n) {
    if (stack_size < MAZE_WIDTH * MAZE_HEIGHT) {
        path_stack[stack_size++] = n;
    }
}

Node stack_pop() {
    if (stack_size > 0) {
        return path_stack[--stack_size];
    }
    return (Node){-1, -1};
}

Node stack_peek() { // return the top of the stack without pop
    if (stack_size > 0) {
        return path_stack[stack_size - 1];
    }
    return (Node){-1, -1};
}

void logMessage(const char* text) {
    fprintf(stderr, "%s\n", text);
    fflush(stderr);
}

int manhattan(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

int calculate_heuristic(int cx, int cy) {
    int h1 = manhattan(cx, cy, 7, 7);
    int h2 = manhattan(cx, cy, 7, 8);
    int h3 = manhattan(cx, cy, 8, 7);
    int h4 = manhattan(cx, cy, 8, 8);

    int minH = h1;
    if (h2 < minH) minH = h2;
    if (h3 < minH) minH = h3;
    if (h4 < minH) minH = h4;
    return minH;
}

void turnTo(int targetDir) {
    int diff = (targetDir - direction + 4) % 4;
    if (diff == 1) {
        API_turnRight();
        }
    else if (diff == 2) {
        API_turnRight();
        API_turnRight();
        }
    else if (diff == 3) {
        API_turnLeft();
        }
    direction = targetDir;
}

void moveForward() {
    if (API_wallFront()) {
        logMessage("Error: Tried to move through a wall!");
        return;}
    API_moveForward();

    current_x += dx[direction];
    current_y += dy[direction];
}

int isWallInDirection(int checkDir) {
    int relativeDir = (checkDir - direction + 4) % 4;
    if (relativeDir == 0) { // Front
        return API_wallFront();
        }
    else if (relativeDir == 1) { // Right
        return API_wallRight();
        }
    else if (relativeDir == 3) { // Left
        return API_wallLeft();
        }
    return 0;
}

void initialize_A_Star() {
    for (int x = 0; x < MAZE_WIDTH; x++) {
        for (int y = 0; y < MAZE_HEIGHT; y++) {
            g_n[x][y] = INFINITY;
            h_n[x][y] = calculate_heuristic(x, y);
        }
    }
    h_n[7][7] = 0; h_n[7][8] = 0; h_n[8][7] = 0; h_n[8][8] = 0;
    g_n[0][0] = 0;
}

void decideAndMove() {
    int current_g_score = g_n[current_x][current_y];
    int min_f_score = INFINITY;
    int best_direction = -1;

    for (int dir = 0; dir < 4; dir++) {
        int nx = current_x + dx[dir];
        int ny = current_y + dy[dir];
        if (nx < 0 || nx >= MAZE_WIDTH || ny < 0 || ny >= MAZE_HEIGHT) {
            continue;
        }
        if (isWallInDirection(dir)) {
            continue;
        }
        if (visited[nx][ny] == 0) {
            int potential_g = current_g_score + 1;
            int f_score = potential_g + h_n[nx][ny];
            if (f_score < min_f_score) {
                min_f_score = f_score;
                best_direction = dir;
            }
        }
    }
    if (best_direction != -1) {
        int nx = current_x + dx[best_direction];
        int ny = current_y + dy[best_direction];
        g_n[nx][ny] = current_g_score + 1;
        visited[nx][ny] = 1;
        turnTo(best_direction);
        moveForward();
        stack_push((Node){current_x, current_y});

    } else {
        stack_pop();

        if (stack_size == 0) {
            logMessage("Fatal Error: Stack is empty, robot is trapped.");
            return;
        }
        Node target = stack_peek();
        int target_dir = -1;
        if (target.x > current_x) target_dir = 1;      // East
        else if (target.x < current_x) target_dir = 3; // West
        else if (target.y > current_y) target_dir = 0; // North
        else if (target.y < current_y) target_dir = 2; // South

        if (target_dir != -1) {
            turnTo(target_dir);
            moveForward();
        }
    }
}

int main(int argc, char* argv[]) {
    logMessage("Initializing A* Micromouse...");
    initialize_A_Star();

    current_x = 0;
    current_y = 0;
    direction = 0;
    stack_push((Node){current_x, current_y});
    visited[current_x][current_y] = 1;
    API_setColor(current_x, current_y, 'G');
    while (h_n[current_x][current_y] != 0) {
        API_setColor(current_x, current_y, 'B');
        decideAndMove();
    }
    logMessage("Goal Reached!");
    API_setColor(current_x, current_y, 'Y');
    return 0;
}
