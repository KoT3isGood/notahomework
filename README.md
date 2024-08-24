
**not a homework is an easy library to develop probably most of the stuff**

it is highly inspired by raylib

![image](https://github.com/user-attachments/assets/a9efaeb5-c5b2-48ac-ba12-c9a987225a86)



Features
--------
 - Supports C, C++
 - Written with std:c++17 and zig using Pascal and Camel notation
 - Hardware accelerated vulkan backend with very simple abstraction *that will bite you whenever you make a mistake*
 - Allows to create custom shader pipelines
 - Open source

Dependencies
------------
Make sure to link them before you start building your stuff, can be found in thirdparty/
- GLFW 3.4
- Vulkan 1.3
- OpenAL
- ws2_32 (windows only, networking stuff)

- Modern GPU, atleast RTX 3000 and RX 6000 series (due to barycentric extension and ray tracing)

Basic example
--------------

```cpp
#include "draw/nhwdraw.h"
#include "stdio.h"
#include "stdarg.h"
#include <string.h>
#include <math.h>

void logMessage(const char* msg, va_list args) {
    vprintf(msg, args);printf("\n");
};


int main() {

	// Inits instance and device
	nhwInstanceInfo InstanceInfo;
	InstanceInfo.logCallback = logMessage;

	CreateInstance(InstanceInfo);
	CreateDevice();

	// Creates Window

	WindowInfo winInfo;
	winInfo.title = "my mind";
	winInfo.width = 1280;
	winInfo.height = 720;
	void* window = nhwCreateWindow(winInfo);

	// Creates pipeline

	RasterizationPipelineInfo rpi;

	rpi.pipelineInfo.constantsSize = 8;
	rpi.pipelineInfo.descriptorsCount = 3;

	DescriptorType descriptors[3] =
	{ UniformBuffer, StorageBuffer,StorageBuffer };
	rpi.pipelineInfo.descriptorTypes = descriptors;

	unsigned char* fragmentShader = LoadFileDataSized("shader.frag.spv", &rpi.fragmentSpirvSize);
	unsigned char* vertexShader = LoadFileDataSized("shader.vert.spv", &rpi.vertexSpirvSize);
	rpi.fragmentSpirv = fragmentShader;
	rpi.vertexSpirv = vertexShader;
	rpi.useDepth = true;
	void* triShader = CreateRasterizationPipeline(rpi);


	// Creates buffer for vertices and indicies
    	float triangleVertices[] = {
	-1.000000, -1.000000, -1.000000,
    	-1.000000, -1.000000,  1.000000,
    	-1.000000,  1.000000, -1.000000,
    	-1.000000,  1.000000,  1.000000,
    	 1.000000, -1.000000, -1.000000,
    	 1.000000, -1.000000,  1.000000,
    	 1.000000,  1.000000, -1.000000,
	 1.000000,  1.000000,  1.000000,

	};

	void* triangleVerticesPtr = 0;

	void* vertexBuffer = CreateBuffer(sizeof(triangleVertices),&triangleVerticesPtr, Vertex);

	memcpy(triangleVerticesPtr, triangleVertices, sizeof(triangleVertices));

	uint32_t triangleIndexes[] = {
	1,2,0,
        3,6,2,
        7,4,6,
        5,0,4,
        6,0,2,
        3,5,7,
        1,3,2,
        3,7,6,
        7,5,4,
        5,1,0,
        6,4,0,
        3,1,5
	};
	void* triangleIndexesPtr = 0;
	void* indexBuffer = CreateBuffer(sizeof(triangleIndexes), &triangleIndexesPtr, Index);
	memcpy(triangleIndexesPtr, triangleIndexes, sizeof(triangleIndexes));


	// View and projection matrices

    	float transformMatrix[16] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,-4,1,
    	};


    	float s = 1/tan(90/2*3.1415926/180);
    	float f = 1000.0;
    	float n = 0.001;

    	float projectionMatrix[16] = {
        s,0,0,0,
        0,-s,0,0,
        0,0,(f+n)/(n-f),-1,
        0,0,2*(f*n)/(n-f),0,
    	};
    
	void* matrixbufptr = 0;
	void* matricesBuffer = CreateBuffer(192, &matrixbufptr, Uniform);
    	memcpy(matrixbufptr,transformMatrix, sizeof(transformMatrix));
    	memcpy((void*)((uint64_t)matrixbufptr+sizeof(transformMatrix)),projectionMatrix,sizeof(projectionMatrix));

	void* depth = CreateImage(1280,720,1);
    	uint32_t prevresolution[2] = { winInfo.width,winInfo.height };



	// Timer to rotate cube
	float timer = 0.0;


	while (!ShouldClose()) {

		// Resize images and update variables
		void* image = GetWindowImage(window);


		timer = GetTime();

	    	winInfo = nhwGetWindowInfo(window);
		uint32_t resolution[2] = { winInfo.width,winInfo.height };

		if (resolution[0]!=prevresolution[0] || resolution[1]!=prevresolution[1] ) {
			DeleteImage(depth);
			depth = CreateImage(resolution[0],resolution[1],1);
		}

		prevresolution[0] =resolution[0];
		prevresolution[1] =resolution[1];

		float aspect = (float)resolution[1]/ (float)resolution[0];
		projectionMatrix[0]=s*aspect;
    		memcpy((void*)((uint64_t)matrixbufptr+sizeof(transformMatrix)),projectionMatrix,sizeof(projectionMatrix));

		float objectMatrix[16] = {
    		sin(timer),0,cos(timer),0,
        	0,1,0,0,
        	-cos(timer),0,sin(timer),0,
        	0,0,0,1,
    		};
	
    		memcpy((void*)((uint64_t)matrixbufptr+sizeof(transformMatrix)*2),objectMatrix,sizeof(objectMatrix));

		// Set descriptor sets

		SetDescriptor(triShader, matricesBuffer, 0);
		SetDescriptor(triShader, indexBuffer, 1);
		SetDescriptor(triShader, vertexBuffer, 2);

		
	    	BeginRendering();
			// Clears images so stuff can be overdrawn
			ClearImage(image);
			ClearImage(depth);

			

			UsePipeline(triShader);

			Record(triShader, resolution[0], resolution[1], image, depth);
				SetVertexBuffer(vertexBuffer);
				SetIndexBuffer(indexBuffer);
				DrawIndexed(36, 1);
			
			// Sync
			BarrierImage(image);
			BarrierImage(depth);

			StopRecord();
	     	Render();

		 // Dealloc window image

		 DeleteImage(image);
	}


	DestroyPipeline(triShader);
	DestroyDevice();
	DestroyInstance();

  	return 0;
}

```

Build and installation
----------------------

Make sure you have installed `zig 0.13.0`. 

Then you can use `zig build` to compile the library. After that you can link it to any application.

*For linux make sure you have installed Vulkan SDK and OpenAL.*

Known bugs:

- Windows don't close properly
- Vulkan sync can get unstable
- There is no validation and tips
