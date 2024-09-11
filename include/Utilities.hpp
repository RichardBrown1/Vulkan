#pragma once
#include <iostream>
#include "vulkan/vulkan.hpp"

extern vk::PhysicalDevice selectPhysicalDevice(vk::Instance& instance);

std::vector<uint32_t> readShader(const std::string& filename);

uint32_t findMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);

