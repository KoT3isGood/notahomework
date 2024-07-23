#pragma once
#include "../nhw.h"
#include <stdint.h>

// Device

// Creates device
// It allows to use graphics (gpu) api functions
void CreateDevice();

void BeginRendering();
void Render();
// Destroys device
void DestroyDevice();

// GPU storage

// Creates a buffer at specific allocation with specified size
// Returns pointer to the buffer so it can be deleted afterwards
void* CreateBuffer(uint32_t size, void* allocation);

// Returns buffer size from buffer pointer
uint32_t GetBufferSize(void* buffer);

// Returns buffer device address from buffer pointer
uint64_t GetBufferDeviceAddress(void* buffer);

// Deletes buffer from buffer pointer
void DeleteBuffer(void* buffer);

// Creates an image with dimensions of x,y
// Image will be created at allocation
// Returns pointer to the image so it can be deleted afterwards
void* CreateImage(uint32_t x, uint32_t y, void* buffer);

// Copies data from the buffer to the image
void UpdateImage(void* image, void* buffer);

void* GetWindowImage(void* window);

// Deletes image from image pointer
void DeleteImage(void* image);

// Shader pipelines

void UsePipeline(void* pipeline);

void SetConstants(void* shader,void* constants);

void DestroyPipeline(void* pipeline);

typedef enum {
	UniformBuffer = 0x1,
	StorageBuffer = 0x2,
	Image = 0x4,
	SampledImage = 0x8,
	AccelerationStrucutre = 0x10,
} DescriptorType;

typedef struct {
	uint32_t descriptorsCount;
	DescriptorType* descriptorTypes;
	uint32_t constantsSize;
} PipelineInfo;

void SetDescriptor(void* shader, void* value, uint32_t binding);

typedef struct {
	PipelineInfo pipelineInfo;
	unsigned char* vertexSpirv;
	unsigned char* fragmentSpirv;
} RasterizationPipelineInfo;
void* CreateRasterizationPipeline(RasterizationPipelineInfo info);

void SetIndexBuffer();
void SetVertexBuffer();
void Draw();
void DrawIndexed();


typedef struct {
	PipelineInfo pipelineInfo;
	unsigned char* computeSpirv;
	uint32_t computeSpirvSize;
} ComputePipelineInfo;
void* CreateComputePipeline(ComputePipelineInfo info);

void Dispatch(uint32_t x, uint32_t y, uint32_t z);

typedef struct {
	PipelineInfo pipelineInfo;
	unsigned char* raygenSpirv;
	unsigned char* rchitSpirv;
	unsigned char* rmissSpirv;
} RayTracingPipelineInfo;

// Creates ray tracing pipeline
void* CreateRayTracingPipeline(RayTracingPipelineInfo info);





void TraceRays(uint32_t x, uint32_t y);




// Window

typedef struct {

	const char* title;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
} WindowInfo;

// Creates window
void* CreateWindow(WindowInfo windowInfo);
WindowInfo GetWindowInfo(void* window);
void SetWindowInfo(void* window, WindowInfo windowInfo);
void DestroyWindow(void* window);
bool ShouldClose();
