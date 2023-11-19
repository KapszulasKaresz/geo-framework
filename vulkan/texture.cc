#pragma once
#include "texture.hh"
#include <QVulkanDeviceFunctions>
#define STB_IMAGE_IMPLEMENTATION
#include "outside/stb_image.h"
	
Texture::Texture()
{
}




void Texture::load(QVulkanInstance* inst, VkDevice dev, const char* filename)
{
	int texWidth, texHeight, texChannels; 
	stbi_uc* pixels = stbi_load(filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	imageSize = texWidth * texHeight * 4; 

	if (!pixels) {
		qFatal("Failed to load image");
	}

	VkBufferCreateInfo bufferInfo;
	memset(&bufferInfo, 0, sizeof(bufferInfo));
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = imageSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkResult err = inst->deviceFunctions(dev)->vkCreateBuffer(dev, &bufferInfo, nullptr, &stagingBuffer);
	if (err != VK_SUCCESS)
		qFatal("Failed to create Stagin buffer: %d", err);

	VkMemoryRequirements memRequirements;
	inst->deviceFunctions(dev)->vkGetBufferMemoryRequirements(dev, stagingBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo; 
	memset(&allocInfo, 0, sizeof(allocInfo));
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; 
	allocInfo.allocationSize = memRequirements.size; 
	allocInfo.memoryTypeIndex = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	err = inst->deviceFunctions(dev)->vkAllocateMemory(dev, &allocInfo, nullptr, &stagingBufferMemory);
	if (err != VK_SUCCESS)
		qFatal("Failed to allocate Stagin buffer memory: %d", err);

	err = inst->deviceFunctions(dev)->vkBindBufferMemory(dev, stagingBuffer, stagingBufferMemory, 0);
	if (err != VK_SUCCESS)
		qFatal("Failed to bind buffer memory: %d", err);

	float* data = nullptr;
	err = inst->deviceFunctions(dev)->vkMapMemory(dev, stagingBufferMemory, 0, imageSize, 0, reinterpret_cast<void**>(&data));
	if (err != VK_SUCCESS)
		qFatal("Failed to map memory: %d", err);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	inst->deviceFunctions(dev)->vkUnmapMemory(dev, stagingBufferMemory);

	VkImageCreateInfo imageInfo; 
	memset(&imageInfo, 0, sizeof(imageInfo));
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO; 
	imageInfo.imageType = VK_IMAGE_TYPE_2D; 
	imageInfo.extent.width = static_cast<uint32_t>(texWidth); 
	imageInfo.extent.height = static_cast<uint32_t>(texHeight); 
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB; 
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; 
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; 
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; 
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; 
	imageInfo.flags = 0;

	err = inst->deviceFunctions(dev)->vkCreateImage(dev, &imageInfo, nullptr, &textureImage); 
	if (err != VK_SUCCESS)
		qFatal("Failed to create textureImage: %d", err);

	inst->deviceFunctions(dev)->vkGetImageMemoryRequirements(dev, textureImage, &memRequirements);

	allocInfo;
	memset(&allocInfo, 0, sizeof(allocInfo));
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; 
	allocInfo.allocationSize = memRequirements.size; 
	allocInfo.memoryTypeIndex = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	err = inst->deviceFunctions(dev)->vkAllocateMemory(dev, &allocInfo, nullptr, &textureImageMemory);
	if (err != VK_SUCCESS)
		qFatal("Failed to allocate image memory: %d", err);

	err = inst->deviceFunctions(dev)->vkBindImageMemory(dev, textureImage, textureImageMemory, 0);
	if (err != VK_SUCCESS)
		qFatal("Failed to bind image memory: %d", err);

	VkImageViewCreateInfo viewInfo;
	memset(&viewInfo, 0, sizeof(viewInfo));
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = textureImage; 
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; 
	viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB; 
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
	viewInfo.subresourceRange.baseMipLevel = 0; 
	viewInfo.subresourceRange.levelCount = 1; 
	viewInfo.subresourceRange.baseArrayLayer = 0; 
	viewInfo.subresourceRange.layerCount = 1; 

	err = inst->deviceFunctions(dev)->vkCreateImageView(dev, &viewInfo, nullptr, &textureImageView);
	if (err != VK_SUCCESS)
		qFatal("Failed to create texture imageview: %d", err);


	VkSamplerCreateInfo samplerInfo;
	memset(&samplerInfo, 0, sizeof(samplerInfo));
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR; 
	samplerInfo.minFilter = VK_FILTER_LINEAR; 
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; 
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; 
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE; 
	samplerInfo.maxAnisotropy = 4.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; 
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE; 
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; 
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; 
	samplerInfo.mipLodBias = 0.0f; 
	samplerInfo.minLod = 0.0f; 
	samplerInfo.maxLod = 0.0f; 

	err = inst->deviceFunctions(dev)->vkCreateSampler(dev, &samplerInfo, nullptr, &textureSampler); 
	if (err != VK_SUCCESS)
		qFatal("Failed to create sampler: %d", err);

	transitionImageLayout(inst, dev, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(inst, dev, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	transitionImageLayout(inst, dev,textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	inst->deviceFunctions(dev)->vkDestroyBuffer(dev, stagingBuffer, nullptr);
	inst->deviceFunctions(dev)->vkFreeMemory(dev, stagingBufferMemory, nullptr);
}

VkCommandBuffer Texture::beginSingleTimeCommands(QVulkanInstance* inst, VkDevice dev)
{
	VkCommandBufferAllocateInfo allocInfo;
	memset(&allocInfo, 0, sizeof(allocInfo));
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandpool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer; 
	inst->deviceFunctions(dev)->vkAllocateCommandBuffers(dev, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	inst->deviceFunctions(dev)->vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Texture::endSingleTimeCommands(QVulkanInstance* inst, VkDevice dev, VkCommandBuffer commandBuffer)
{
	inst->deviceFunctions(dev)->vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo;
	memset(&submitInfo, 0, sizeof(submitInfo));
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	inst->deviceFunctions(dev)->vkQueueSubmit(m_window->graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	inst->deviceFunctions(dev)->vkQueueWaitIdle(m_window->graphicsQueue());

	inst->deviceFunctions(dev)->vkFreeCommandBuffers(dev, commandpool, 1, &commandBuffer);
}

void Texture::copyBuffer(QVulkanInstance* inst, VkDevice dev, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(inst, dev);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	inst->deviceFunctions(dev)->vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(inst, dev, commandBuffer);
}

void Texture::transitionImageLayout(QVulkanInstance* inst, VkDevice dev, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(inst, dev);

	VkImageMemoryBarrier barrier;
	memset(&barrier, 0, sizeof(barrier));
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; 
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; 
	barrier.image = image; 
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
	barrier.subresourceRange.baseMipLevel = 0; 
	barrier.subresourceRange.levelCount = 1; 
	barrier.subresourceRange.baseArrayLayer = 0; 
	barrier.subresourceRange.layerCount = 1; 

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	inst->deviceFunctions(dev)->vkCmdPipelineBarrier( 
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	endSingleTimeCommands(inst, dev, commandBuffer);
}

void Texture::copyBufferToImage(QVulkanInstance* inst, VkDevice dev, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(inst,dev);

	VkBufferImageCopy region;
	memset(&region, 0, sizeof(region));
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	inst->deviceFunctions(dev)->vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	endSingleTimeCommands(inst,dev,commandBuffer);
}

void Texture::reset(QVulkanInstance* inst, VkDevice dev)
{
	inst->deviceFunctions(dev)->vkDestroyImage(dev, textureImage, nullptr);
	inst->deviceFunctions(dev)->vkFreeMemory(dev, textureImageMemory, nullptr);
	textureImage = VK_NULL_HANDLE;
	textureImageMemory = VK_NULL_HANDLE;

	inst->deviceFunctions(dev)->vkDestroyImageView(dev, textureImageView, nullptr);
	textureImageView = VK_NULL_HANDLE;
	
	inst->deviceFunctions(dev)->vkDestroySampler(dev, textureSampler, nullptr);
	inst->deviceFunctions(dev)->vkDestroyImageView(dev, textureImageView, nullptr);
	textureSampler = VK_NULL_HANDLE;
	textureImageView = VK_NULL_HANDLE;
}

Texture::~Texture()
{
}
