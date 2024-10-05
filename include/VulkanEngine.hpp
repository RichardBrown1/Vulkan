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
		vk::PhysicalDevice _physicalDevice;
		vk::Device _device;
		vk::Queue _graphicsQueue;
		vk::Instance _instance;
		vk::SurfaceKHR _surface;
		vk::SwapchainKHR _swapchain;
		std::vector<vk::ImageView> _imageViews;
		std::vector<vk::Framebuffer> _frameBuffers;
		vk::ShaderModule _fragmentShaderModule;
		vk::ShaderModule _vertexShaderModule;
		vk::DescriptorPool _descriptorPool;
		std::vector<vk::DescriptorSet> _descriptorSets;
		vk::DescriptorSetLayout _descriptorSetLayout;		
		vk::PipelineLayout _pipelineLayout;
		vk::RenderPass _renderPass;
		vk::Pipeline _graphicsPipeline;
		vk::CommandPool _commandPool;
		vk::Buffer _vertexBuffer;
		vk::Buffer _indexBuffer;
		vk::DeviceMemory _deviceMemory;
		std::vector<vk::Buffer> _uniformBuffers;
		std::vector<vk::DeviceMemory> _uniformBufferDeviceMemories;
		std::vector<void*> _uniformBuffersMapped;
		std::vector<vk::CommandBuffer> _commandBuffers;
		std::vector<vk::Semaphore> _imageAvailableSemaphores;
		std::vector<vk::Semaphore> _renderFinishedSemaphores;
		std::vector<vk::Fence> _inflightFences;
		vk::Extent2D _windowExtent;
		uint32_t currentFrame = 0;

		//init
		void initDevice();
		void initSwapchain();
		void initImageViews();
		void initRenderPass();
		void initFramebuffers();
		void initCommandPool();
		void initVertexBuffer();
		void initIndexBuffer();
		void initUniformBuffers();
		void initDescriptorPool();
		void initDescriptorSets();
		void initCommandBuffers();
		void initDescriptorSetLayout();
		void initGraphicsPipeline();
		void initSemaphores();

		//draw
		void draw();
		void updateUniformBuffers();

};
