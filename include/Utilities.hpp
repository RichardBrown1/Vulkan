#pragma once
#include <iostream>
#include "vulkan/vulkan.hpp"

extern vk::PhysicalDevice selectPhysicalDevice(vk::Instance& instance);

std::vector<uint32_t> readShader(const std::string& filename);
