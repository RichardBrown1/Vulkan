#pragma once
#include <iostream>
#include "vulkan/vulkan.hpp"

extern vk::PhysicalDevice selectPhysicalDevice(vk::Instance& instance);

std::vector<uint32_t> readShader(const std::string& filename);

uint32_t findMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);

vk::Buffer createBuffer(vk::Device* device, vk::DeviceMemory* deviceMemory, vk::PhysicalDevice* physicalDevice, vk::BufferUsageFlags bufferUsageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, uint32_t bufferSize);

void copyBuffer(vk::Device device, vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize deviceSize, vk::CommandPool commandPool, vk::Queue queue);
