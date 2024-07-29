#include "nhwtexture.h"
#include <cstring>

void LoadTexture(uint32_t size, void* image, unsigned char* data)
{

	void* alloc;
	void* buffer = CreateBuffer(size, &alloc, Storage);
	memcpy(alloc, data, size);

	UpdateImage(image, buffer);
	DeleteBuffer(buffer);
}
