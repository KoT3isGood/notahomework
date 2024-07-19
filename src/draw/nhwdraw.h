#pragma once
#include <stdint.h>

// Device

// Creates device
// It allows to use graphics (gpu) api functions
void nhwCreateDevice();

// Destroys device
void nhwDestroyDevice();

// GPU storage

// Creates a buffer at specific allocation with specified size
// Returns pointer to the buffer so it can be deleted afterwards
void* nhwCreateBuffer(uint32_t size, void* allocation);

// Returns buffer size from buffer pointer
uint32_t nhwGetBufferSize(void* buffer);

// Returns buffer device address from buffer pointer
uint64_t nhwGetBufferDeviceAddress(void* buffer);

// Deletes buffer from buffer pointer
void nhwDeleteBuffer(void* buffer);

// Creates an image with dimensions of x,y
// Image will be created at allocation
// Returns pointer to the image so it can be deleted afterwards
void* nhwCreateImage(uint32_t x, uint32_t y, void* buffer);

// Copies data from the buffer to the image
void nhwUpdateImage(void* image, void* buffer);

// Deletes image from image pointer
void nhwDeleteBuffer(void* image);

// Shader pipelines


void nhwSetConstants(void* constants);
void nhwDestroyPipeline(void* pipeline);

struct PipelineInfo {
	uint32_t descriptorsCount;
	enum class DescriptorType: uint32_t {
		UniformBuffer = 0x1,
		StorageBuffer = 0x2,
		Image = 0x4,
		SampledImage = 0x8,
		AccelerationStrucutre = 0x10,
	}* descriptorType;
	uint32_t constantsSize;
};

void nhwSetDescriptor(void* value, PipelineInfo::DescriptorType descriptorType);

struct RasterizationPipelineInfo: PipelineInfo {
	unsigned char* vertexSpirv;
	unsigned char* fragmentSpirv;
};
void* nhwCreateRasterizationPipeline(RasterizationPipelineInfo info);

void nhwSetIndexBuffer();
void nhwSetVertexBuffer();
void nhwDraw();
void nhwDrawIndexed();


struct ComputePipelineInfo : PipelineInfo {
	unsigned char* computeSpirv;
};
void* nhwCreateComputePipeline(ComputePipelineInfo info);

void nhwDispatch(uint32_t x, uint32_t y, uint32_t z);

struct RayTracingPipelineInfo : PipelineInfo {
	unsigned char* raygenSpirv;
	unsigned char* rchitSpirv;
	unsigned char* rmissSpirv;
};
void* nhwCreateRayTracingPipeline(RayTracingPipelineInfo info);





void nhwTraceRays(uint32_t x, uint32_t y);




// Window

struct WindowInfo {

	const char* title;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
};

void* nhwCreateWindow(WindowInfo windowInfo);
WindowInfo nhwGetWindowInfo(void* window);
void nhwSetWindowInfo(void* window, WindowInfo windowInfo);
void nhwDestroyWindow(void* window);
