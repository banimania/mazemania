#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <queue>
#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <stack>
#include <string>
#include <vector>
#if defined(__EMSCRIPTEN__)
  #include <emscripten/emscripten.h>
#endif

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const char* WINDOW_NAME = "MazeMania";

const int GAME_SCREEN_WIDTH = 1280;
const int GAME_SCREEN_HEIGHT = 720;

Camera2D cam;

Vector2 mouse;

RenderTexture2D target;

class Maze {
public:
  int x, y, size;
  std::vector<std::vector<int>> ady;

  int scale = 20;
  int thickness = 5;
 
  std::vector<int> solution;

  Maze(int x, int y, int size) : x(x), y(y), size(size) {
    ady = std::vector<std::vector<int>>(size * size);

    srand(time(NULL));

    std::vector<int> visited(size * size, 0);
    std::stack<int> rec;
    generateMazeDFS(0, visited, rec);

    visitedStep = std::vector<bool>(size * size, false);
  }

  void generateMazeDFS(int n, std::vector<int> &visited, std::stack<int> &rec) {
    rec.push(n);
    visited[n] = 1;
    while (!rec.empty()) {
      n = rec.top();

      std::vector<int> reachable;
      if (n % size > 0 && !visited[n - 1]) reachable.push_back(n - 1);
      if (n % size < size - 1 && !visited[n + 1]) reachable.push_back(n + 1);
      if (n >= size && !visited[n - size]) reachable.push_back(n - size);
      if (n < size * (size - 1) && !visited[n + size]) reachable.push_back(n + size);

      if (reachable.empty()) {
        rec.pop();
        continue;
      }

      int nextCell = reachable[rand() % reachable.size()];
      rec.push(nextCell);
      ady[n].push_back(nextCell);
      ady[nextCell].push_back(n);
      visited[nextCell] = 1;
    }
  }

  void draw() {
    for (int i = 0; i < size; i++) {
      for (int j = 0; j < size; j++) {
        int nCell = i * size + j;
        std::vector<int> adyCell = ady[nCell];
        
        bool arriba = j == 0 || (std::find(adyCell.begin(), adyCell.end(), nCell - 1) == adyCell.end());
        bool abajo = j == size - 1 || (std::find(adyCell.begin(), adyCell.end(), nCell + 1) == adyCell.end());
        bool izquierda = i == 0 || (std::find(adyCell.begin(), adyCell.end(), nCell - size) == adyCell.end());
        bool derecha = i == size - 1 || (std::find(adyCell.begin(), adyCell.end(), nCell + size) == adyCell.end());
        
        if (arriba) DrawRectangle(x + i * scale, y + j * scale, scale + thickness, thickness, WHITE);
        if (abajo) DrawRectangle(x + i * scale, y + (j + 1) * scale, scale + thickness, thickness, WHITE);
        if (izquierda) DrawRectangle(x + i * scale, y + j * scale, thickness, scale + thickness, WHITE);
        if (derecha) DrawRectangle(x + (i + 1) * scale, y + j * scale, thickness, scale + thickness, WHITE);
      }
    }
    DrawRectangle(x + size * scale, y + size * scale, thickness, thickness, WHITE);
    DrawRectangle(x, y + 5, 5, scale - thickness, std::find(solution.begin(), solution.end(), 0) == solution.end() ? BLACK : GREEN);
    DrawRectangle(x + size * scale, y + size * scale - scale + thickness, 5, scale - thickness, std::find(solution.begin(), solution.end(), size * size - 1) == solution.end() ? BLACK : GREEN);
  }

  void drawSolution() {
    for (int i = 0; i < solution.size(); i++) {
      DrawRectangle(x + (solution[i] / size) * scale + thickness, y + (solution[i] % size) * scale + thickness, scale, scale, GREEN);
    }
  }

  void solveMazeDFS() {
    if (!solution.empty()) solution.clear();
    else {
      int target = size * size - 1;

      std::stack<int> rec;
      std::vector<bool> visited(size * size, false);
      rec.push(0);

      while (!rec.empty()) {
        int v = rec.top();
        if (v == target) break;

        if (visited[v]) {
          rec.pop();
          continue;
        }
        visited[v] = true;

        for (int i = 0; i < ady[v].size(); i++) {
          if (!visited[ady[v][i]]) rec.push(ady[v][i]);
        }
      }

      while (!rec.empty()) {
        solution.push_back(rec.top());
        rec.pop();
      }
    } 
  }

  std::stack<int> recStep;
  std::vector<bool> visitedStep;
  void solveMazeDFSStep() {
    int target = size * size - 1;

    if (recStep.empty()) {
      recStep.push(0);
    } else {
      int v = recStep.top();
      if (v == target) return;

      if (visitedStep[v]) {
        recStep.pop();
        solution.push_back(recStep.top());
        return;
      }
      visitedStep[v] = true;

      for (int i = 0; i < ady[v].size(); i++) {
        if (!visitedStep[ady[v][i]]) recStep.push(ady[v][i]);
      }
    }

    solution.push_back(recStep.top());
  }

  std::queue<int> bfsQueue;
  void solveMazeBFSStep() {
    int target = size * size - 1;

    if (bfsQueue.empty()) {
      bfsQueue.push(0);
      visitedStep[0] = true;
      solution.push_back(0);
    } else {
      int v = bfsQueue.front();
      if (v == target) return;
      bfsQueue.pop();

      for (int i = 0; i < ady[v].size(); i++) {
        int neighbor = ady[v][i];
        if (!visitedStep[neighbor]) {
          bfsQueue.push(neighbor);
          visitedStep[neighbor] = true;
          solution.push_back(neighbor);
        }
      }
    }
  }

  void restartMaze() {
    solution.clear();
    bfsQueue = std::queue<int>();
    recStep = std::stack<int>();
    visitedStep = std::vector<bool>(size * size, false);

    std::vector<int> visited(size * size, 0);
    std::stack<int> rec;

    ady = std::vector<std::vector<int>>(size * size);

    generateMazeDFS(0, visited, rec);
  }
};

Maze maze(0, 0, 40);
Maze maze2(40 * 20 + 40, 0, 40);
void mainLoop() {
  float scale = fmin((float) GetScreenWidth() / GAME_SCREEN_WIDTH, (float) GetScreenHeight() / GAME_SCREEN_HEIGHT);

  mouse = GetMousePosition();
  mouse.x = (mouse.x - (GetScreenWidth() - (GAME_SCREEN_WIDTH * scale)) * 0.5f) / scale;
  mouse.y = (mouse.y - (GetScreenHeight() - (GAME_SCREEN_HEIGHT * scale)) * 0.5f) / scale;
  mouse = Vector2Clamp(mouse, (Vector2){ 0, 0 }, (Vector2){ (float) GAME_SCREEN_WIDTH, (float) GAME_SCREEN_HEIGHT });

  if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
    Vector2 delta = GetMouseDelta();
    delta = {delta.x / cam.zoom, delta.y / cam.zoom};
    cam.target = Vector2Subtract(cam.target, delta);
  }

  float wheelDelta = GetMouseWheelMove();
  if (wheelDelta != 0) {
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), cam);

    cam.offset = GetMousePosition();

    cam.target = mouseWorldPos;

    float scaleFactor = 1.0f + (0.25f * fabsf(wheelDelta));
    if (wheelDelta < 0) scaleFactor = 1.0f / scaleFactor;
    cam.zoom = Clamp(cam.zoom * scaleFactor, 0.125f, 64.0f);
  }

  BeginTextureMode(target);
  ClearBackground(BLACK);
  BeginMode2D(cam);

  DrawText("Depth First Search", 200, -50, 40, WHITE);
  DrawText("Breadth First Search", 1000, -50, 40, WHITE);
  DrawText("Spacebar -> Solve", 690, 810, 30, WHITE);
  DrawText("R -> Restart", 730, 840, 30, WHITE);

  //if (IsKeyDown(KEY_SPACE)) maze.solveMazeDFS();
  if (IsKeyDown(KEY_R)) {
    maze.restartMaze();
    maze2.ady = std::vector<std::vector<int>>(maze.ady);
    maze2.solution.clear();
    maze2.recStep = std::stack<int>();
    maze2.bfsQueue = std::queue<int>();
    maze2.visitedStep = std::vector<bool>(maze2.size * maze2.size, false);
    //maze2.restartMaze();
  }
  if (IsKeyDown(KEY_SPACE)) maze.solveMazeDFSStep();
  if (IsKeyDown(KEY_SPACE)) maze2.solveMazeBFSStep();

  maze.drawSolution();
  maze.draw();

  maze2.drawSolution();
  maze2.draw();

  EndMode2D();
  EndTextureMode();

  BeginDrawing();
  ClearBackground(BLACK);
  DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float) target.texture.width, (float) -target.texture.height },
                         (Rectangle){ (GetScreenWidth() - ((float) GAME_SCREEN_WIDTH * scale)) * 0.5f, (GetScreenHeight() - ((float) GAME_SCREEN_HEIGHT * scale)) * 0.5f,
                         (float) GAME_SCREEN_WIDTH * scale, (float) GAME_SCREEN_HEIGHT * scale }, (Vector2) { 0, 0 }, 0.0f, WHITE);
  EndDrawing();
}

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME);

  target = LoadRenderTexture(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
  SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

  SetTargetFPS(60);

  cam = {  };
  cam.zoom = 0.65;
  cam.target.x = -180;
  cam.target.y = -180;
  cam.offset.x = 0;
  cam.offset.y = 0;

#if defined(__EMSCRIPTEN__)
  emscripten_set_main_loop(mainLoop, 240, 1);
#else
  while (!WindowShouldClose()) {
    mainLoop();
  }
#endif
  UnloadRenderTexture(target);
  CloseWindow();
  return 0;
}
