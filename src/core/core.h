#ifndef CORE_H
#define CORE_H

#include "render/color.h"
#include <stdbool.h>

void InitWindow(int width, int height, const char* title);
bool WindowShouldClose();
void ClearBackground(Color color);
void CloseWindow();

void BeginDrawing();
void EndDrawing();

#endif /* CORE_H */

