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

uint32_t family = 0;
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
	VkImage swapchainsImagesReal[FRAMES_IN_FLIGHT_COUNT];
	VkImageView swapchainsImages[FRAMES_IN_FLIGHT_COUNT];
	uint32_t width;
	uint32_t height;
};
std::unordered_map<void*, RenderWindowInfo> renderWindows;
uint32_t imageIndex = 0;

uint32_t counter = 0;
VkSwapchainKHR* swapchains = 0;
uint32_t* imageIndexes = 0;

bool isRecording = false;





void CreateSwapchain(void* window);
void DestroySwapchain(void* window);
void ResizeSwapchain(void* window);


void CreateDevice()
{


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

	//const char* instanceLayers[] = {
	//	"VK_LAYER_KHRONOS_validation"
	//};
	//instanceCreateInfo.enabledLayerCount = sizeof(instanceLayers) / 8;
	//instanceCreateInfo.ppEnabledLayerNames = instanceLayers;
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
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	VkPhysicalDeviceBufferDeviceAddressFeatures pdbdaf{};
	pdbdaf.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	pdbdaf.bufferDeviceAddress = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.enabledExtensionCount = sizeof(deviceExtensions) / 8;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.pNext = &pdbdaf;
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
	isRecording = true;
}

void Render()
{

	isRecording = false;
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

uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	return 0;
}

struct BufferHandle {
	VkBuffer buffer;
	uint32_t size;
	VkDeviceMemory memory;
};

void* CreateBuffer(uint32_t size, void** allocation, BufferType type)
{
	VkBuffer buffer;
	VkMemoryRequirements memRequirements;
	VkDeviceMemory memory;

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;

	VkBufferUsageFlagBits usage;
	switch (type) {
	case Storage: { usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; break; }
	case Uniform: { usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break; }
	case Vertex: { usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; break; }
	case Index: { usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; break; }

	}

	bufferCreateInfo.usage = usage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);

	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateFlagsInfo allocFlagsInfo{};
	allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	allocInfo.pNext = &allocFlagsInfo;
	vkAllocateMemory(device, &allocInfo, nullptr, &memory);


	vkMapMemory(device, memory, 0, size, 0, allocation);
	vkBindBufferMemory(device, buffer, memory, 0);

	return new BufferHandle{ buffer,size,memory };
}

uint32_t GetBufferSize(void* buffer)
{
	return ((BufferHandle*) buffer)->size;
}

uint64_t GetBufferDeviceAddress(void* buffer)
{
	return 0;
}

void DeleteBuffer(void* buffer)
{
	vkUnmapMemory(device, ((BufferHandle*)buffer)->memory);
	vkFreeMemory(device, ((BufferHandle*)buffer)->memory, nullptr);
	vkDestroyBuffer(device, ((BufferHandle*)buffer)->buffer, nullptr);
	delete ((BufferHandle*)buffer);
}


struct ImageHandle {
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory memory;
	uint32_t x;
	uint32_t y;
	VkFormat format;
	VkSampler sampler = nullptr;
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

	// Raster pipeline
	VkRenderPass renderPass = nullptr;
	VkFramebuffer framebuffer = nullptr;
	bool useDepth = false;
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


void* CreateImage(uint32_t x, uint32_t y, unsigned char imageFormat)
{
	VkImage image;
	VkDeviceMemory memory;
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = x > 1 ? x : 1;
	imageInfo.extent.height = y > 1 ? y : 1;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	if (imageFormat == 0) {
		imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	} else
	if (imageFormat == 1) {
		imageInfo.format = VK_FORMAT_D32_SFLOAT;
	} else
	if (imageFormat == 2) {
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	}
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if (imageFormat == 0) {
		imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT;
	}
	if (imageFormat == 2) {
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	}
	
	if (imageInfo.format == VK_FORMAT_D32_SFLOAT) {
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	};
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	//VmaAllocationCreateInfo allocInfo = {};
	//allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	//allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

	//vmaCreateImage(device->allocator, &imageInfo, &allocInfo, &textureImage, &allocation, nullptr);
	vkCreateImage(device, &imageInfo, nullptr, &image);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vkAllocateMemory(device, &allocInfo, nullptr, &memory);
	vkBindImageMemory(device, image, memory, 0);

	ImageHandle* imageHandle = new ImageHandle;
	imageHandle->format = imageInfo.format;
	imageHandle->image = image;
	imageHandle->imageView = MakeImageView(image, imageInfo.format);
	imageHandle->memory = memory;
	imageHandle->x = x;
	imageHandle->y = y;

	return imageHandle;
}

void UpdateImage(void* image, void* buffer)
{
	VkCommandBuffer commandBuffer;
	VkCommandPool commandPool;

	VkCommandPoolCreateInfo cmdPoolCreateInfo{};
	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolCreateInfo.queueFamilyIndex = family;
	vkCreateCommandPool(device, &cmdPoolCreateInfo, nullptr, &commandPool);

	VkCommandBufferAllocateInfo cmdAllocateInfo{};
	cmdAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdAllocateInfo.commandBufferCount = 1;
	cmdAllocateInfo.commandPool = commandPool;
	vkAllocateCommandBuffers(device, &cmdAllocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);


	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		((ImageHandle*)image)->x,
		((ImageHandle*)image)->y,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		((BufferHandle*)buffer)->buffer,
		((ImageHandle*)image)->image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, nullptr);
	vkQueueWaitIdle(graphicsQueue);
}
void DeleteImage(void* image)
{
	// Protection against deleting swapchain images
	if (((ImageHandle*)(image))->x!=UINT32_MAX) {
		vkUnmapMemory(device, ((ImageHandle*)(image))->memory);
		vkFreeMemory(device, ((ImageHandle*)(image))->memory, nullptr);
		vkDestroyImageView(device, ((ImageHandle*)(image))->imageView,nullptr);
		vkDestroyImage(device, ((ImageHandle*)(image))->image, nullptr);
	}
	delete (ImageHandle*)image;
}

void ClearImage(void* image)
{
	VkImageSubresourceRange subresourceRange{};
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	if (((ImageHandle*)image)->format != VK_FORMAT_D32_SFLOAT) {
		VkClearColorValue color = VkClearColorValue{ 0,0,0,0 };
		vkCmdClearColorImage(cmd[imageIndex], ((ImageHandle*)image)->image, VK_IMAGE_LAYOUT_GENERAL, &color, 1, &subresourceRange);
	}
	else {
		VkClearDepthStencilValue color = VkClearDepthStencilValue{ 1,0 };
		vkCmdClearDepthStencilImage(cmd[imageIndex], ((ImageHandle*)image)->image, VK_IMAGE_LAYOUT_GENERAL, &color, 1, &subresourceRange);

	}
}

void* GenerateSampler(void* image)
{
	ImageHandle* imghandle = (ImageHandle*)image;
	ImageHandle imgHandle = *imghandle;


	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	VkSampler* sampler = &imgHandle.sampler;
	vkCreateSampler(device, &samplerInfo, nullptr, sampler);

	ImageHandle* copy = new ImageHandle(imgHandle);
	return copy;
}

void DestroySampler(void* image)
{
	ImageHandle* imghandle = (ImageHandle*)image;
	vkDestroySampler(device, imghandle->sampler,nullptr);
	delete imghandle;
}

void BarrierImage(void* image) {
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE_KHR;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_NONE_KHR;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = ((ImageHandle*)image)->image;  // The VkImage being transitioned
	//aaimageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;
	vkCmdPipelineBarrier(cmd[imageIndex], VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0, 0, 0, 0, 1, &imageMemoryBarrier);

}

void* GetWindowImage(void* window)
{
	ImageHandle* imghandle = new ImageHandle;
	imghandle->image = renderWindows[window].swapchainsImagesReal[imageIndex];
	imghandle->imageView = renderWindows[window].swapchainsImages[imageIndex];
	imghandle->format = VK_FORMAT_R8G8B8A8_UNORM;
	imghandle->x = -1;
	imghandle->y = -1;
	return imghandle;
}

void UsePipeline(void* shader) {
	vkUpdateDescriptorSets(device, ((ShaderHandle*)shader)->wdssCount, ((ShaderHandle*)shader)->wdss, 0, 0);
	vkCmdBindPipeline(cmd[imageIndex], ((ShaderHandle*)shader)->bindPoint, ((ShaderHandle*)shader)->pipeline);
	vkCmdBindDescriptorSets(cmd[imageIndex], ((ShaderHandle*)shader)->bindPoint, ((ShaderHandle*)shader)->pipelineLayout, 0, 1, &((ShaderHandle*)shader)->descriptorSet, 0, nullptr);
};

void SetDescriptor(void* shader, void* value, uint32_t binding)
{
	// Reset to prevent bugs
	if (((ShaderHandle*)shader)->wdss[binding].pBufferInfo) { 
		delete ((ShaderHandle*)shader)->wdss[binding].pBufferInfo; 
		((ShaderHandle*)shader)->wdss[binding].pBufferInfo = nullptr;};
	if (((ShaderHandle*)shader)->wdss[binding].pImageInfo) { 
		delete ((ShaderHandle*)shader)->wdss[binding].pImageInfo; 
		((ShaderHandle*)shader)->wdss[binding].pImageInfo = nullptr;};
	//if (!((ShaderHandle*)shader)->wdss[binding].pNext) { delete ((ShaderHandle*)shader)->wdss[binding].pNext; ((ShaderHandle*)shader)->wdss[binding].pNext = nullptr;};
	
	// Now we set descriptor
	BufferHandle* bValue = (BufferHandle*)value;
	ImageHandle* iValue = (ImageHandle*)value;
	VkDescriptorType dt = ((ShaderHandle*)shader)->wdss[binding].descriptorType;
	switch (dt) {
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: { ((ShaderHandle*)shader)->wdss[binding].pBufferInfo = new VkDescriptorBufferInfo{ bValue->buffer,0,bValue->size}; break; }
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: { ((ShaderHandle*)shader)->wdss[binding].pBufferInfo = new VkDescriptorBufferInfo{ bValue->buffer,0,bValue->size }; break; }
	case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: { ((ShaderHandle*)shader)->wdss[binding].pImageInfo = new VkDescriptorImageInfo{ nullptr, iValue->imageView, VK_IMAGE_LAYOUT_GENERAL}; break; }
	case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: { ((ShaderHandle*)shader)->wdss[binding].pImageInfo = new VkDescriptorImageInfo{ iValue->sampler, iValue->imageView, VK_IMAGE_LAYOUT_GENERAL }; break; }
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
	VkShaderModule vsm = CreateShader(info.vertexSpirv, info.vertexSpirvSize);
	VkPipelineShaderStageCreateInfo vertexShaderStage = BuildShaderStage(vsm, VK_SHADER_STAGE_VERTEX_BIT);
	VkShaderModule fsm = CreateShader(info.fragmentSpirv, info.fragmentSpirvSize);
	VkPipelineShaderStageCreateInfo fragmentShaderStage = BuildShaderStage(fsm, VK_SHADER_STAGE_FRAGMENT_BIT);


	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
	VkRenderPass renderPass;


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
		bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
		bindings[i].pImmutableSamplers = nullptr;
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
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
	if (info.pipelineInfo.constantsSize) {
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
	}

	vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);






















	VkDynamicState states[2] = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = states;


	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkVertexInputBindingDescription vertexBindingDescriptions{};
	vertexBindingDescriptions.stride = 12;

	VkVertexInputAttributeDescription vertexAttributeDescriptions{};
	vertexAttributeDescriptions.format = VK_FORMAT_R32G32B32_SFLOAT;

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescriptions;
	vertexInputInfo.vertexAttributeDescriptionCount = 1;
	vertexInputInfo.pVertexAttributeDescriptions = &vertexAttributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;

	viewport.width = 1280;
	viewport.height = 720;

	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_GENERAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	if (info.useDepth) {
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
	}

	VkAttachmentDescription rattachments[] = {
		colorAttachment,
		depthAttachment
	};

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = info.useDepth?2:1;
	renderPassInfo.pAttachments = rattachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);

	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.layout = pipelineLayout;
	createInfo.stageCount = 2;
	VkPipelineShaderStageCreateInfo stages[2] = { vertexShaderStage, fragmentShaderStage };
	createInfo.pStages = stages;

	createInfo.pVertexInputState = &vertexInputInfo;
	createInfo.pInputAssemblyState = &inputAssembly;
	createInfo.pViewportState = &viewportState;
	createInfo.pRasterizationState = &rasterizer;
	createInfo.pMultisampleState = &multisampling;
	createInfo.pColorBlendState = &colorBlending;
	createInfo.pDynamicState = &dynamicState;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = info.useDepth;
	depthStencil.depthWriteEnable = info.useDepth;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.stencilTestEnable = VK_FALSE;

	createInfo.pDepthStencilState = &depthStencil;
	createInfo.renderPass = renderPass;
	createInfo.subpass = 0;

	vkCreateGraphicsPipelines(device, nullptr, 1, &createInfo, nullptr, &pipeline);

	DestroyShader(vsm);
	DestroyShader(fsm);

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
	pipelineData->bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	pipelineData->shaderStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT| VK_SHADER_STAGE_VERTEX_BIT;
	pipelineData->pushConstantsSize = info.pipelineInfo.constantsSize;
	pipelineData->renderPass = renderPass;
	pipelineData->useDepth = info.useDepth;

	return pipelineData;
}

void Record(void* shader, uint32_t x, uint32_t y, void* image, void* depth)
{
	VkImageView* imageViews = (VkImageView*)malloc(((ShaderHandle*)shader)->useDepth?16:8);
	imageViews[0] = ((ImageHandle*)image)->imageView;
	if (((ShaderHandle*)shader)->useDepth) {
		imageViews[1] = ((ImageHandle*)depth)->imageView;
	}


	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = ((ShaderHandle*)shader)->renderPass;
	framebufferInfo.attachmentCount = ((ShaderHandle*)shader)->useDepth ? 2 : 1;
	framebufferInfo.pAttachments = imageViews;
	framebufferInfo.width = x;
	framebufferInfo.height = y;
	framebufferInfo.layers = 1;	
	if (((ShaderHandle*)shader)->framebuffer) {
		vkDestroyFramebuffer(device, ((ShaderHandle*)shader)->framebuffer, nullptr);
	}
	vkCreateFramebuffer(device, &framebufferInfo, nullptr, &((ShaderHandle*)shader)->framebuffer);

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = ((ShaderHandle*)shader)->renderPass;
	renderPassInfo.framebuffer = ((ShaderHandle*)shader)->framebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = VkExtent2D{ x,y };
	renderPassInfo.clearValueCount = 0;

	vkCmdBeginRenderPass(cmd[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(cmd[imageIndex], ((ShaderHandle*)shader)->bindPoint, ((ShaderHandle*)shader)->pipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = x;
	viewport.height = y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cmd[imageIndex], 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = VkExtent2D{ x,y };
	vkCmdSetScissor(cmd[imageIndex], 0, 1, &scissor);
}

void StopRecord()
{
	vkCmdEndRenderPass(cmd[imageIndex]);
}

void SetIndexBuffer(void* buffer)
{
	vkCmdBindIndexBuffer(cmd[imageIndex], ((BufferHandle*)buffer)->buffer, 0, VK_INDEX_TYPE_UINT32);
}

void SetVertexBuffer(void* buffer)
{
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(cmd[imageIndex], 0, 1, &((BufferHandle*)buffer)->buffer, offsets);
}

void Draw()
{
}

void DrawIndexed(uint32_t triangles, uint32_t instances)
{
	vkCmdDrawIndexed(cmd[imageIndex], triangles, instances, 0, 0, 0);
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

	// Creating pipeline
	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.layout = pipelineLayout;
	computePipelineCreateInfo.stage = shaderStage;
	vkCreateComputePipelines(device, nullptr, 1, &computePipelineCreateInfo, nullptr, &pipeline);

	// Destroying shader module just to make it work
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
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;

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
		rwi.swapchainsImagesReal[i] = images[i];
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
	// Resizing is slightly fucked
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
			ResizeSwapchain(window);
		}
	}
	return !windows.size();
}

