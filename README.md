
**not a homework is an easy library to develop probably most of the stuff**

it is highly inspired by raylib and tries to fix all of it's disadvantages


Features
--------

 - Written with std:c++17 using Pascal and Camel notation
 - Hardware accelerated vulkan backend with very simple abstraction *that will bite you whenever you make a mistake*
 - Allows to create custom shader pipelines
 - Opensource


Basic example
--------------

```cpp
#include "nhw.h"
#include "draw/nhwdraw.h"
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

  void* image = GetWindowImage(window);
  while (!ShouldClose()) {
    DeleteImage(image);
		image = GetWindowImage(window);

    winInfo = GetWindowInfo(window);
    uint32_t resolution[2] = { winInfo.width,winInfo.height };
    

    SetDescriptor(shader, image, 0);

    BeginRendering();
    UsePipeline(shader);
		SetConstants(shader, resolution);
		Dispatch(resolution[0], resolution[1], 1);
    Render();
  }
	DestroyPipeline(shader);
	DestroyDevice();
	DestroyInstance();
  return 0;
}

```

Build and installation
----------------------

Make sure you have installed `zig 0.12.0-dev.3180+83e578a18`. 

Then you can use `zig build` to compile the library. After that you can link it to any application.

*For linux make sure you have installed Vulkan SDK and OpenAL.*
