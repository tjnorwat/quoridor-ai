#include "raylib.h"
#include <cmath>
#include <algorithm>

constexpr int WIDTH = 800;
constexpr int HEIGHT = 900; // extra space for UI at bottom

int main() {
    InitWindow(WIDTH, HEIGHT, "Quoridor");

    const float cellSize = (float)WIDTH / 9.0f;
    const float margin = 6.0f;
    const float wallThickness = std::max(6.0f, cellSize * 0.12f);

    // Wall grids: 8x8 possible wall start positions
    bool hWalls[8][8] = { false };
    bool vWalls[8][8] = { false };

    int whiteWalls = 10;
    int blackWalls = 10;
    bool whiteToMove = true;

    enum Orientation { HORIZONTAL, VERTICAL };
    Orientation selected = HORIZONTAL;

    Color lightBrown = Color{222, 184, 135, 255};
    Color darkBrown = Color{160, 82, 45, 255};
    Color boardLine = Color{99, 60, 44, 255};

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(lightBrown);

        // Draw 9x9 squares
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) {
                float x = c * cellSize + margin/2;
                float y = r * cellSize + margin/2;
                DrawRectangleRec(Rectangle{ x, y, cellSize - margin, cellSize - margin }, darkBrown);
                DrawRectangleLinesEx(Rectangle{ x, y, cellSize - margin, cellSize - margin }, 2.0f, boardLine);
            }
        }

        // Draw existing horizontal walls
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                if (hWalls[r][c]) {
                    float centerX = (c + 1) * cellSize;
                    float centerY = (r + 1) * cellSize;
                    float w = cellSize * 2 - margin;
                    DrawRectangleV(Vector2{ centerX - w/2.0f, centerY - wallThickness/2.0f }, Vector2{ w, wallThickness }, BLACK);
                }
            }
        }

        // Draw existing vertical walls
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                if (vWalls[r][c]) {
                    float centerX = (c + 1) * cellSize;
                    float centerY = (r + 1) * cellSize;
                    float h = cellSize * 2 - margin;
                    DrawRectangleV(Vector2{ centerX - wallThickness/2.0f, centerY - h/2.0f }, Vector2{ wallThickness, h }, BLACK);
                }
            }
        }

        // UI area (bottom)
        float uiTop = 9 * cellSize + 10.0f;
        DrawRectangle(0, (int)uiTop, WIDTH, HEIGHT - (int)uiTop, Color{200, 180, 150, 255});

        // Buttons for wall orientation
        Rectangle hBtn = { 20, uiTop + 20, 140, 40 };
        Rectangle vBtn = { 180, uiTop + 20, 140, 40 };
        DrawRectangleRec(hBtn, selected == HORIZONTAL ? GREEN : Color{180,160,140,255});
        DrawRectangleRec(vBtn, selected == VERTICAL ? GREEN : Color{180,160,140,255});
        DrawText("Horizontal Wall", (int)hBtn.x + 8, (int)hBtn.y + 10, 18, BLACK);
        DrawText("Vertical Wall", (int)vBtn.x + 20, (int)vBtn.y + 10, 18, BLACK);

        // Player wall counts
        const char *whiteLabel = "White Walls:";
        const char *blackLabel = "Black Walls:";
        DrawText(whiteLabel, 360, (int)uiTop + 20, 18, WHITE);
        DrawText(TextFormat("%d", whiteWalls), 520, (int)uiTop + 20, 18, WHITE);
        DrawText(blackLabel, 360, (int)uiTop + 50, 18, BLACK);
        DrawText(TextFormat("%d", blackWalls), 520, (int)uiTop + 50, 18, BLACK);

        DrawText(whiteToMove ? "White to move" : "Black to move", 20, (int)uiTop + 70, 20, MAROON);

        // Handle input
        Vector2 mouse = GetMousePosition();
        bool mouseInBoard = mouse.x >= 0 && mouse.x <= 9 * cellSize && mouse.y >= 0 && mouse.y <= 9 * cellSize;

        // Ghost wall under mouse
        if (mouseInBoard) {
            int gx = std::clamp((int)std::round(mouse.x / cellSize) - 1, 0, 7);
            int gy = std::clamp((int)std::round(mouse.y / cellSize) - 1, 0, 7);
            float centerX = (gx + 1) * cellSize;
            float centerY = (gy + 1) * cellSize;
            if (selected == HORIZONTAL) {
                float w = cellSize * 2 - margin;
                DrawRectangleV(Vector2{ centerX - w/2.0f, centerY - wallThickness/2.0f }, Vector2{ w, wallThickness }, Fade(BLACK, 0.5f));
            } else {
                float h = cellSize * 2 - margin;
                DrawRectangleV(Vector2{ centerX - wallThickness/2.0f, centerY - h/2.0f }, Vector2{ wallThickness, h }, Fade(BLACK, 0.5f));
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // if UI clicked
                if (CheckCollisionPointRec(mouse, hBtn)) {
                    selected = HORIZONTAL;
                } else if (CheckCollisionPointRec(mouse, vBtn)) {
                    selected = VERTICAL;
                } else {
                    // attempt to place wall
                    if (selected == HORIZONTAL) {
                        if (!hWalls[gy][gx]) {
                            if ((whiteToMove && whiteWalls > 0) || (!whiteToMove && blackWalls > 0)) {
                                hWalls[gy][gx] = true;
                                if (whiteToMove) whiteWalls--; else blackWalls--;
                                whiteToMove = !whiteToMove;
                            }
                        }
                    } else {
                        if (!vWalls[gy][gx]) {
                            if ((whiteToMove && whiteWalls > 0) || (!whiteToMove && blackWalls > 0)) {
                                vWalls[gy][gx] = true;
                                if (whiteToMove) whiteWalls--; else blackWalls--;
                                whiteToMove = !whiteToMove;
                            }
                        }
                    }
                }
            }
        } else {
            // clicks outside board (in UI)
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mouse, hBtn)) selected = HORIZONTAL;
                if (CheckCollisionPointRec(mouse, vBtn)) selected = VERTICAL;
            }
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}