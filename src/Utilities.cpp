#pragma once
#include "../include/Utilities.hpp"
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

extern vk::PhysicalDevice selectPhysicalDevice(vk::Instance& instance) {
	auto physicalDevices = instance.enumeratePhysicalDevices();
	vk::PhysicalDevice selectedPhysicalDevice;
	
	//Available Devices 
	std::cout << "Available Devices: " << std::endl;
	for (vk::PhysicalDevice physicalDevice : physicalDevices) {
		std::cout << physicalDevice.getProperties2().properties.deviceName << std::endl;
		if (physicalDevice.getProperties2().properties.apiVersion >= vk::ApiVersion13) {
			selectedPhysicalDevice = physicalDevice;
		}					
	}
	std::cout << std::endl;
	
	{
		auto properties = selectedPhysicalDevice.getProperties2().properties;
		std::cout << "Selected Device: " << properties.deviceName << std::endl;
		std::cout << "driverVersion: " << properties.driverVersion << std::endl;
		std::cout << std::endl;			
	}
	return selectedPhysicalDevice;
}

std::vector<uint32_t> readShader(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open SPIR-V Shader file.");
	}

	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
	file.close();

	return buffer;
}