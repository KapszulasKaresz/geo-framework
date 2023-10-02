#pragma once
#include "vulkan/shader.hh"
#include <qfile.h>
#include <qdir.h>
#include <QVulkanDeviceFunctions>

void Shader::load(QVulkanInstance* inst, VkDevice dev, const QString& filename)
{
	reset();
	QFile f(filename);
	if (!f.open(QIODevice::ReadOnly)) {
		qWarning() << "Failed to open file: " << f.errorString();
		return;
	}

    QByteArray blob = f.readAll();
    VkShaderModuleCreateInfo shaderInfo;
    memset(&shaderInfo, 0, sizeof(shaderInfo));
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = blob.size();
    shaderInfo.pCode = reinterpret_cast<const uint32_t*>(blob.constData());
    VkResult err = inst->deviceFunctions(dev)->vkCreateShaderModule(dev, &shaderInfo, nullptr, &s_data.shaderModule);
    if (err != VK_SUCCESS) {
        qWarning("Failed to create shader module: %d", err);
    }
}

ShaderData* Shader::data()
{
	return &s_data;
}

void Shader::reset()
{
	s_data = ShaderData();	
}
