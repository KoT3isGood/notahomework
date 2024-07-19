#include "nhwdraw.h"
#include "glfw/glfw3.h"

void nhwCreateDevice()
{
}

void nhwDestroyDevice()
{
}

void* nhwCreateBuffer(uint32_t size, void* allocation)
{
	return nullptr;
}

uint32_t nhwGetBufferSize(void* buffer)
{
	return 0;
}

uint64_t nhwGetBufferDeviceAddress(void* buffer)
{
	return 0;
}

void nhwDeleteBuffer(void* buffer)
{
}

void nhwSetDescriptor(void* value, PipelineInfo::DescriptorType descriptorType)
{
}

void nhwSetConstants(void* constants)
{
}

void nhwDestroyPipeline(void* pipeline)
{
}

void* nhwCreateImage(uint32_t x, uint32_t y, void* buffer)
{
	return nullptr;
}

void nhwUpdateImage(void* image, void* buffer)
{
}

void* nhwCreateRasterizationPipeline(RasterizationPipelineInfo info)
{
	return nullptr;
}

void nhwSetIndexBuffer()
{
}

void nhwSetVertexBuffer()
{
}

void nhwDraw()
{
}

void nhwDrawIndexed()
{
}

void* nhwCreateComputePipeline(ComputePipelineInfo info)
{
	return nullptr;
}

void nhwDispatch(uint32_t x, uint32_t y, uint32_t z)
{
}

void* nhwCreateRayTracingPipeline(RayTracingPipelineInfo info)
{
	return nullptr;
}

void nhwTraceRays(uint32_t x, uint32_t y)
{
}

void* nhwCreateWindow(WindowInfo windowInfo)
{
	return nullptr;
}

WindowInfo nhwGetWindowInfo(void* window)
{
	return WindowInfo();
}

void nhwSetWindowInfo(void* window, WindowInfo windowInfo)
{
}

void nhwDestroyWindow(void* window)
{
}
