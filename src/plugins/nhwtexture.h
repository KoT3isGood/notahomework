#pragma once
#include "../draw/nhwdraw.h"

/*
Textures Plugin
Description:
This plugin allows to load texture data right into the image

Adds:
	LoadTexture()
*/
#ifdef __cplusplus
extern "C" {
#endif

void LoadTexture(uint32_t size, void* image, unsigned char* data);

#ifdef __cplusplus
}
#endif