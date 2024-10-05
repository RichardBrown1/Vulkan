#pragma once
#include <iostream>
#include "vulkan/vulkan.hpp"

extern vk::PhysicalDevice selectPhysicalDevice(vk::Instance& instance);

std::vector<uint32_t> readShader(const std::string& filename);

uint32_t findMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);

void createBuffer(vk::PhysicalDevice& physicalDevice, vk::Device& device, vk::DeviceSize bufferSize, vk::BufferUsageFlags bufferUsageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Buffer& buffer, vk::DeviceMemory& deviceMemory);

void copyBuffer(vk::Device device, vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize deviceSize, vk::CommandPool commandPool, vk::Queue queue);
