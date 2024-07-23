#include "nhw.h"
#include "draw/nhwdraw.h"
#include <stdio.h>
#include "stdarg.h"

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
	
	WindowInfo winInfo{};
	winInfo.title = "Real time stategy example";
	winInfo.width = 640;
	winInfo.height = 480;
	void* window = CreateWindow(winInfo);



	while (!ShouldClose()) {
		SetDescriptor(shader, GetWindowImage(window), 0);

		BeginRendering();

		winInfo = GetWindowInfo(window);

		uint32_t resolution[2] = { winInfo.width,winInfo .height};
		UsePipeline(shader);
		SetConstants(shader, resolution);
		Dispatch(resolution[0], resolution[1], 1);


		
		Render();
	};

	DestroyPipeline(shader);

	DestroyDevice();

	DestroyInstance();

	return 0;
}