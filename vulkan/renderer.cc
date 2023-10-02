#pragma once
#include "vulkan/renderer.hh"
#include <qvector3d.h>
#include <qlist.h>
#include <qvulkanfunctions.h>

static inline VkDeviceSize aligned(VkDeviceSize v, VkDeviceSize byteAlign)
{
    return (v + byteAlign - 1) & ~(byteAlign - 1);
}


Renderer::Renderer(QVulkanWindow* w) : m_window(w), lightPos(QVector3D(0.0f, 5.0f, 15.0f)), cam(QVector3D(0.0f, 0.0f, 10.0f)) {
}

void Renderer::preInitResources() {
    qDebug("preInitResources");
    const QList<int> sampleCounts = m_window->supportedSampleCounts();
    qDebug() << "Supported sample counts:" << sampleCounts;
    if (sampleCounts.contains(4)) {
        m_window->setSampleCount(4);
    }
}

void Renderer::initResources() {
	qDebug("initResources");

    QVulkanInstance* inst = m_window->vulkanInstance();
    VkDevice dev = m_window->device();
    const VkPhysicalDeviceLimits* pdevlimits = &m_window->physicalDeviceProperties()->limits;
    const VkDeviceSize uniAlign = pdevlimits->minUniformBufferOffsetAlignment;

    m_devFuncs = inst->deviceFunctions(dev);

    m_material.vertUniSize = aligned(2 * 64 + 48, uniAlign);
    m_material.fragUniSize = aligned(6 * 16 + 12 + 2 * 4, uniAlign);

    if (!m_material.vs.isValid()) {
        m_material.vs.load(inst, dev, QString("D:/Temalabor/geo-framework/shaders/test_vert.spv"));
    }
    if (!m_material.fs.isValid()) {
        m_material.fs.load(inst, dev, QString("D:/Temalabor/geo-framework/shaders/test_frag.spv"));
    }

    VkPipelineCacheCreateInfo pipelineCacheInfo;
    memset(&pipelineCacheInfo, 0, sizeof(pipelineCacheInfo));
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult err = m_devFuncs->vkCreatePipelineCache(dev, &pipelineCacheInfo, nullptr, &m_pipelineCache);

    if (err != VK_SUCCESS) {
        qFatal("Failed to create pipeline cache %d", err);
    }

    createObjectPipeline();
}

void Renderer::createObjectPipeline()
{
    VkDevice dev = m_window->device();

    // Vertex layout.
    VkVertexInputBindingDescription vertexBindingDesc[] = {
        {
            0, // binding
            8 * sizeof(float),
            VK_VERTEX_INPUT_RATE_VERTEX
        },
        {
            1,
            6 * sizeof(float),
            VK_VERTEX_INPUT_RATE_INSTANCE
        }
    };
    VkVertexInputAttributeDescription vertexAttrDesc[] = {
        { // position
            0, // location
            0, // binding
            VK_FORMAT_R32G32B32_SFLOAT,
            0 // offset
        },
        { // normal
            1,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            5 * sizeof(float)
        },
        { // instTranslate
            2,
            1,
            VK_FORMAT_R32G32B32_SFLOAT,
            0
        },
        { // instDiffuseAdjust
            3,
            1,
            VK_FORMAT_R32G32B32_SFLOAT,
            3 * sizeof(float)
        }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = sizeof(vertexBindingDesc) / sizeof(vertexBindingDesc[0]);
    vertexInputInfo.pVertexBindingDescriptions = vertexBindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = sizeof(vertexAttrDesc) / sizeof(vertexAttrDesc[0]);
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttrDesc;

    // Descriptor set layout.
    VkDescriptorPoolSize descPoolSizes[] = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2 }
    };
    VkDescriptorPoolCreateInfo descPoolInfo;
    memset(&descPoolInfo, 0, sizeof(descPoolInfo));
    descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descPoolInfo.maxSets = 1; // a single set is enough due to the dynamic uniform buffer
    descPoolInfo.poolSizeCount = sizeof(descPoolSizes) / sizeof(descPoolSizes[0]);
    descPoolInfo.pPoolSizes = descPoolSizes;
    VkResult err = m_devFuncs->vkCreateDescriptorPool(dev, &descPoolInfo, nullptr, &m_material.descPool);
    if (err != VK_SUCCESS)
        qFatal("Failed to create descriptor pool: %d", err);

    VkDescriptorSetLayoutBinding layoutBindings[] =
    {
        {
            0, // binding
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1, // descriptorCount
            VK_SHADER_STAGE_VERTEX_BIT,
            nullptr
        },
        {
            1,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr
        }
    };
    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        sizeof(layoutBindings) / sizeof(layoutBindings[0]),
        layoutBindings
    };
    err = m_devFuncs->vkCreateDescriptorSetLayout(dev, &descLayoutInfo, nullptr, &m_material.descSetLayout);
    if (err != VK_SUCCESS)
        qFatal("Failed to create descriptor set layout: %d", err);

    VkDescriptorSetAllocateInfo descSetAllocInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        m_material.descPool,
        1,
        &m_material.descSetLayout
    };
    err = m_devFuncs->vkAllocateDescriptorSets(dev, &descSetAllocInfo, &m_material.descSet);
    if (err != VK_SUCCESS)
        qFatal("Failed to allocate descriptor set: %d", err);

    // Graphics pipeline.
    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    memset(&pipelineLayoutInfo, 0, sizeof(pipelineLayoutInfo));
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_material.descSetLayout;

    err = m_devFuncs->vkCreatePipelineLayout(dev, &pipelineLayoutInfo, nullptr, &m_material.pipelineLayout);
    if (err != VK_SUCCESS)
        qFatal("Failed to create pipeline layout: %d", err);

    VkGraphicsPipelineCreateInfo pipelineInfo;
    memset(&pipelineInfo, 0, sizeof(pipelineInfo));
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            m_material.vs.data()->shaderModule,
            "main",
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            m_material.fs.data()->shaderModule,
            "main",
            nullptr
        }
    };
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;

    VkPipelineInputAssemblyStateCreateInfo ia;
    memset(&ia, 0, sizeof(ia));
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInfo.pInputAssemblyState = &ia;

    VkPipelineViewportStateCreateInfo vp;
    memset(&vp, 0, sizeof(vp));
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;
    pipelineInfo.pViewportState = &vp;

    VkPipelineRasterizationStateCreateInfo rs;
    memset(&rs, 0, sizeof(rs));
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;
    pipelineInfo.pRasterizationState = &rs;

    VkPipelineMultisampleStateCreateInfo ms;
    memset(&ms, 0, sizeof(ms));
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = m_window->sampleCountFlagBits();
    pipelineInfo.pMultisampleState = &ms;

    VkPipelineDepthStencilStateCreateInfo ds;
    memset(&ds, 0, sizeof(ds));
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pipelineInfo.pDepthStencilState = &ds;

    VkPipelineColorBlendStateCreateInfo cb;
    memset(&cb, 0, sizeof(cb));
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    VkPipelineColorBlendAttachmentState att;
    memset(&att, 0, sizeof(att));
    att.colorWriteMask = 0xF;
    cb.attachmentCount = 1;
    cb.pAttachments = &att;
    pipelineInfo.pColorBlendState = &cb;

    VkDynamicState dynEnable[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn;
    memset(&dyn, 0, sizeof(dyn));
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = sizeof(dynEnable) / sizeof(VkDynamicState);
    dyn.pDynamicStates = dynEnable;
    pipelineInfo.pDynamicState = &dyn;

    pipelineInfo.layout = m_material.pipelineLayout;
    pipelineInfo.renderPass = m_window->defaultRenderPass();

    err = m_devFuncs->vkCreateGraphicsPipelines(dev, m_pipelineCache, 1, &pipelineInfo, nullptr, &m_material.pipeline);
    if (err != VK_SUCCESS)
        qFatal("Failed to create graphics pipeline: %d", err);
}

void Renderer::initSwapChainResources() {
    m_proj = m_window->clipCorrectionMatrix();
    const QSize s = m_window->swapChainImageSize();
    m_proj.perspective(45.0f, s.width() / s.height(), 0.01f, 1000.0f);
    markViewProjDirty();
} 
void Renderer::releaseSwapChainResources() {
    m_window->frameReady();
} 
void Renderer::releaseResources() {
    qDebug("Renderer release");
    VkDevice dev = m_window->device();

    if (m_material.descSetLayout) {
        m_devFuncs->vkDestroyDescriptorSetLayout(dev, m_material.descSetLayout, nullptr);
        m_material.descSetLayout = VK_NULL_HANDLE;
    }

    if (m_material.descPool) {
        m_devFuncs->vkDestroyDescriptorPool(dev, m_material.descPool, nullptr);
        m_material.descPool = VK_NULL_HANDLE;
    }

    if (m_material.pipeline) {
        m_devFuncs->vkDestroyPipeline(dev, m_material.pipeline, nullptr);
        m_material.pipeline = VK_NULL_HANDLE;
    }

    if (m_material.pipelineLayout) {
        m_devFuncs->vkDestroyPipelineLayout(dev, m_material.pipelineLayout, nullptr);
        m_material.pipelineLayout = VK_NULL_HANDLE;
    }

    if (m_pipelineCache) {
        m_devFuncs->vkDestroyPipelineCache(dev, m_pipelineCache, nullptr);
        m_pipelineCache = VK_NULL_HANDLE;
    }

    if (m_material.vs.isValid()) {
        m_devFuncs->vkDestroyShaderModule(dev, m_material.vs.data()->shaderModule, nullptr);
        m_material.vs.reset();
    }
    if (m_material.fs.isValid()) {
        m_devFuncs->vkDestroyShaderModule(dev, m_material.fs.data()->shaderModule, nullptr);
        m_material.fs.reset();
    }
}

void Renderer::startNextFrame() 
{
   
}

void Renderer::addObject(std::shared_ptr<Object> object)
{
    objects.push_back(object);
}


