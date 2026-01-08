#pragma once
#include <raylib.h>
#include <string>

#define DEF_ANIMATION_OFFSET -2.0f
#define ANIMATION_OFFSET 10
#define ANIMATION_START_SPEED 50.0f
#define ANIMATION_RETURN_SPEED 30.f

struct Button {
    Rectangle bounds;
    Color color;
    float roundness;
    int segments;
    std::string text;
    int fontSize;
    float textX;
    float textY;
    Color textColor;

    float border;
    Color borderColor;
    float animationOffset;
    float maxAnimationOffset;

    Button(Rectangle bounds,
            std::string text,
            Color color = BLUE,
            float roundness = 0.5f,
            int segments = 2,
            int fontSize = 25,
            Color textColor = WHITE,
            float border = 5.0f,
            Color borderColor = SKYBLUE,
            float animationOffset = DEF_ANIMATION_OFFSET,
            float maxAnimationOffset = 10.0f
        );

    void Reinitialise(Rectangle bounds, int fontSize = 25);

    void Draw();

    void AnimateStart();
    void AnimateReturn();
};

struct LevelButton {
    Button button;
    int level;
    int difficulty;

    LevelButton(Rectangle bounds,
                std::string text,
                int level, 
                int difficulty
            );
};



bool CheckCollisionPointButton(Vector2 point, Button button);
