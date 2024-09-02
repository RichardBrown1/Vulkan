// VulkanFromScratch.cpp : Defines the entry point for the application.
#pragma once

#include <iostream>
#include <vector>

// TODO: Reference additional headers your program requires here.
#include "vulkan/vulkan.hpp"

#include "Utilities.hpp"

class VulkanEngine {
	public: 
		VulkanEngine();
		void run();
		void destroy();

	private:
		vk::Device _device;
		vk::Instance _instance;
		vk::SurfaceKHR _surface;
		uint32_t _queueFamilyIndex;
		vk::SwapchainKHR _swapchain;
		std::vector<vk::ImageView> _imageViews;
		std::vector<vk::Framebuffer> _frameBuffers;
		vk::ShaderModule _fragmentShaderModule;
		vk::ShaderModule _vertexShaderModule;
		vk::PipelineLayout _pipelineLayout;
		vk::RenderPass _renderPass;
		vk::Pipeline _graphicsPipeline;
		vk::CommandPool _commandPool;
		std::vector<vk::CommandBuffer> _commandBuffers;
		vk::Semaphore _imageAvailableSemaphore;
		vk::Semaphore _renderFinishedSemaphore;
		vk::Fence _inflightFence;
		vk::Extent2D _windowExtent;

		//init
		void initDevice();
		void initSwapchain();
		void initImageViews();
		void initRenderPass();
		void initFramebuffers();
		void initCommandBuffers();
		void initGraphicsPipeline();
		void initSemaphores();

		void draw();

};
