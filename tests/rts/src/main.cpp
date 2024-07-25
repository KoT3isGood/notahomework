#include "nhw.h"
#include "draw/nhwdraw.h"
#include "device/nhwdevice.h"
#include <stdio.h>
#include "stdarg.h"
#include <cstring>

void logMessage(const char* msg, va_list args) {

	vprintf(msg,args); printf("\n");
}

int main() {



	nhwInstanceInfo InstanceInfo{
		logMessage
	};
	CreateInstance(InstanceInfo);


	
	Log("Hi");
	CreateDevice();


	ComputePipelineInfo cpi{};
	unsigned char* computeShader = LoadFileData("computeShader.comp.spv", &cpi.computeSpirvSize);

	
	cpi.pipelineInfo = {};
	cpi.pipelineInfo.constantsSize = 8;
	cpi.pipelineInfo.descriptorsCount = 1;
	DescriptorType descriptors[1] = { DescriptorType::Image};
	cpi.pipelineInfo.descriptorTypes = descriptors;
	cpi.computeSpirv = computeShader;
	void* shader = CreateComputePipeline(cpi);


	RasterizationPipelineInfo rpi{};

	rpi.pipelineInfo = {};
	rpi.pipelineInfo.constantsSize = 8;
	rpi.pipelineInfo.descriptorsCount = 0;

	unsigned char* fragmentShader = LoadFileData("shader.frag.spv", &rpi.fragmentSpirvSize);
	unsigned char* vertexShader = LoadFileData("shader.vert.spv", &rpi.vertexSpirvSize);
	rpi.fragmentSpirv = fragmentShader;
	rpi.vertexSpirv = vertexShader;
	void* triShader = CreateRasterizationPipeline(rpi);
	
	WindowInfo winInfo{};
	winInfo.title = R"(Real real time real time strategy named "Real time strategy example" example of real time strategy example)";
	winInfo.width = 1280;
	winInfo.height = 720;
	void* window = CreateWindow(winInfo);

	float triangleVertices[] = {
	0.0,1.0,0.0,
	1.0,-1.0,0.0,
	0.0,-1.0,0.0,
	-1.0,-1.0,0.0,

	};

	void* triangleVerticesPtr = nullptr;

	void* vertexBuffer = CreateBuffer(sizeof(triangleVertices),&triangleVerticesPtr, BufferType::Vertex);

	memcpy(triangleVerticesPtr, triangleVertices, sizeof(triangleVertices));
	Log("%f,%f,%f", ((float*)triangleVerticesPtr)[0], ((float*)triangleVerticesPtr)[1], ((float*)triangleVerticesPtr)[2]);

	uint32_t triangleIndexes[] = {
		0,1,2,1,2,3
	};
	void* triangleIndexesPtr = nullptr;
	void* indexBuffer = CreateBuffer(sizeof(triangleIndexes), &triangleIndexesPtr, BufferType::Index);
	memcpy(triangleIndexesPtr, triangleIndexes, sizeof(triangleIndexes));
	void* image = GetWindowImage(window);
	

	while (!ShouldClose()) {

		DeleteImage(image);
		image = GetWindowImage(window);

		uint32_t resolution[2] = { winInfo.width,winInfo.height };
		

		SetDescriptor(shader, image, 0);






		BeginRendering();



		

		winInfo = GetWindowInfo(window);

		ClearImage(image);
		BarrierImage(image);
		UsePipeline(shader);
		SetConstants(shader, resolution);
		Dispatch(resolution[0], resolution[1], 1);
		BarrierImage(image);

		Record(triShader, resolution[0], resolution[1], image, nullptr);
		SetVertexBuffer(vertexBuffer);
		SetIndexBuffer(indexBuffer);

		DrawIndexed(6, 1);

		StopRecord();

		BarrierImage(image);

		Render();

	};

	DestroyPipeline(shader);

	DestroyDevice();

	DestroyInstance();

	return 0;
};