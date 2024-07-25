#pragma once
#include <stdint.h>

bool IsKeyPressed(void* window, uint32_t key);
bool IsButtonPressed(void* window, uint32_t key);
void GetCursorPos(void* window, float* x, float* y);
void SetCursorPos(void* window, float x, float y);
void UseRaw(void* window, bool use);