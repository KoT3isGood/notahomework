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

bool nhwIsKeyPressed(void* window, uint32_t key);
bool nhwIsButtonPressed(void* window, uint32_t key);
void nhwGetCursorPos(void* window, float* x, float* y);
void nhwSetCursorPos(void* window, float x, float y);
void nhwUseRaw(void* window, bool use);

#ifdef __cplusplus
}
#endif