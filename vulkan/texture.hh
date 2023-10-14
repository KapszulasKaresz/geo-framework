#pragma once

#include <qimage.h>
#include <qvulkaninstance.h>
#include <qvulkanwindow.h>


struct Texture {
	VkDeviceSize imageSize;
	VkImage textureImage = VK_NULL_HANDLE;
	VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
	
	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

	VkSampler textureSampler = VK_NULL_HANDLE;
	VkImageView textureImageView = VK_NULL_HANDLE;

	VkCommandPool commandpool;
	QVulkanWindow* m_window;

	Texture();

	void load(QVulkanInstance* inst, VkDevice dev, const char* filename);

	void reset(QVulkanInstance* inst, VkDevice dev);

	~Texture();
private:
	VkCommandBuffer beginSingleTimeCommands(QVulkanInstance* inst, VkDevice dev);
	void endSingleTimeCommands(QVulkanInstance* inst, VkDevice dev, VkCommandBuffer commandBuffer);
	void copyBuffer(QVulkanInstance* inst, VkDevice dev, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void transitionImageLayout(QVulkanInstance* inst, VkDevice dev, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(QVulkanInstance* inst, VkDevice dev, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};
