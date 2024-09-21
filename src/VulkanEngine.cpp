#include <chrono>
#include <thread>

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"

#include "../include/VulkanEngine.hpp"

const std::string APPLICATION_NAME = "APPLICATION NAME";
const std::string ENGINE_NAME = "VULKAN ENGINE";
const vk::Format VULKAN_FORMAT = vk::Format::eB8G8R8A8Unorm; 

static VulkanEngine* loadedEngine = nullptr;

struct Vertex {
	float position[2];
	float color[3];

	static vk::VertexInputBindingDescription getBindingDescription() {
		vk::VertexInputBindingDescription bindingDescription({});
		bindingDescription.setStride(sizeof(Vertex));
		bindingDescription.setInputRate(vk::VertexInputRate::eVertex);

		return bindingDescription;
	};

	static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions({});
		attributeDescriptions[0].setLocation(0);
		attributeDescriptions[0].setFormat(vk::Format::eR32G32Sfloat);
		attributeDescriptions[0].setOffset(offsetof(Vertex, position));

		attributeDescriptions[1].setLocation(1);
		attributeDescriptions[1].setFormat(vk::Format::eR32G32B32Sfloat);
		attributeDescriptions[1].setOffset(offsetof(Vertex, color));

		return attributeDescriptions;
	}
};

const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
};



VulkanEngine::VulkanEngine()
{
	const uint32_t windowHeight = 800, windowWidth = 1200;
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
	_windowExtent = vk::Extent2D(windowWidth, windowHeight);


	//Only 1 engine allowed
	assert(loadedEngine == nullptr);
	loadedEngine = this;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		throw std::runtime_error("SDL_CreateWindow Error: " + std::string(SDL_GetError()));
	}

	// Create an SDL window with Vulkan support
	//TODO: resize support
	SDL_Window* window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, (SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE));
	if (!window) {
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		throw std::runtime_error("SDL_CreateWindow Error: " + std::string(SDL_GetError()));
	}

	vk::ApplicationInfo applicationInfo(APPLICATION_NAME.c_str(), 1, ENGINE_NAME.c_str(), 1, VK_API_VERSION_1_3);

	unsigned int sdlExtensionCount;
	SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, nullptr);
	std::vector<const char*> sdlExtensions(sdlExtensionCount);
	SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, sdlExtensions.data());

	vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo, 0, nullptr, sdlExtensionCount, sdlExtensions.data());
	auto instanceLayerProperties = vk::enumerateInstanceLayerProperties();
	for (vk::LayerProperties ilp : instanceLayerProperties) {
		std::cout << ilp.layerName << std::endl;
	}
	std::cout << std::endl;

	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();

	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
	instanceCreateInfo.pNext = &debugCreateInfo;


	_instance = vk::createInstance(instanceCreateInfo);

	VkSurfaceKHR cSurface;
	if (!SDL_Vulkan_CreateSurface(window, _instance, &cSurface)) {
		vkDestroyInstance(_instance, nullptr);
		SDL_DestroyWindow(window);
		SDL_Quit();
		throw std::exception("Failed to create Vulkan surface");
	}
	_surface = vk::SurfaceKHR(cSurface);

	initDevice();
	initSwapchain();
	initImageViews();
	initRenderPass();
	initFramebuffers();
	initCommandPool();
	initVertexBuffer();
	initIndexBuffer();
	initCommandBuffers();
	initGraphicsPipeline();
	initSemaphores();
}

void VulkanEngine::initDevice() {
	_physicalDevice = selectPhysicalDevice(_instance);

	//creating graphics queue
	uint32_t queueFamilyIndex = 0;
	auto queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();
	for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i) {
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
			queueFamilyIndex = i;
			break;
		}
	}

	float queuePriority = 1.0f;
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo({}, queueFamilyIndex, 1, &queuePriority);

	// Enable the extension
	std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	vk::DeviceCreateInfo deviceCreateInfo({}, 1, &deviceQueueCreateInfo);
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	_device = _physicalDevice.createDevice(deviceCreateInfo);
	_graphicsQueue = _device.getQueue(queueFamilyIndex, 0);
}

void VulkanEngine::initSwapchain() {
	//Swapchain setup
	vk::SwapchainCreateInfoKHR swapchainCreateInfo({});
	swapchainCreateInfo.surface = _surface; // The surface to present images to
	swapchainCreateInfo.minImageCount = 2; // Minimum number of images in the swapchain
	swapchainCreateInfo.imageFormat = VULKAN_FORMAT; // Image format
	swapchainCreateInfo.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear; // Color space
	swapchainCreateInfo.imageExtent = _windowExtent; // Image size
	swapchainCreateInfo.imageArrayLayers = 1; // Number of layers in each image
	swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment; // Image usage
	swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive; // Sharing mode
	swapchainCreateInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity; // Pre-transform
	swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque; // Composite alpha
	swapchainCreateInfo.presentMode = vk::PresentModeKHR::eFifoRelaxed; // Present mode
	swapchainCreateInfo.clipped = VK_TRUE; // Clipping
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE; // Old swapchain

	_swapchain = _device.createSwapchainKHR(swapchainCreateInfo);
}

void VulkanEngine::initImageViews() {
	//ImageViews setup
	std::vector<vk::Image> images = _device.getSwapchainImagesKHR(_swapchain);
	for (int i = 0; i < images.size(); i++) {
		vk::ImageViewCreateInfo imageViewCreateInfo({});
		imageViewCreateInfo.image = images[i];
		imageViewCreateInfo.format = VULKAN_FORMAT;
		imageViewCreateInfo.viewType = vk::ImageViewType::e2D;

		vk::ImageSubresourceRange imageSubresourceRange({});
		imageSubresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageSubresourceRange.levelCount = 1;
		imageSubresourceRange.layerCount = 1;

		imageViewCreateInfo.subresourceRange = imageSubresourceRange;
		_imageViews.push_back(_device.createImageView(imageViewCreateInfo));
	}
}

void VulkanEngine::initRenderPass() {
	//RenderPass Setup
	vk::RenderPassCreateInfo2 renderPassCreateInfo2({});
	vk::SubpassDescription2 subpassDescription({});

	vk::AttachmentReference2 attachmentReference({});
	attachmentReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	subpassDescription.setColorAttachments(attachmentReference);

	std::vector<vk::SubpassDescription2> subpassDescriptions = { subpassDescription };
	subpassDescription.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpassDescription.setColorAttachments(attachmentReference);
	renderPassCreateInfo2.setSubpasses(subpassDescriptions);
	renderPassCreateInfo2.subpassCount = 1;

	vk::AttachmentDescription2 colorAttachment({});
	colorAttachment.setFormat(VULKAN_FORMAT);
	colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
	colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
	colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	colorAttachment.setStoreOp(vk::AttachmentStoreOp::eDontCare);
	colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
	colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	std::vector<vk::AttachmentDescription2> attachmentDescriptions = { colorAttachment };
	renderPassCreateInfo2.setAttachments(attachmentDescriptions);

	_renderPass = _device.createRenderPass2(renderPassCreateInfo2);
}

void VulkanEngine::initFramebuffers() {
	//Framebuffer setup
	for (int i = 0; i < _imageViews.size(); i++) {
		vk::ImageView attachments[] = {
			_imageViews[i]
		};

		vk::FramebufferCreateInfo framebufferCreateInfo({});
		framebufferCreateInfo.renderPass = _renderPass;
		framebufferCreateInfo.width = _windowExtent.width;
		framebufferCreateInfo.height = _windowExtent.height;
		framebufferCreateInfo.layers = 1;
		framebufferCreateInfo.setAttachments(attachments);

		vk::Framebuffer framebuffer = _device.createFramebuffer(framebufferCreateInfo);
		_frameBuffers.push_back(framebuffer);
	}
}

void VulkanEngine::initCommandPool() {
	//CommandPool setup
	vk::CommandPoolCreateInfo commandPoolCreateInfo({});
	commandPoolCreateInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
	_commandPool = _device.createCommandPool(commandPoolCreateInfo);
}

void VulkanEngine::initVertexBuffer() {
	uint32_t vertexBufferSize = sizeof(Vertex) * static_cast<uint32_t>(vertices.size());

	vk::DeviceMemory stagingBufferDeviceMemory = vk::DeviceMemory();
	vk::Buffer stagingBuffer = createBuffer(&_device, &stagingBufferDeviceMemory, &_physicalDevice, vk::BufferUsageFlagBits::eTransferSrc , (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) , vertexBufferSize);

	void* data =_device.mapMemory(stagingBufferDeviceMemory, 0, vertexBufferSize, {});
	memcpy(data, vertices.data(), (size_t) vertexBufferSize);
	_device.unmapMemory(stagingBufferDeviceMemory);
	
	_vertexBuffer = createBuffer(&_device, &_deviceMemory, &_physicalDevice, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBufferSize);
	copyBuffer(_device, stagingBuffer, _vertexBuffer, vertexBufferSize, _commandPool, _graphicsQueue);

	_device.destroyBuffer(stagingBuffer);
	_device.freeMemory(stagingBufferDeviceMemory);
}

void VulkanEngine::initIndexBuffer() {
	uint32_t indexBufferSize = sizeof(uint16_t) * static_cast<uint32_t>(indices.size());

	vk::DeviceMemory stagingBufferDeviceMemory = vk::DeviceMemory();
	vk::Buffer stagingBuffer = createBuffer(&_device, &stagingBufferDeviceMemory, &_physicalDevice, vk::BufferUsageFlagBits::eTransferSrc, (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent), indexBufferSize);

	void* data = _device.mapMemory(stagingBufferDeviceMemory, 0, indexBufferSize, {});
	memcpy(data, indices.data(), (size_t) indexBufferSize);
	_device.unmapMemory(stagingBufferDeviceMemory);

	_indexBuffer = createBuffer(&_device, &_deviceMemory, &_physicalDevice, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBufferSize);
	copyBuffer(_device, stagingBuffer, _indexBuffer, indexBufferSize, _commandPool, _graphicsQueue);

	_device.destroyBuffer(stagingBuffer);
	_device.freeMemory(stagingBufferDeviceMemory);	
}

void VulkanEngine::initCommandBuffers() {
	//CommandBuffer setup
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo({});
	commandBufferAllocateInfo.commandPool = _commandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;
	_commandBuffers = _device.allocateCommandBuffers(commandBufferAllocateInfo);
}

void VulkanEngine::initGraphicsPipeline() {
	//Graphics Pipeline
	vk::ShaderModuleCreateInfo vertexShaderCreateInfo({});
	std::vector<uint32_t> vertexShaderCode = readShader("shaders/v_shader.spv");
	vertexShaderCreateInfo.setCode(vertexShaderCode);
	_vertexShaderModule = _device.createShaderModule(vertexShaderCreateInfo);
	vk::PipelineShaderStageCreateInfo vertexPipelineShaderStageCreateInfo = vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, _vertexShaderModule, "VS_main");

	vk::ShaderModuleCreateInfo fragmentShaderCreateInfo({});
	std::vector<uint32_t> fragmentShaderCode = readShader("shaders/f_shader.spv");
	fragmentShaderCreateInfo.setCode(fragmentShaderCode);
	_fragmentShaderModule = _device.createShaderModule(fragmentShaderCreateInfo);
	vk::PipelineShaderStageCreateInfo fragmentPipelineShaderStageCreateInfo = vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, _fragmentShaderModule, "FS_main");

	vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfoArray[] = { vertexPipelineShaderStageCreateInfo, fragmentPipelineShaderStageCreateInfo };

	_pipelineLayout = _device.createPipelineLayout(vk::PipelineLayoutCreateInfo({}));

	vk::DynamicState dynamicStateArray[] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo({}, dynamicStateArray);
	vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo({});

	std::vector<vk::VertexInputBindingDescription> bindingDescriptions = { Vertex::getBindingDescription() };
	std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions = Vertex::getAttributeDescriptions();

	vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo({});
	pipelineVertexInputStateCreateInfo.setVertexBindingDescriptions(bindingDescriptions);
	pipelineVertexInputStateCreateInfo.setVertexAttributeDescriptions(attributeDescriptions);
	vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

	vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo({});
	pipelineRasterizationStateCreateInfo.setRasterizerDiscardEnable(VK_FALSE);
	pipelineRasterizationStateCreateInfo.setLineWidth(1.0);

	vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo({});
	vk::Viewport viewport({});
	viewport.setHeight(static_cast<float>(_windowExtent.height));
	viewport.setWidth(static_cast<float>(_windowExtent.width));
	viewport.setMinDepth(0.0);
	viewport.setMaxDepth(1.0);
	pipelineViewportStateCreateInfo.setViewports(viewport);
	vk::Rect2D scissor({});
	scissor.setExtent(_windowExtent);
	pipelineViewportStateCreateInfo.setScissors(scissor);

	vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState({});
	pipelineColorBlendAttachmentState.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
	vk::PipelineColorBlendStateCreateInfo pipelineColorBlendState({});
	pipelineColorBlendState.setAttachments(pipelineColorBlendAttachmentState);

	vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo({});
	graphicsPipelineCreateInfo.setStages(pipelineShaderStageCreateInfoArray);
	graphicsPipelineCreateInfo.setRenderPass(_renderPass);
	graphicsPipelineCreateInfo.setLayout(_pipelineLayout);
	graphicsPipelineCreateInfo.setPDynamicState(&pipelineDynamicStateCreateInfo);
	graphicsPipelineCreateInfo.setPMultisampleState(&pipelineMultisampleStateCreateInfo);
	graphicsPipelineCreateInfo.setPVertexInputState(&pipelineVertexInputStateCreateInfo);
	graphicsPipelineCreateInfo.setPInputAssemblyState(&pipelineInputAssemblyStateCreateInfo);
	graphicsPipelineCreateInfo.setPRasterizationState(&pipelineRasterizationStateCreateInfo);
	graphicsPipelineCreateInfo.setPViewportState(&pipelineViewportStateCreateInfo);
	graphicsPipelineCreateInfo.setPColorBlendState(&pipelineColorBlendState);

	_graphicsPipeline = _device.createGraphicsPipeline({}, graphicsPipelineCreateInfo).value;
}

void VulkanEngine::initSemaphores() {
	_imageAvailableSemaphore = _device.createSemaphore(vk::SemaphoreCreateInfo({}));
	_renderFinishedSemaphore = _device.createSemaphore(vk::SemaphoreCreateInfo({}));
	vk::FenceCreateInfo fenceCreateInfo({});
	fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
	_inflightFence = _device.createFence(fenceCreateInfo);
}

void VulkanEngine::draw() {
	uint32_t imageIndex = 0;
	
	vk::Result fenceWaitResult = _device.waitForFences(_inflightFence, VK_TRUE, UINT64_MAX);
	if (fenceWaitResult != vk::Result::eSuccess) {
		std::string err = std::format("acquire next image failure: {} ", vk::to_string(fenceWaitResult));
		throw std::runtime_error(err);
	}	
	_device.resetFences(_inflightFence);
	
	vk::Result acquireNextImageResult = _device.acquireNextImageKHR(_swapchain, UINT64_MAX, _imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	if (acquireNextImageResult != vk::Result::eSuccess) {
		std::string err = std::format("acquire next image failure: {} ", vk::to_string(acquireNextImageResult));
		throw std::runtime_error(err);
	}
	
	vk::CommandBuffer* p_commandBuffer = &_commandBuffers[0];
	p_commandBuffer->reset();

	//CommandBuffer begin recording
	vk::CommandBufferBeginInfo commandBufferBeginInfo({});
	p_commandBuffer->begin(commandBufferBeginInfo);

	//RenderPass
	vk::RenderPassBeginInfo renderPassBeginInfo({});
	vk::ClearColorValue clearColorValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	vk::ClearValue clearValue = vk::ClearValue();
	clearValue.setColor(clearColorValue);
	std::vector<vk::ClearValue> clearValues = { clearValue };
	renderPassBeginInfo.setClearValues(clearValues);
	renderPassBeginInfo.setRenderPass(_renderPass);
	renderPassBeginInfo.setFramebuffer(_frameBuffers[imageIndex]);
	renderPassBeginInfo.renderArea.setExtent(_windowExtent);

	p_commandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
	p_commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, _graphicsPipeline);

	vk::Viewport viewport({});
	viewport.setHeight(static_cast<float>(_windowExtent.height));
	viewport.setWidth(static_cast<float>(_windowExtent.width));
	viewport.setMaxDepth(1.0f);
	p_commandBuffer->setViewport(0, viewport);

	vk::Rect2D scissor({});
	scissor.extent = _windowExtent;
	p_commandBuffer->setScissor(0, scissor);
	
	vk::Buffer vertexBuffers[] = { _vertexBuffer };
	vk::DeviceSize offsets[] = { 0 };
	p_commandBuffer->bindVertexBuffers(0, vertexBuffers, offsets);
	p_commandBuffer->bindIndexBuffer(_indexBuffer, 0, vk::IndexType::eUint16);
	
	//p_commandBuffer->draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0);

	p_commandBuffer->drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	p_commandBuffer->endRenderPass();
	p_commandBuffer->end();

	vk::SubmitInfo submitInfo({});

	std::vector<vk::PipelineStageFlags> waitDstStageMasks = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.setPWaitDstStageMask(waitDstStageMasks.data());
	
	std::vector<vk::Semaphore> waitSemaphores = {_imageAvailableSemaphore};
	submitInfo.setWaitSemaphores(waitSemaphores);

	std::vector<vk::Semaphore> signalSemaphores = { _renderFinishedSemaphore };
	submitInfo.setSignalSemaphores(signalSemaphores);

	submitInfo.setCommandBuffers(_commandBuffers);
	std::vector<vk::SubmitInfo> submitInfos = { submitInfo };

	vk::DeviceQueueCreateInfo deviceQueueCreateInfo({});
	_graphicsQueue.submit(submitInfos, _inflightFence);
	
	vk::PresentInfoKHR presentInfo({});
	presentInfo.setWaitSemaphores(_renderFinishedSemaphore);
	std::vector<vk::SwapchainKHR> swapchains = { _swapchain };
	presentInfo.setSwapchains(swapchains);
	presentInfo.setImageIndices(imageIndex);

	vk::Result queuePresentResult = _graphicsQueue.presentKHR(presentInfo);
	if (queuePresentResult != vk::Result::eSuccess) {
		throw queuePresentResult;
	}

}

void VulkanEngine::run() {
	SDL_Event e;
    bool bQuit = false;
	bool stopRendering = false;

    // main loop
    while (!bQuit) {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            // close the window when user alt-f4s or clicks the X button
            if (e.type == SDL_QUIT)
                bQuit = true;

            if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_MINIMIZED) {
                    stopRendering = true;
                }
                if (e.window.event == SDL_WINDOWEVENT_RESTORED) {
                    stopRendering = false;
                }
            }
        }

        // do not draw if we are minimized
        if (stopRendering) {
            // throttle the speed to avoid the endless spinning
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        draw();
    }
	_device.waitIdle();
}

void VulkanEngine::destroy() {
	_device.destroyFence(_inflightFence);
	_device.destroySemaphore(_imageAvailableSemaphore);
	_device.destroySemaphore(_renderFinishedSemaphore);

	_device.destroyShaderModule(_fragmentShaderModule);
	_device.destroyShaderModule(_vertexShaderModule);

	_device.destroyBuffer(_indexBuffer);
	_device.destroyBuffer(_vertexBuffer);
	_device.freeMemory(_deviceMemory);

	_device.destroyCommandPool(_commandPool);
	_device.destroyRenderPass(_renderPass);
	_device.destroyPipeline(_graphicsPipeline);

	_device.destroyPipelineLayout(_pipelineLayout);
	for (vk::Framebuffer fb : _frameBuffers) {
		_device.destroyFramebuffer(fb);
	}
	for (vk::ImageView iv : _imageViews) {
		_device.destroyImageView(iv);
	}
	_device.destroySwapchainKHR(_swapchain);
	_device.destroy();
	_instance.destroySurfaceKHR(_surface);
	_instance.destroy();
}