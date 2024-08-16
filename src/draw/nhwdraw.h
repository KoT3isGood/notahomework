#pragma once
#include "../nhw.h"
#include <stdint.h>

/*
Drawing subsystem
Description:
This subsystem adds ability to communicate GPU drivers and create windows

Adds:
	-- rendering
	CreateDevice()
	DestroyDevice()

	BeginRendering()
	Render()


	-- buffers
	CreateBuffer()
	DeleteBuffer()
	GetBufferSize()
	GetBufferDeviceAddress()

	-- images
	CreateImage()
	DeleteImage()

	GenerateSampler()
	DestroySampler()
	UpdateImage()
	BarrierImage()
	ClearImage()


	-- pipelines
	UsePipeline()
	SetConstants()
	SetDescriptor()
	DestroyPipeline()

	CreateRasterizationPipeline()
	Record()
	StopRecord()
	SetIndexBuffer()
	SetVertexBuffer()
	Draw()
	DrawIndexed()

	CreateComputePipeline()
	Dispatch()

	CreateRayTracingPipeline()
	TraceRays()

	-- windows
	nhwCreateWindow()
	nhwGetWindowInfo()
	nhwSetWindowInfo()
	nhwDestroyWindow()

	GetWindowImage()

	ShouldClose()
*/







#ifdef __cplusplus
extern "C" {
#endif


// Creates device
// It allows to use graphics (gpu) api functions
void CreateDevice();

void BeginRendering();
void Render();
// Destroys device
void DestroyDevice();

// GPU storage

typedef enum BufferType {
	Uniform,
	Storage,
	Vertex,
	Index,
	AccelerationStructure,
	ShaderBindingTable
} BufferType;

// Creates a buffer at specific allocation with specified size
// Returns pointer to the buffer so it can be deleted afterwards
void* CreateBuffer(uint32_t size, void** allocation, BufferType type);

// Returns buffer size from buffer pointer
uint32_t GetBufferSize(void* buffer);

// Returns buffer device address from buffer pointer
uint64_t GetBufferDeviceAddress(void* buffer);

// Deletes buffer from buffer pointer
void DeleteBuffer(void* buffer);


// Creates an image with dimensions of x,y
// Image will be created at allocation
// Returns pointer to the image so it can be deleted afterwards
void* CreateImage(uint32_t x, uint32_t y, unsigned char imageFormat);

// Copies data from the buffer to the image
void UpdateImage(void* image, void* buffer);

void* GetWindowImage(void* window);

// Deletes image from image pointer
void DeleteImage(void* image);

void BarrierImage(void* image);
void FixDepthImage(void* image);
void ClearImage(void* image);
void* GenerateSampler(void* image);
void DestroySampler(void* image);

// Shader pipelines

void UsePipeline(void* pipeline);

void SetConstants(void* shader,void* constants);

void DestroyPipeline(void* pipeline);

typedef enum DescriptorType {
	UniformBuffer = 1,
	StorageBuffer = 2,
	Image = 3,
	SampledImage = 4,
	AccelerationStrucutreHandle = 5,
} DescriptorType;

typedef struct {
	uint32_t descriptorsCount;
	DescriptorType* descriptorTypes;
	uint32_t constantsSize;
} PipelineInfo;

void SetDescriptor(void* shader, void* value, uint32_t binding);

typedef struct RasterizationPipelineInfo {
	PipelineInfo pipelineInfo;

	unsigned char* vertexSpirv;
	uint32_t vertexSpirvSize;
	unsigned char* fragmentSpirv;
	uint32_t fragmentSpirvSize;

	bool useDepth;
} RasterizationPipelineInfo;
void* CreateRasterizationPipeline(RasterizationPipelineInfo info);

void Record(void* shader, uint32_t x, uint32_t y, void* image, void* depth);
void StopRecord();
void SetIndexBuffer(void* buffer);
void SetVertexBuffer(void* buffer);
void Draw();
void DrawIndexed(uint32_t triangles,uint32_t instances);


typedef struct ComputePipelineInfo {
	PipelineInfo pipelineInfo;
	unsigned char* computeSpirv;
	uint32_t computeSpirvSize;
} ComputePipelineInfo;
void* CreateComputePipeline(ComputePipelineInfo info);

void Dispatch(uint32_t x, uint32_t y, uint32_t z);

typedef struct RayTracingPipelineInfo {
	PipelineInfo pipelineInfo;
	unsigned char* raygenSpirv;
	uint32_t raygenSpirvSize;
	unsigned char* rchitSpirv;
	uint32_t rchitSpirvSize;
	unsigned char* rmissSpirv;
	uint32_t rmissSpirvSize;
} RayTracingPipelineInfo;

// Creates ray tracing pipeline
void* CreateRayTracingPipeline(RayTracingPipelineInfo info);





void TraceRays(uint32_t x, uint32_t y);

// Acceleration structures
void* CreateBLAS(void* vertexBuffer, void* indexBuffer);
void UpdateBLAS(void* blas);
void DestroyBLAS(void* blas);

typedef struct MeshInstance {
	float transformMatrix[3][4];
	uint32_t instanceID;
	void* blas;
} MeshInstance;

void* CreateTLAS(MeshInstance* meshes, uint32_t meshesCount);
void BuildTLAS(void* tlas);
void DestroyTLAS(void* tlas);



// Window

typedef struct WindowInfo{

	const char* title;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
} WindowInfo;

// Creates window
void* nhwCreateWindow(WindowInfo windowInfo);
WindowInfo nhwGetWindowInfo(void* window);
void nhwSetWindowInfo(void* window, WindowInfo windowInfo);
void nhwDestroyWindow(void* window);
bool ShouldClose();

#ifdef __cplusplus
}
#endif
