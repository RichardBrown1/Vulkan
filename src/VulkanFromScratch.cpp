#include "../include/VulkanEngine.hpp"

int main() {
	
	try {
		VulkanEngine *vulkanEngine = new VulkanEngine();
		vulkanEngine->run();
		vulkanEngine->destroy();
	} catch (vk::SystemError& err) {
		std::cout << "vk::SystemError: " << err.what() << std::endl;
	}
	catch (std::exception& err) {
		std::cout << "std::Exception: " << err.what() << std::endl;
	}
	catch (...) {
		std::cout << "unknown error" << std::endl;
	}


	return 0;
}