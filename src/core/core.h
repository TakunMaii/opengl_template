#ifndef CORE_H
#define CORE_H

#include "render/color.h"
#include <stdbool.h>

void InitWindow(int width, int height, const char* title);
bool WindowShouldClose();
void ClearBackground(Color color);
double GetTime(void);
void GetFramebufferSize(int* width, int* height);
void CloseWindow();

void BeginDrawing();
void EndDrawing();

#endif /* CORE_H */
