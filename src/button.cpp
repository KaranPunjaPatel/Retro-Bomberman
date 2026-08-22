
#include <iostream>

#include "button.h"

Button::Button(Rectangle bounds,
                std::string text,
                Color color,
                float roundness,
                int segments,
                int fontSize,
                Color textColor,
                float border,
                Color borderColor,
                float animationOffset,
                float maxAnimationOffset)
    : bounds(bounds), color(color), 
      roundness(roundness), segments(segments),
      text(text), fontSize(fontSize), textColor(textColor),
      border(border), borderColor(borderColor), 
      animationOffset(animationOffset), maxAnimationOffset(maxAnimationOffset)
{

  int tWidth = bounds.width;
  int fSize = this->fontSize;
  while(tWidth > (bounds.width * .75)){
    fSize = --this->fontSize;
    tWidth = MeasureText(text.c_str(), fSize);
  }

  this->textX = bounds.x + (bounds.width - tWidth) / 2;
  this->textY = bounds.y + (bounds.height - fSize) / 2;

}

void Button::Reinitialise(Rectangle bounds, int tempFontSize)
{
  this->bounds = bounds;
  this->fontSize = tempFontSize;

  int tWidth = bounds.width;
  int fSize = this->fontSize;
  while(tWidth > (bounds.width * .75)){
    fSize = --this->fontSize;
    tWidth = MeasureText(text.c_str(), fSize);
  }
  this->textX = bounds.x + (bounds.width - tWidth) / 2;
  this->textY = bounds.y + (bounds.height - fSize) / 2;
}

void Button::Draw()
{
  DrawRectangleRounded(
        bounds, 
        roundness, 
        segments, 
        color
      );

  DrawRectangleRoundedLinesEx(
      Rectangle{
        bounds.x - animationOffset,
        bounds.y - animationOffset,
        bounds.width  + animationOffset*2,
        bounds.height + animationOffset*2
      },
      roundness,
      segments,
      border,
      borderColor
    );


  DrawText(
      text.c_str(), 
      textX, 
      textY, 
      fontSize, 
      textColor
    );
}

void Button::AnimateStart()
{
  if(animationOffset < maxAnimationOffset){
    animationOffset += ANIMATION_START_SPEED * GetFrameTime();
  }
}

void Button::AnimateReturn()
{
  if(animationOffset > DEF_ANIMATION_OFFSET){
    animationOffset -= ANIMATION_RETURN_SPEED * GetFrameTime();
  }
}

LevelButton::LevelButton(Rectangle bounds,
                std::string text,
                int level, 
                int difficulty
            ) : button(bounds, text), level(level), difficulty(difficulty)
{
}

bool CheckCollisionPointButton(Vector2 point, Button button) {
    
    Rectangle rect  = button.bounds;
    float roundness = button.roundness;

    // First, check if inside the main rectangle (ignoring corners)
    float cornerRadius = (roundness * (rect.width < rect.height ? rect.width : rect.height)) / 2.0f;

    Rectangle innerRect = { rect.x + cornerRadius, rect.y, rect.width - 2 * cornerRadius, rect.height };
    if (CheckCollisionPointRec(point, innerRect)) return true;

    innerRect = { rect.x, rect.y + cornerRadius, rect.width, rect.height - 2 * cornerRadius };
    if (CheckCollisionPointRec(point, innerRect)) return true;

    // Now check for collision with the rounded corners (circle collision)
    Vector2 corners[4] = {
      { rect.x + cornerRadius, rect.y + cornerRadius }, // Top-left
      { rect.x + rect.width - cornerRadius, rect.y + cornerRadius }, // Top-right
      { rect.x + cornerRadius, rect.y + rect.height - cornerRadius }, // Bottom-left
      { rect.x + rect.width - cornerRadius, rect.y + rect.height - cornerRadius } // Bottom-right
    };

    for (int i = 0; i < 4; i++) {
      if (CheckCollisionPointCircle(point, corners[i], cornerRadius)) return true;
    }

    return false;
}
