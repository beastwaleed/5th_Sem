#include <iostream>
using namespace std;

// Global variables
int grid[16][16];
bool visited[16][16];

// Movement directions
int moveRow[] = { -1, 1, 0, 0, -1, -1 };
int moveCol[] = { 0, 0, 1, -1, 1, -1 };
int moveCost[] = { 2, -1, 2, -1, 3, -3 };
string moveName[] = { "Up", "Down", "Right", "Left", "Diag-Up-Right", "Diag-Up-Left" };

// Simple state structure
struct Node {
    int r, c, cost;
    int parent;
};

Node allNodes[100000];
int nodeCount = 0;

// Manual Queue for BFS
int manualQueue[100000];
int qFront = 0;
int qRear = 0;

void queuePush(int val) {
    manualQueue[qRear] = val;
    qRear++;
}

int queueFront() {
    return manualQueue[qFront];
}

void queuePop() {
    qFront++;
}

bool queueEmpty() {
    return qFront >= qRear;
}

void queueReset() {
    qFront = 0;
    qRear = 0;
}

// Manual Stack for DFS
int manualStack[100000];
int sTop = -1;

void stackPush(int val) {
    sTop++;
    manualStack[sTop] = val;
}

int stackTop() {
    return manualStack[sTop];
}

void stackPop() {
    sTop--;
}

bool stackEmpty() {
    return sTop < 0;
}

void stackReset() {
    sTop = -1;
}

// Setup obstacles
void setupGrid() {
    // Clear grid
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            grid[i][j] = 0;
            visited[i][j] = false;
        }
    }

    // Mark obstacles as 1
    grid[2][3] = 1; grid[2][4] = 1; grid[2][5] = 1; grid[2][6] = 1;
    grid[2][8] = 1; grid[2][9] = 1; grid[2][10] = 1; grid[2][13] = 1; grid[2][15] = 1;

    grid[3][8] = 1; grid[3][9] = 1; grid[3][10] = 1;

    grid[4][1] = 1; grid[4][2] = 1; grid[4][3] = 1; grid[4][4] = 1;

    grid[5][1] = 1; grid[5][2] = 1; grid[5][3] = 1; grid[5][4] = 1;
    grid[5][6] = 1; grid[5][7] = 1; grid[5][8] = 1; grid[5][11] = 1; grid[5][13] = 1;

    grid[6][6] = 1; grid[6][7] = 1; grid[6][8] = 1; grid[6][11] = 1; grid[6][13] = 1;

    grid[7][6] = 1; grid[7][7] = 1; grid[7][8] = 1; grid[7][9] = 1; grid[7][11] = 1;

    grid[8][1] = 1; grid[8][2] = 1; grid[8][6] = 1; grid[8][7] = 1;
    grid[8][8] = 1; grid[8][9] = 1; grid[8][11] = 1;

    grid[9][4] = 1; grid[9][5] = 1; grid[9][6] = 1; grid[9][7] = 1;
    grid[9][8] = 1; grid[9][11] = 1; grid[9][13] = 1; grid[9][14] = 1;

    grid[10][11] = 1; grid[10][12] = 1; grid[10][13] = 1; grid[10][14] = 1;

    grid[11][1] = 1; grid[11][2] = 1; grid[11][3] = 1; grid[11][6] = 1;
    grid[11][7] = 1; grid[11][8] = 1; grid[11][9] = 1; grid[11][12] = 1;

    grid[12][1] = 1; grid[12][2] = 1; grid[12][3] = 1; grid[12][6] = 1;
    grid[12][7] = 1; grid[12][8] = 1; grid[12][9] = 1;

    grid[14][10] = 1; grid[14][11] = 1; grid[14][13] = 1;

    grid[15][1] = 1; grid[15][2] = 1; grid[15][3] = 1; grid[15][4] = 1;
    grid[15][5] = 1; grid[15][6] = 1; grid[15][10] = 1; grid[15][11] = 1; grid[15][13] = 1;
}

// Check if position is safe
bool isSafe(int r, int c) {
    if (r < 1 || r > 15 || c < 1 || c > 15) return false;
    if (grid[r][c] == 1) return false;
    if (visited[r][c]) return false;
    return true;
}

// Print the grid
void printGrid(int pathIndexes[], int pathSize) {
    char display[16][16];

    for (int i = 1; i <= 15; i++) {
        for (int j = 1; j <= 15; j++) {
            if (grid[i][j] == 1) display[i][j] = '1';
            else display[i][j] = '0';
        }
    }

    for (int i = 0; i < pathSize; i++) {
        int idx = pathIndexes[i];
        display[allNodes[idx].r][allNodes[idx].c] = '*';
    }

    display[1][2] = 'S';
    display[15][14] = 'G';

    cout << "\nGrid: S=Start, G=Goal, *=Path, 1=Obstacle, 0=Empty\n\n";
    for (int i = 1; i <= 15; i++) {
        for (int j = 1; j <= 15; j++) {
            cout << display[i][j] << " ";
        }
        cout << "\n";
    }
}

// Print path
void printPath(int pathIndexes[], int pathSize, int totalCost) {
    cout << "\nPath Found: YES\n";
    cout << "Total Cost: " << totalCost << "\n";
    cout << "Steps: " << pathSize - 1 << "\n\n";

    cout << "Path:\n";
    for (int i = 0; i < pathSize; i++) {
        int idx = pathIndexes[i];
        cout << "(" << allNodes[idx].r << "," << allNodes[idx].c << ")";
        if (i < pathSize - 1) cout << " -> ";
    }
    cout << "\n";
}

// ========== BFS ==========
void BFS() {
    setupGrid();
    nodeCount = 0;
    queueReset();

    // Add start node
    Node start;
    start.r = 1;
    start.c = 2;
    start.cost = 0;
    start.parent = -1;
    allNodes[nodeCount] = start;
    nodeCount++;

    queuePush(0);
    visited[1][2] = true;

    int goalIndex = -1;

    while (!queueEmpty()) {
        int currentIdx = queueFront();
        queuePop();

        int r = allNodes[currentIdx].r;
        int c = allNodes[currentIdx].c;
        int cost = allNodes[currentIdx].cost;

        // Check if goal
        if (r == 15 && c == 14) {
            goalIndex = currentIdx;
            break;
        }

        // Try all 6 moves
        for (int i = 0; i < 6; i++) {
            int newR = r + moveRow[i];
            int newC = c + moveCol[i];

            if (isSafe(newR, newC)) {
                Node newNode;
                newNode.r = newR;
                newNode.c = newC;
                newNode.cost = cost + moveCost[i];
                newNode.parent = currentIdx;

                allNodes[nodeCount] = newNode;
                queuePush(nodeCount);
                nodeCount++;
                visited[newR][newC] = true;
            }
        }
    }

    if (goalIndex == -1) {
        cout << "\nPath Found: NO\n";
        return;
    }

    // Build path backwards
    int path[1000];
    int pathSize = 0;
    int idx = goalIndex;
    while (idx != -1) {
        path[pathSize] = idx;
        pathSize++;
        idx = allNodes[idx].parent;
    }

    // Reverse path
    int finalPath[1000];
    for (int i = 0; i < pathSize; i++) {
        finalPath[i] = path[pathSize - 1 - i];
    }

    printPath(finalPath, pathSize, allNodes[goalIndex].cost);
    printGrid(finalPath, pathSize);
}

// ========== DFS with Stack ==========
void DFS_Stack() {
    setupGrid();
    nodeCount = 0;
    stackReset();

    Node start;
    start.r = 1;
    start.c = 2;
    start.cost = 0;
    start.parent = -1;
    allNodes[nodeCount] = start;
    nodeCount++;

    stackPush(0);
    visited[1][2] = true;

    int goalIndex = -1;

    while (!stackEmpty()) {
        int currentIdx = stackTop();
        stackPop();

        int r = allNodes[currentIdx].r;
        int c = allNodes[currentIdx].c;
        int cost = allNodes[currentIdx].cost;

        if (r == 15 && c == 14) {
            goalIndex = currentIdx;
            break;
        }

        for (int i = 0; i < 6; i++) {
            int newR = r + moveRow[i];
            int newC = c + moveCol[i];

            if (isSafe(newR, newC)) {
                Node newNode;
                newNode.r = newR;
                newNode.c = newC;
                newNode.cost = cost + moveCost[i];
                newNode.parent = currentIdx;

                allNodes[nodeCount] = newNode;
                stackPush(nodeCount);
                nodeCount++;
                visited[newR][newC] = true;
            }
        }
    }

    if (goalIndex == -1) {
        cout << "\nPath Found: NO\n";
        return;
    }

    int path[1000];
    int pathSize = 0;
    int idx = goalIndex;
    while (idx != -1) {
        path[pathSize] = idx;
        pathSize++;
        idx = allNodes[idx].parent;
    }

    int finalPath[1000];
    for (int i = 0; i < pathSize; i++) {
        finalPath[i] = path[pathSize - 1 - i];
    }

    printPath(finalPath, pathSize, allNodes[goalIndex].cost);
    printGrid(finalPath, pathSize);
}

// ========== DFS Recursive Helper ==========
bool DFS_Helper(int r, int c, int cost, int parentIdx, int path[], int& pathSize) {
    if (r == 15 && c == 14) {
        Node goal;
        goal.r = r;
        goal.c = c;
        goal.cost = cost;
        goal.parent = parentIdx;
        allNodes[nodeCount] = goal;
        path[pathSize] = nodeCount;
        pathSize++;
        nodeCount++;
        return true;
    }

    for (int i = 0; i < 6; i++) {
        int newR = r + moveRow[i];
        int newC = c + moveCol[i];

        if (isSafe(newR, newC)) {
            visited[newR][newC] = true;

            Node newNode;
            newNode.r = newR;
            newNode.c = newC;
            newNode.cost = cost + moveCost[i];
            newNode.parent = parentIdx;
            allNodes[nodeCount] = newNode;

            int newIdx = nodeCount;
            nodeCount++;
            path[pathSize] = newIdx;
            pathSize++;

            if (DFS_Helper(newR, newC, cost + moveCost[i], newIdx, path, pathSize)) {
                return true;
            }

            pathSize--;
            visited[newR][newC] = false;
        }
    }
    return false;
}

void DFS_Recursive() {
    setupGrid();
    nodeCount = 0;

    Node start;
    start.r = 1;
    start.c = 2;
    start.cost = 0;
    start.parent = -1;
    allNodes[nodeCount] = start;
    nodeCount++;

    visited[1][2] = true;

    int path[1000];
    int pathSize = 0;
    path[pathSize] = 0;
    pathSize++;

    if (DFS_Helper(1, 2, 0, 0, path, pathSize)) {
        int goalIdx = path[pathSize - 1];
        printPath(path, pathSize, allNodes[goalIdx].cost);
        printGrid(path, pathSize);
    }
    else {
        cout << "\nPath Found: NO\n";
    }
}

// ========== Dijkstra ==========
void Dijkstra() {
    setupGrid();

    int dist[16][16];
    int parent[16][16];

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            dist[i][j] = 99999;
            parent[i][j] = -1;
            visited[i][j] = false;
        }
    }

    cout << "\nNote: This problem has negative costs (-1, -3)\n";
    cout << "Dijkstra may not give correct answer with negative costs!\n";
    cout << "Use Bellman-Ford for accurate results.\n";

    dist[1][2] = 0;

    for (int count = 0; count < 225; count++) {
        int minDist = 99999;
        int minR = -1, minC = -1;

        for (int i = 1; i <= 15; i++) {
            for (int j = 1; j <= 15; j++) {
                if (!visited[i][j] && dist[i][j] < minDist) {
                    minDist = dist[i][j];
                    minR = i;
                    minC = j;
                }
            }
        }

        if (minR == -1) break;

        visited[minR][minC] = true;

        if (minR == 15 && minC == 14) break;

        for (int i = 0; i < 6; i++) {
            int newR = minR + moveRow[i];
            int newC = minC + moveCol[i];

            if (newR >= 1 && newR <= 15 && newC >= 1 && newC <= 15 &&
                grid[newR][newC] != 1 && !visited[newR][newC]) {

                int newDist = dist[minR][minC] + moveCost[i];
                if (newDist < dist[newR][newC]) {
                    dist[newR][newC] = newDist;
                    parent[newR][newC] = minR * 100 + minC;
                }
            }
        }
    }

    if (dist[15][14] == 99999) {
        cout << "\nPath Found: NO\n";
        return;
    }

    int pathR[1000], pathC[1000];
    int pathSize = 0;
    int r = 15, c = 14;

    while (!(r == 1 && c == 2)) {
        pathR[pathSize] = r;
        pathC[pathSize] = c;
        pathSize++;
        int p = parent[r][c];
        if (p == -1) break;
        r = p / 100;
        c = p % 100;
    }
    pathR[pathSize] = 1;
    pathC[pathSize] = 2;
    pathSize++;

    cout << "\nPath Found: YES\n";
    cout << "Total Cost: " << dist[15][14] << "\n";
    cout << "Steps: " << pathSize - 1 << "\n\n";

    cout << "Path:\n";
    for (int i = pathSize - 1; i >= 0; i--) {
        cout << "(" << pathR[i] << "," << pathC[i] << ")";
        if (i > 0) cout << " -> ";
    }
    cout << "\n";

    char display[16][16];
    for (int i = 1; i <= 15; i++) {
        for (int j = 1; j <= 15; j++) {
            if (grid[i][j] == 1) display[i][j] = '1';
            else display[i][j] = '0';
        }
    }
    for (int i = 0; i < pathSize; i++) {
        display[pathR[i]][pathC[i]] = '*';
    }
    display[1][2] = 'S';
    display[15][14] = 'G';

    cout << "\nGrid:\n";
    for (int i = 1; i <= 15; i++) {
        for (int j = 1; j <= 15; j++) {
            cout << display[i][j] << " ";
        }
        cout << "\n";
    }
}

// ========== Bellman Ford ==========
void BellmanFord() {
    setupGrid();

    int dist[16][16];
    int parent[16][16];

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            dist[i][j] = 99999;
            parent[i][j] = -1;
        }
    }

    dist[1][2] = 0;

    for (int times = 0; times < 225; times++) {
        bool changed = false;

        for (int r = 1; r <= 15; r++) {
            for (int c = 1; c <= 15; c++) {
                if (dist[r][c] == 99999) continue;
                if (grid[r][c] == 1) continue;

                for (int i = 0; i < 6; i++) {
                    int newR = r + moveRow[i];
                    int newC = c + moveCol[i];

                    if (newR >= 1 && newR <= 15 && newC >= 1 && newC <= 15 &&
                        grid[newR][newC] != 1) {

                        int newDist = dist[r][c] + moveCost[i];
                        if (newDist < dist[newR][newC]) {
                            dist[newR][newC] = newDist;
                            parent[newR][newC] = r * 100 + c;
                            changed = true;
                        }
                    }
                }
            }
        }
        if (!changed) break;
    }

    if (dist[15][14] == 99999) {
        cout << "\nPath Found: NO (Goal Unreachable)\n";
        return;
    }

    int pathR[1000], pathC[1000];
    int pathSize = 0;
    int r = 15, c = 14;
    int safetyCounter = 0;

    while (!(r == 1 && c == 2)) {
        pathR[pathSize] = r;
        pathC[pathSize] = c;
        pathSize++;

        int p = parent[r][c];

        if (p == -1) {
            cout << "\nError: Path is broken (Parent is -1). Cannot reach start.\n";
            return;
        }

        r = p / 100;
        c = p % 100;

        safetyCounter++;
        if (safetyCounter > 300) {
            cout << "\n\n[ERROR] Negative Cycle Detected!\n";
            cout << "The robot is stuck in an infinite loop reducing cost forever.\n";
            cout << "Path reconstruction stopped to prevent crash.\n";
            return;
        }
    }

    pathR[pathSize] = 1;
    pathC[pathSize] = 2;
    pathSize++;

    cout << "\nPath Found: YES\n";
    cout << "Total Cost: " << dist[15][14] << "\n";
    cout << "Steps: " << pathSize - 1 << "\n\n";

    cout << "Path:\n";
    for (int i = pathSize - 1; i >= 0; i--) {
        cout << "(" << pathR[i] << "," << pathC[i] << ")";
        if (i > 0) cout << " -> ";
    }
    cout << "\n";

    char display[16][16];
    for (int i = 1; i <= 15; i++) {
        for (int j = 1; j <= 15; j++) {
            if (grid[i][j] == 1) display[i][j] = '1';
            else display[i][j] = '0';
        }
    }
    for (int i = 0; i < pathSize; i++) {
        display[pathR[i]][pathC[i]] = '*';
    }
    display[1][2] = 'S';
    display[15][14] = 'G';

    cout << "\nGrid:\n";
    for (int i = 1; i <= 15; i++) {
        for (int j = 1; j <= 15; j++) {
            cout << display[i][j] << " ";
        }
        cout << "\n";
    }
}

// ========== MST Explanation ==========
void explainMST() {
    cout << "\n=== WHY KRUSKAL AND PRIM DON'T WORK ===\n\n";
    cout << "KRUSKAL'S ALGORITHM:\n";
    cout << "- Makes a tree connecting ALL cells\n";
    cout << "- We only need path from start to goal\n";
    cout << "- NOT for pathfinding\n\n";

    cout << "PRIM'S ALGORITHM:\n";
    cout << "- Also makes a tree connecting ALL cells\n";
    cout << "- We only need path from start to goal\n";
    cout << "- NOT for pathfinding\n\n";

    cout << "FOR THIS PROBLEM USE:\n";
    cout << "- BFS (simple, finds a path)\n";
    cout << "- DFS (simple, finds a path)\n";
    cout << "- Bellman-Ford (BEST - finds shortest path with negative costs)\n";
}

int main() {
    while (true) {
        cout << "\n==============================\n";
        cout << "   ROBOT NAVIGATION\n";
        cout << "==============================\n";
        cout << "1. BFS\n";
        cout << "2. DFS (Stack)\n";
        cout << "3. DFS (Recursive)\n";
        cout << "4. Dijkstra\n";
        cout << "5. Bellman-Ford\n";
        cout << "6. Why MST doesn't work?\n";
        cout << "7. Exit\n";
        cout << "==============================\n";
        cout << "Choice: ";

        int choice;
        cin >> choice;

        if (choice == 1) {
            cout << "\n--- Running BFS ---\n";
            BFS();
        }
        else if (choice == 2) {
            cout << "\n--- Running DFS (Stack) ---\n";
            DFS_Stack();
        }
        else if (choice == 3) {
            cout << "\n--- Running DFS (Recursive) ---\n";
            DFS_Recursive();
        }
        else if (choice == 4) {
            cout << "\n--- Running Dijkstra ---\n";
            Dijkstra();
        }
        else if (choice == 5) {
            cout << "\n--- Running Bellman-Ford ---\n";
            BellmanFord();
        }
        else if (choice == 6) {
            explainMST();
        }
        else if (choice == 7) {
            cout << "\nGoodbye!\n";
            break;
        }
        else {
            cout << "\nInvalid choice!\n";
        }
    }

    return 0;
}