#pragma once
#include <stdint.h>
#include <stdbool.h>

/*
Devices Input Subsystem
Description:
This subsystem allows to use keyboard, mouse and other types of input
*/

#ifdef __cplusplus
extern "C" {
#endif

bool IsKeyPressed(void* window, uint32_t key);
bool IsButtonPressed(void* window, uint32_t key);
void GetCursorPos(void* window, float* x, float* y);
void SetCursorPos(void* window, float x, float y);
void UseRaw(void* window, bool use);

#ifdef __cplusplus
}
#endif