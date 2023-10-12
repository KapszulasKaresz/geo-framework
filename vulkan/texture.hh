#pragma once

#include <qimage.h>
#include <qvulkaninstance.h>
#include <qvulkanwindow.h>


struct Texture {
	QImage img;
	VkDeviceSize imageSize;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VkSampler textureSampler;
	VkImageView textureImageView;

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
