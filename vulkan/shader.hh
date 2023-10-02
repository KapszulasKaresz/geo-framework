#pragma once

#include <qvulkaninstance.h>

struct ShaderData {
	VkShaderModule shaderModule = VK_NULL_HANDLE;
	bool isValid() const { return shaderModule != VK_NULL_HANDLE; }
};

class Shader {
	ShaderData s_data;
public:
	void load(QVulkanInstance* inst, VkDevice dev, const QString& filename);
	ShaderData* data();
	bool isValid() { return  data()->isValid(); }
	void reset();
};