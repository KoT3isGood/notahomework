#include "nhwdraw.h"
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#ifdef _WIN32
#include "Windows.h"
#include "vulkan/vulkan_win32.h"
#endif
#include "unordered_map"

#define VK_FUNCTION_GEN(function) PFN_##function function;
#define VKI_FUNCTION(function) PFN_##function function = (PFN_##function)vkGetInstanceProcAddr(instance,#function)

VkInstance instance;
VkDebugUtilsMessengerEXT messenger;

VkPhysicalDevice physicalDevice;

VkDevice device;
VkQueue graphicsQueue;
VkQueue presentQueue;

#define DESCRIPTOR_COUNT 128
#define FRAMES_IN_FLIGHT_COUNT 2
VkCommandPool cmdPool;
VkCommandBuffer cmd[FRAMES_IN_FLIGHT_COUNT];

VkFence fence[FRAMES_IN_FLIGHT_COUNT];
VkSemaphore graphicsSemaphore[FRAMES_IN_FLIGHT_COUNT];
VkSemaphore presentSemaphore[FRAMES_IN_FLIGHT_COUNT];

VkBool32 debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData
) {
	Log(pCallbackData->pMessage);
	return VK_FALSE;
}


std::unordered_map<void*, WindowInfo> windows;
struct RenderWindowInfo {
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkImageView swapchainsImages[FRAMES_IN_FLIGHT_COUNT];
	uint32_t width;
	uint32_t height;
};
std::unordered_map<void*, RenderWindowInfo> renderWindows;
uint32_t imageIndex = 0;

uint32_t counter = 0;
VkSwapchainKHR* swapchains = 0;
uint32_t* imageIndexes = 0;





void CreateSwapchain(void* window);
void DestroySwapchain(void* window);
void ResizeSwapchain(void* window);


void CreateDevice()
{
	if (!glfwInit()) Mayday("Failed to init GLFW");
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


	// Instance
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	const char* instanceExtensions[] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME,
		#ifdef _WIN32
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME
		#endif
	};
	instanceCreateInfo.enabledExtensionCount = sizeof(instanceExtensions) / 8;
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions;

	const char* instanceLayers[] = {
		"VK_LAYER_KHRONOS_validation"
	};
	instanceCreateInfo.enabledLayerCount = sizeof(instanceLayers) / 8;
	instanceCreateInfo.ppEnabledLayerNames = instanceLayers;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	vkCreateInstance(&instanceCreateInfo, 0, &instance);

	// Debug messenger

	VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo{};
	messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messengerCreateInfo.pfnUserCallback = debugCallback;
	messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

	VKI_FUNCTION(vkCreateDebugUtilsMessengerEXT);
	vkCreateDebugUtilsMessengerEXT(instance, &messengerCreateInfo, 0, &messenger);

	// Physical device selection


	// enumerate physical devices
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, 0);
	Log("physicalDeviceCount = %i", physicalDeviceCount);
	if (physicalDeviceCount == 0) {
		Mayday("Failed to find physical device");
	}
	VkPhysicalDevice* physicalDevices = (VkPhysicalDevice*)malloc(physicalDeviceCount * 8);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);

	// select the best one
	uint32_t rating = 0;

	for (uint32_t i = 0; i < physicalDeviceCount; i++) {
		VkPhysicalDevice pd = physicalDevices[i];
		VkPhysicalDeviceProperties pdp;
		vkGetPhysicalDeviceProperties(pd, &pdp);
		uint32_t deviceRating = 0;
		if (pdp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			deviceRating += 256;
		}
		VkPhysicalDeviceMemoryProperties pdmp;
		vkGetPhysicalDeviceMemoryProperties(pd, &pdmp);
		deviceRating += pdmp.memoryHeaps[0].size / (1024 * 1024 * 256);
		if (rating < deviceRating) {
			rating = deviceRating;
			physicalDevice = pd;
		};
	}
	free(physicalDevices);



	// Device and queues
	uint32_t deviceQueuePropertiesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &deviceQueuePropertiesCount, 0);
	VkQueueFamilyProperties* queueFamilyProperties = (VkQueueFamilyProperties*)malloc(deviceQueuePropertiesCount * 24);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &deviceQueuePropertiesCount, queueFamilyProperties);

	uint32_t family = 0;
	for (int i = 0; i < deviceQueuePropertiesCount; i++) {
		if (queueFamilyProperties[i].queueFlags == VK_QUEUE_GRAPHICS_BIT) {
			family = i;
			break;
		}
	}

	float priority = 1.0;
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = family;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &priority;

	const char* deviceExtensions[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.enabledExtensionCount = sizeof(deviceExtensions) / 8;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
	vkGetDeviceQueue(device, family, 0, &graphicsQueue);
	vkGetDeviceQueue(device, family, 0, &presentQueue);

	// Command buffers
	VkCommandPoolCreateInfo cmdPoolCreateInfo{};
	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolCreateInfo.queueFamilyIndex = family;
	vkCreateCommandPool(device, &cmdPoolCreateInfo, nullptr, &cmdPool);

	VkCommandBufferAllocateInfo cmdAllocateInfo{};
	cmdAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdAllocateInfo.commandBufferCount = FRAMES_IN_FLIGHT_COUNT;
	cmdAllocateInfo.commandPool = cmdPool;
	vkAllocateCommandBuffers(device, &cmdAllocateInfo, cmd);

	// Sync
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (uint32_t i = 0; i < FRAMES_IN_FLIGHT_COUNT;i++) {
		vkCreateFence(device, &fenceCreateInfo, nullptr, &fence[i]);
		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &graphicsSemaphore[i]);
		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &presentSemaphore[i]);
	}

	Log("Created device");
	


}

void BeginRendering()
{
	vkWaitForFences(device, 1, &fence[imageIndex], VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &fence[imageIndex]);

	swapchains = (VkSwapchainKHR*)malloc(renderWindows.size() * 8);
	imageIndexes = (uint32_t*)malloc(renderWindows.size() * 4);



	for (auto swapchain : renderWindows) {
		if (swapchain.second.swapchain) {
			
			

		
			vkAcquireNextImageKHR(device, swapchain.second.swapchain, UINT64_MAX, presentSemaphore[imageIndex], fence[imageIndex], &imageIndexes[counter]);
			swapchains[counter] = swapchain.second.swapchain;
			counter++;

		}
	}

	vkResetCommandBuffer(cmd[imageIndex], 0);
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(cmd[imageIndex], &beginInfo);
}

void Render()
{

	vkEndCommandBuffer(cmd[imageIndex]);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { presentSemaphore[imageIndex] };
	VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = &waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd[imageIndex];

	VkSemaphore signalSemaphores[] = { graphicsSemaphore[imageIndex] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence[imageIndex]);
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = counter;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = imageIndexes;
	vkQueuePresentKHR(presentQueue, &presentInfo);

	imageIndex = (imageIndex + 1) % FRAMES_IN_FLIGHT_COUNT;

	free(swapchains);
	free(imageIndexes);
	counter = 0;
}

void DestroyDevice()
{
	for (uint32_t i = 0; i < FRAMES_IN_FLIGHT_COUNT; i++) {
		vkDestroySemaphore(device, presentSemaphore[i], nullptr);
		vkDestroySemaphore(device, graphicsSemaphore[i], nullptr);
		vkDestroyFence(device, fence[i], nullptr);
	}
	vkDestroyCommandPool(device, cmdPool,nullptr);
	vkDestroyDevice(device, nullptr);

	VKI_FUNCTION(vkDestroyDebugUtilsMessengerEXT);
	vkDestroyDebugUtilsMessengerEXT(instance, messenger, 0);
	vkDestroyInstance(instance, 0);

	glfwTerminate();
}

void* CreateBuffer(uint32_t size, void* allocation)
{
	return 0;
}

uint32_t GetBufferSize(void* buffer)
{
	return 0;
}

uint64_t GetBufferDeviceAddress(void* buffer)
{
	return 0;
}

void DeleteBuffer(void* buffer)
{
}

struct BufferHandle {
	VkBuffer buffer;
	uint32_t size;
};

struct ImageHandle {
	VkImage image;
	VkImageView imageView;
	uint32_t x;
	uint32_t y;
};

struct ShaderHandle {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;

	uint32_t wdssCount;
	VkWriteDescriptorSet* wdss;

	uint32_t pushConstantsSize;

	VkShaderStageFlags shaderStageFlags;
	VkPipelineBindPoint bindPoint;
};

VkImageView MakeImageView(const VkImage& image, VkFormat format) {

	VkImageView imageView = nullptr;
	VkImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	if (format == VK_FORMAT_D32_SFLOAT) {
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView);
	return imageView;
}


void* CreateImage(uint32_t x, uint32_t y, void* buffer)
{
	return 0;
}

void UpdateImage(void* image, void* buffer)
{
}
void DeleteImage(void* image)
{
	// Protection against deleting swapchain images
	if (((ImageHandle*)(image))->image != nullptr) {

	}
	delete (ImageHandle*)image;
}

void* GetWindowImage(void* window)
{
	ImageHandle* imghandle = new ImageHandle;
	imghandle->image = nullptr;
	imghandle->imageView = renderWindows[window].swapchainsImages[imageIndex];
	return imghandle;
}

void UsePipeline(void* shader) {
	vkUpdateDescriptorSets(device, ((ShaderHandle*)shader)->wdssCount, ((ShaderHandle*)shader)->wdss, 0, 0);
	vkCmdBindPipeline(cmd[imageIndex], ((ShaderHandle*)shader)->bindPoint, ((ShaderHandle*)shader)->pipeline);
	vkCmdBindDescriptorSets(cmd[imageIndex], ((ShaderHandle*)shader)->bindPoint, ((ShaderHandle*)shader)->pipelineLayout, 0, 1, &((ShaderHandle*)shader)->descriptorSet, 0, nullptr);
};

void SetDescriptor(void* shader, void* value, uint32_t binding)
{
	if (!((ShaderHandle*)shader)->wdss[binding].pBufferInfo) { delete ((ShaderHandle*)shader)->wdss[binding].pBufferInfo; ((ShaderHandle*)shader)->wdss[binding].pBufferInfo = nullptr;};
	if (!((ShaderHandle*)shader)->wdss[binding].pImageInfo) { delete ((ShaderHandle*)shader)->wdss[binding].pImageInfo; ((ShaderHandle*)shader)->wdss[binding].pImageInfo = nullptr;};
	if (!((ShaderHandle*)shader)->wdss[binding].pNext) { delete ((ShaderHandle*)shader)->wdss[binding].pNext; ((ShaderHandle*)shader)->wdss[binding].pNext = nullptr;};
	
	BufferHandle* bValue = (BufferHandle*)value;
	ImageHandle* iValue = (ImageHandle*)value;
	VkDescriptorType dt = ((ShaderHandle*)shader)->wdss[binding].descriptorType;
	switch (dt) {
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: { ((ShaderHandle*)shader)->wdss[binding].pBufferInfo = new VkDescriptorBufferInfo{ bValue->buffer,0,bValue->size}; break; }
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: { ((ShaderHandle*)shader)->wdss[binding].pBufferInfo = new VkDescriptorBufferInfo{ bValue->buffer,0,bValue->size }; break; }
	case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: { ((ShaderHandle*)shader)->wdss[binding].pImageInfo = new VkDescriptorImageInfo{ nullptr, iValue->imageView, VK_IMAGE_LAYOUT_GENERAL}; break; }
	case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: { ((ShaderHandle*)shader)->wdss[binding].pImageInfo = new VkDescriptorImageInfo{ nullptr, iValue->imageView, VK_IMAGE_LAYOUT_GENERAL }; break; }
	}
}

void SetConstants(void* shader, void* constants)
{
	vkCmdPushConstants(cmd[imageIndex], ((ShaderHandle*)shader)->pipelineLayout, ((ShaderHandle*)shader)->shaderStageFlags, 0, ((ShaderHandle*)shader)->pushConstantsSize, constants);
}

void DestroyPipeline(void* pipeline)
{
}



VkShaderModule CreateShader(unsigned char* spirv, uint32_t spirvSize) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = spirvSize;
	createInfo.pCode = (uint32_t*)spirv;
	
	VkShaderModule shaderModule;
	vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);

	return shaderModule;
};

void DestroyShader(VkShaderModule shaderModule) {
	vkDestroyShaderModule(device, shaderModule, nullptr);
}

VkPipelineShaderStageCreateInfo BuildShaderStage(VkShaderModule shaderModule, VkShaderStageFlagBits shaderType) {
	VkPipelineShaderStageCreateInfo shaderStageInfo{};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = shaderType;
	shaderStageInfo.module = shaderModule;
	shaderStageInfo.pName = "main";
	return shaderStageInfo;
}










void* CreateRasterizationPipeline(RasterizationPipelineInfo info)
{
	return 0;
}

void SetIndexBuffer()
{
}

void SetVertexBuffer()
{
}

void Draw()
{
}

void DrawIndexed()
{
}


void* CreateComputePipeline(ComputePipelineInfo info)
{
	// Create shader module
	VkShaderModule csm = CreateShader(info.computeSpirv, info.computeSpirvSize);
	VkPipelineShaderStageCreateInfo shaderStage = BuildShaderStage(csm,VK_SHADER_STAGE_COMPUTE_BIT);






	
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;


	// Descriptor Layout 
	VkDescriptorSetLayoutBinding* bindings = (VkDescriptorSetLayoutBinding*)malloc(sizeof(VkDescriptorSetLayoutBinding) * info.pipelineInfo.descriptorsCount);
	for (uint32_t i = 0; i < info.pipelineInfo.descriptorsCount; i++) {
		bindings[i].descriptorCount = 1;
		switch (info.pipelineInfo.descriptorTypes[i]) {
		case DescriptorType::UniformBuffer:			bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;				break;
		case DescriptorType::StorageBuffer:			bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;				break;
		case DescriptorType::Image:					bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;				break;
		case DescriptorType::SampledImage:			bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;		break;
		case DescriptorType::AccelerationStrucutre: bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR; break;
		}
		bindings[i].binding = i;
		bindings->stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = info.pipelineInfo.descriptorsCount;
	descriptorSetLayoutCreateInfo.pBindings = bindings;

	vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutCreateInfo.setLayoutCount = 1;

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.size = info.pipelineInfo.constantsSize;
	pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	if (info.pipelineInfo.constantsSize) {
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
	}

	vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.layout = pipelineLayout;
	computePipelineCreateInfo.stage = shaderStage;
	// Creating pipeline
	vkCreateComputePipelines(device, nullptr, 1, &computePipelineCreateInfo, nullptr, &pipeline);

	DestroyShader(csm);

	// Descriptor set

	

	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	poolSize.descriptorCount = DESCRIPTOR_COUNT;

	VkDescriptorPoolSize poolSize2 = {};
	poolSize2.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize2.descriptorCount = DESCRIPTOR_COUNT;

	VkDescriptorPoolSize poolSize3 = {};
	poolSize3.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	poolSize3.descriptorCount = DESCRIPTOR_COUNT;

	VkDescriptorPoolSize poolSize4 = {};
	poolSize4.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSize4.descriptorCount = DESCRIPTOR_COUNT;

	VkDescriptorPoolSize poolSize5 = {};
	poolSize5.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize5.descriptorCount = DESCRIPTOR_COUNT;

	VkDescriptorPoolSize poolSizes[] = { poolSize,poolSize2,poolSize3,poolSize4,poolSize5 };

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.poolSizeCount = 5;
	poolCreateInfo.pPoolSizes = poolSizes;
	poolCreateInfo.maxSets = 1;

	VkDescriptorPool descPool;
	VkDescriptorSet descSet;
	vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &descPool);

	VkDescriptorSetAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool = descPool;
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.pSetLayouts = &descriptorSetLayout;


	vkAllocateDescriptorSets(device, &allocateInfo, &descSet);

	// Setting up all descriptor sets

	VkWriteDescriptorSet* wdss = (VkWriteDescriptorSet*)malloc(sizeof(VkWriteDescriptorSet) * info.pipelineInfo.descriptorsCount);
	for (uint32_t i = 0; i < info.pipelineInfo.descriptorsCount; i++) {
		wdss[i] = VkWriteDescriptorSet{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descSet,
			.dstBinding = i,
			.descriptorType = bindings[i].descriptorType,
			.descriptorCount = 1,
		};	
	};

	// Generating shader handle

	ShaderHandle* pipelineData = new ShaderHandle;
	pipelineData->pipeline = pipeline;
	pipelineData->pipelineLayout = pipelineLayout;
	pipelineData->descriptorSetLayout = descriptorSetLayout;
	pipelineData->descriptorSet = descSet;
	pipelineData->wdssCount = info.pipelineInfo.descriptorsCount;
	pipelineData->wdss = wdss;
	pipelineData->bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
	pipelineData->shaderStageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	pipelineData->pushConstantsSize = info.pipelineInfo.constantsSize;
	
	return pipelineData;
}

void Dispatch(uint32_t x, uint32_t y, uint32_t z)
{
	vkCmdDispatch(cmd[imageIndex], x, y, z);
}

void* CreateRayTracingPipeline(RayTracingPipelineInfo info)
{
	return 0;
}

void TraceRays(uint32_t x, uint32_t y)
{
}

void CreateSwapchain(void* window) {
	VkSurfaceKHR surface = nullptr;
	VkResult result = glfwCreateWindowSurface(instance, (GLFWwindow*)window, nullptr, &surface);

	RenderWindowInfo rwi{};
	rwi.surface = surface;


	VkSurfaceCapabilitiesKHR capabilies{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilies);

	VkSwapchainKHR swapchain = nullptr;
	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = FRAMES_IN_FLIGHT_COUNT;
#ifdef LINUX_PLATFORM
	swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
#else 
	swapchainCreateInfo.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
#endif
	swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapchainCreateInfo.imageExtent = capabilies.minImageExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
	rwi.swapchain = swapchain;

	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
	VkImage images[FRAMES_IN_FLIGHT_COUNT];
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images);
	for (uint32_t i = 0; i < FRAMES_IN_FLIGHT_COUNT; i++) {
		rwi.swapchainsImages[i] = MakeImageView(images[i], VK_FORMAT_R8G8B8A8_UNORM);
	}

	
	WindowInfo wi = GetWindowInfo(window);
	rwi.width = wi.width;
	rwi.height = wi.height;
	renderWindows[window] = rwi;
}

void DestroySwapchain(void* window) {
	for (uint32_t i = 0; i < FRAMES_IN_FLIGHT_COUNT; i++) {
		vkDestroyImageView(device,renderWindows[window].swapchainsImages[i],nullptr);
	}
	vkDestroySwapchainKHR(device, renderWindows[window].swapchain, nullptr);
	vkDestroySurfaceKHR(instance, renderWindows[window].surface, nullptr);
	renderWindows.erase(window);
}

void ResizeSwapchain(void* window)
{
	for (uint32_t i = 0; i < FRAMES_IN_FLIGHT_COUNT; i++) {
		vkDestroyImageView(device, renderWindows[window].swapchainsImages[i], nullptr);
	}
	vkDestroySwapchainKHR(device, renderWindows[window].swapchain, nullptr);
	vkDestroySurfaceKHR(instance, renderWindows[window].surface, nullptr);
	CreateSwapchain(window);
}

#undef CreateWindow
void* CreateWindow(WindowInfo windowInfo)
{
	GLFWwindow* window = glfwCreateWindow(windowInfo.width,windowInfo.height,windowInfo.title, nullptr,nullptr);
	windows[window] = windowInfo;
	
	CreateSwapchain(window);

	return window;
}


WindowInfo GetWindowInfo(void* window)
{
	return windows[window];
}

void SetWindowInfo(void* window, WindowInfo windowInfo)
{
	windows[window] = windowInfo;
}

void DestroyWindow(void* window)
{
	DestroySwapchain(window);
	windows.erase(window);
	glfwDestroyWindow((GLFWwindow*)window);
}

bool ShouldClose() {

	glfwPollEvents();
	vkDeviceWaitIdle(device);
	for (auto& window : windows) {
		glfwGetWindowSize((GLFWwindow*)window.first, (int*)&window.second.width, (int*)&window.second.height);
		glfwGetWindowPos((GLFWwindow*)window.first, (int*)&window.second.x, (int*)&window.second.y);
		if (glfwWindowShouldClose((GLFWwindow*)window.first)) {
			DestroyWindow(window.first);
			glfwWaitEvents();
		};
	}
	for (auto swapchain : renderWindows) {
		WindowInfo wi = windows[swapchain.first];
		void* window = swapchain.first;
		if (!window) continue;
		if (swapchain.second.width != wi.width || swapchain.second.height != wi.height) {
			Log("Resizing");
			ResizeSwapchain(window);
		}
	}
	return !windows.size();
}

