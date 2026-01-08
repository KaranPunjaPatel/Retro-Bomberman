#include <raylib.h>
#include <iostream>

#include "game.h"

int main() 
{
    const Color darkGreen = {20, 160, 133, 255};
    
    // constexpr int screenWidth  = BLOCK_SIZE * COL;
    // constexpr int screenHeight = BLOCK_SIZE * ROW;
    
    constexpr int screenWidth  = 1000;
    constexpr int screenHeight = 600;
    
    // SetConfigFlags(FLAG_FULLSCREEN_MODE);
    SetConfigFlags(
            FLAG_WINDOW_RESIZABLE | 
            FLAG_VSYNC_HINT | 
            FLAG_MSAA_4X_HINT
        );
    InitWindow(screenWidth, screenHeight, "Bomberman!!!");

    SetTargetFPS(100);

    // SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);

    SetRandomSeed(GetTime() * 1000000);

    loadAssets();

    Game game;

    while (!WindowShouldClose())
    {
        game.HandleChanges(false);

        game.HandleInput();

        BeginDrawing();
            ClearBackground(darkGreen);

            game.Draw();

            DrawFPS(50,100);
        EndDrawing();
    }

    unloadAssets();
    
    CloseWindow();
}


/*
#include "raylib.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Raylib Camera2D Example");

    // Define the camera
    Camera2D camera = { 0 };
    camera.target = (Vector2){ 0, 0 };   // Point the camera follows
    // camera.offset = (Vector2){ screenWidth / 2, screenHeight / 2 }; // Camera center
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Move camera with arrow keys
        if (IsKeyDown(KEY_RIGHT)) camera.target.x += 5;
        if (IsKeyDown(KEY_LEFT)) camera.target.x -= 5;
        if (IsKeyDown(KEY_DOWN)) camera.target.y += 5;
        if (IsKeyDown(KEY_UP)) camera.target.y -= 5;

        // Zoom in and out with mouse wheel
        float zoomFactor = GetMouseWheelMove();
        if (zoomFactor != 0) camera.zoom += zoomFactor * 0.1f;

        // Prevent zoom from going too small or too big
        if (camera.zoom < 0.5f) camera.zoom = 0.5f;
        if (camera.zoom > 3.0f) camera.zoom = 3.0f;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Begin Camera Mode
        BeginMode2D(camera);

        // Draw a simple world (a grid for reference)
        for (int y = -500; y <= 500; y += 50) {
            for (int x = -500; x <= 500; x += 50) {
                DrawRectangleLines(x, y, 50, 50, LIGHTGRAY);
            }
        }

        // Draw a red rectangle at world position (0,0)
        DrawRectangle(-25, -25, 50, 50, RED);

        EndMode2D(); // End Camera Mode

        // UI Info
        DrawText("Use Arrow Keys to Move", 10, 10, 20, DARKGRAY);
        DrawText("Scroll to Zoom", 10, 40, 20, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

*/