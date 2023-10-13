#pragma once
#include "vulkan/renderer.hh"
#include <qvector3d.h>
#include <qlist.h>
#include <qvulkanfunctions.h>

static inline VkDeviceSize aligned(VkDeviceSize v, VkDeviceSize byteAlign)
{
    return (v + byteAlign - 1) & ~(byteAlign - 1);
}


Renderer::Renderer(QVulkanWindow* w, bool _wireframe) : m_window(w), lightPos(QVector3D(0.0f, 50.0f, 100.0f)), cam(QVector3D(0.0f, 0.0f, 5.0f)), wireframe(_wireframe) {
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
    m_material.fragUniSize = aligned(7 * 16 + 12 + 3 * 4, uniAlign);

    m_sphereTexture.commandpool = m_window->graphicsCommandPool();
    m_sphereTexture.m_window = m_window;

    if (!m_material.vs.isValid()) {
        m_material.vs.load(inst, dev, QString("D:/Temalabor/geo-framework/shaders/test_vert.spv"));
    }

    switch (m_visType)
    {
    case VisType::PLAIN:
        if (!m_material.fs.isValid()) {
            m_material.fs.load(inst, dev, wireframe ? QString("D:/Temalabor/geo-framework/shaders/wireframe_frag.spv") : QString("D:/Temalabor/geo-framework/shaders/test_frag.spv"));
        }
        m_sphereTexture.load(inst, dev, "environment.png");
        break;
    case VisType::MEAN:
        break;
    case VisType::SLICING:
        if (!m_material.fs.isValid()) {
            m_material.fs.load(inst, dev, QString("D:/Temalabor/geo-framework/shaders/slicing_frag.spv"));
        }
        break;
    case VisType::ISOPHOTES:
        if (!m_material.fs.isValid()) {
            m_material.fs.load(inst, dev, QString("D:/Temalabor/geo-framework/shaders/isophotes_frag.spv"));
        }
        if (isEnvironment) {
            m_sphereTexture.load(inst, dev, "environment.png");
        }
        else {
            m_sphereTexture.load(inst, dev, "isophotes.png"); 
        }
        break;
    default:
        break;
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
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2 },
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2}
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
        },
        {
            2,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
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
    rs.polygonMode = wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rs.cullMode =  VK_CULL_MODE_NONE;
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
    qDebug("initSwapChainResources");
    if (isOrtho) {
        m_proj = m_window->clipCorrectionMatrix();
        const QSize s = m_window->swapChainImageSize();
        float aratio = s.width() / (float)s.height();
        float top, bottom, right, left;
        if (s.width() > s.height())
        {
            top = 3.0f;
            bottom = -top;
            right = top * aratio;
            left = -right;
        }
        else
        {
            right = 3.0f;
            left = -right;
            top = right / aratio;
            bottom = -top;
        }
        m_proj.ortho(left, right, bottom, top, 0.01f, 400);
    }
    else {
        m_proj = m_window->clipCorrectionMatrix();
        const QSize s = m_window->swapChainImageSize();
        m_proj.perspective(45.0f, s.width() / (float)s.height(), 0.01f, 1000.0f);
    }
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

    if (m_material.pipelineLayout) {
        m_devFuncs->vkDestroyPipelineLayout(dev, m_material.pipelineLayout, nullptr);
        m_material.pipelineLayout = VK_NULL_HANDLE;
    }

    if (m_pipelineCache) {
        m_devFuncs->vkDestroyPipelineCache(dev, m_pipelineCache, nullptr);
        m_pipelineCache = VK_NULL_HANDLE;
    }

    if (m_material.pipeline) {
        m_devFuncs->vkDestroyPipeline(dev, m_material.pipeline, nullptr);
        m_material.pipeline = VK_NULL_HANDLE;
    }

    if (m_material.vs.isValid()) {
        m_devFuncs->vkDestroyShaderModule(dev, m_material.vs.data()->shaderModule, nullptr);
        m_material.vs.reset();
    }
    if (m_material.fs.isValid()) {
        m_devFuncs->vkDestroyShaderModule(dev, m_material.fs.data()->shaderModule, nullptr);
        m_material.fs.reset();
    }

    if (m_objectVertexBuf) {
        m_devFuncs->vkDestroyBuffer(dev, m_objectVertexBuf, nullptr);
        m_objectVertexBuf = VK_NULL_HANDLE;
    }

    if (m_uniBuf) {
        m_devFuncs->vkDestroyBuffer(dev, m_uniBuf, nullptr);
        m_uniBuf = VK_NULL_HANDLE;
    }

    if (m_bufMem) {
        m_devFuncs->vkFreeMemory(dev, m_bufMem, nullptr);
        m_bufMem = VK_NULL_HANDLE;
    }

    if (m_instBuf) {
        m_devFuncs->vkDestroyBuffer(dev, m_instBuf, nullptr);
        m_instBuf = VK_NULL_HANDLE;
    }

    if (m_instBufMem) {
        m_devFuncs->vkFreeMemory(dev, m_instBufMem, nullptr);
        m_instBufMem = VK_NULL_HANDLE;
    }
    QVulkanInstance* inst = m_window->vulkanInstance();
    m_sphereTexture.reset(inst, dev);
}

void Renderer::ensureBuffers()
{
    if (m_objectVertexBuf || !hasObject) {
        return;
    }

    VkDevice dev = m_window->device();
    const int concurrentFrameCount = m_window->concurrentFrameCount();


    VkBufferCreateInfo bufInfo;
    memset(&bufInfo, 0, sizeof(bufInfo));
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    const int meshByteCount = object->getVerticieCount() * 8 * sizeof(float);
    bufInfo.size = meshByteCount;
    bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    VkResult err = m_devFuncs->vkCreateBuffer(dev, &bufInfo, nullptr, &m_objectVertexBuf);
    if (err != VK_SUCCESS)
        qFatal("Failed to create vertex buffer: %d", err);

    VkMemoryRequirements objectVertMemReq;
    m_devFuncs->vkGetBufferMemoryRequirements(dev, m_objectVertexBuf, &objectVertMemReq);

    bufInfo.size = (m_material.fragUniSize + m_material.vertUniSize) * concurrentFrameCount;
    bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    err = m_devFuncs->vkCreateBuffer(dev, &bufInfo, nullptr, &m_uniBuf);
    if (err != VK_SUCCESS)
        qFatal("Failed to create uniform buffer: %d", err);

    VkMemoryRequirements uniMemReq;
    m_devFuncs->vkGetBufferMemoryRequirements(dev, m_uniBuf, &uniMemReq);

    m_material.uniMemStartOffset = aligned(0 + objectVertMemReq.size, uniMemReq.alignment);
    VkMemoryAllocateInfo memAllocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        m_material.uniMemStartOffset + uniMemReq.size,
        m_window->hostVisibleMemoryIndex()
    };
    err = m_devFuncs->vkAllocateMemory(dev, &memAllocInfo, nullptr, &m_bufMem);
    if (err != VK_SUCCESS)
        qFatal("Failed to allocate memory: %d", err);

    err = m_devFuncs->vkBindBufferMemory(dev, m_objectVertexBuf, m_bufMem, 0);
    if (err != VK_SUCCESS)
        qFatal("Failed to bind vertex buffer memory: %d", err);

    err = m_devFuncs->vkBindBufferMemory(dev, m_uniBuf, m_bufMem, m_material.uniMemStartOffset);
    if (err != VK_SUCCESS) 
        qFatal("Failed to bind uniform buffer memory: %d", err);

    //Vertex data upload
    quint8* p;
    err = m_devFuncs->vkMapMemory(dev, m_bufMem, 0, m_material.uniMemStartOffset, 0, reinterpret_cast<void**>(&p));
    if (err != VK_SUCCESS)
        qFatal("Failed to map memory: %d", err);

    float* vertexData = object->getVertexData();
    memcpy(p, vertexData, meshByteCount);
    delete[] vertexData;

    m_devFuncs->vkUnmapMemory(dev, m_bufMem);


    //create discriptor sets for uniform buffers
    const VkPhysicalDeviceLimits* pdevlimits = &m_window->physicalDeviceProperties()->limits;
    const VkDeviceSize uniAlign = pdevlimits->minUniformBufferOffsetAlignment; 


    VkDescriptorBufferInfo vertUni = { m_uniBuf, 0, m_material.vertUniSize };
    VkDescriptorBufferInfo fragUni = { m_uniBuf, m_material.vertUniSize, m_material.fragUniSize };
    VkDescriptorBufferInfo textureBuf = { m_sphereTexture.stagingBuffer ,0 , aligned(m_sphereTexture.imageSize, uniAlign)};

    VkDescriptorImageInfo imageInfo;
    memset(&imageInfo, 0, sizeof(imageInfo));
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_sphereTexture.textureImageView;
    imageInfo.sampler = m_sphereTexture.textureSampler;

    VkWriteDescriptorSet descWrite[3]; 
    memset(descWrite, 0, sizeof(descWrite));
    descWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; 
    descWrite[0].dstSet = m_material.descSet; 
    descWrite[0].dstBinding = 0; 
    descWrite[0].descriptorCount = 1; 
    descWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC; 
    descWrite[0].pBufferInfo = &vertUni; 

    descWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; 
    descWrite[1].dstSet = m_material.descSet; 
    descWrite[1].dstBinding = 1; 
    descWrite[1].descriptorCount = 1; 
    descWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descWrite[1].pBufferInfo = &fragUni;

    descWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite[2].dstSet = m_material.descSet;
    descWrite[2].dstBinding = 1;
    descWrite[2].dstArrayElement = 0;
    descWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descWrite[2].descriptorCount = 1;
    descWrite[2].pImageInfo = &imageInfo;
    descWrite[2].pBufferInfo = &textureBuf;


    m_devFuncs->vkUpdateDescriptorSets(dev, 3, descWrite, 0, nullptr);
}

void Renderer::ensureInstanceBuffer()
{
    if (m_inst || !hasObject) {
        return;
    }

    m_inst = true;

    VkDevice dev = m_window->device(); 

    VkBufferCreateInfo bufInfo; 
    memset(&bufInfo, 0, sizeof(bufInfo)); 
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO; 
    bufInfo.size = 6 * sizeof(float);
    bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; 

    VkResult err = m_devFuncs->vkCreateBuffer(dev, &bufInfo, nullptr, &m_instBuf);
    if (err != VK_SUCCESS)
        qFatal("Failed to create instance buffer: %d", err);

    VkMemoryRequirements memReq; 
    m_devFuncs->vkGetBufferMemoryRequirements(dev, m_instBuf, &memReq); 

    VkMemoryAllocateInfo memAllocInfo = { 
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, 
        nullptr, 
        memReq.size, 
        m_window->hostVisibleMemoryIndex() 
    };
    err = m_devFuncs->vkAllocateMemory(dev, &memAllocInfo, nullptr, &m_instBufMem); 
    if (err != VK_SUCCESS)
        qFatal("Failed to allocate memory: %d", err);

    err = m_devFuncs->vkBindBufferMemory(dev, m_instBuf, m_instBufMem, 0); 
    if (err != VK_SUCCESS)
        qFatal("Failed to bind instance buffer memory: %d", err);

    float* p1 = new float[6];
    float t[] = { 0, 0, 0 };
    memcpy(p1, t, 12);
    float d[] = { 0, 0, 0 };
    memcpy(p1 + 12, d, 12); 

    quint8* p;
    err = m_devFuncs->vkMapMemory(dev, m_instBufMem, 0, 6 * sizeof(float), 0,
        reinterpret_cast<void**>(&p));
    if (err != VK_SUCCESS)
        qFatal("Failed to map memory: %d", err);
    memcpy(p, p,6);
    m_devFuncs->vkUnmapMemory(dev, m_instBufMem);
    delete[] p1;
}

void Renderer::getMatrices(QMatrix4x4* vp, QMatrix4x4* model, QMatrix3x3* modelNormal, QVector3D* eyePos)
{
    model->setToIdentity();
    model->rotate(m_rotation_x, 0, 1, 0);
    model->rotate(m_rotation_y, 1, 0, 0);
    *modelNormal = model->normalMatrix();
    QMatrix4x4 view = cam.viewMatrix();
    *vp = m_proj * view;

    *eyePos = view.inverted().column(3).toVector3D();
}

void Renderer::writeFragUni(quint8* p, const QVector3D& eyePos)
{
    float ECCameraPosition[] = { eyePos.x(), eyePos.y(), eyePos.z() };
    memcpy(p, ECCameraPosition, 12);
    p += 16;

    // Material
    float ka[] = { 0.25f, 0.25f, 0.25f };
    memcpy(p, ka, 12);
    p += 16;

    float kd[] = { 0.7f, 0.7f, 0.7f };
    memcpy(p, kd, 12);
    p += 16;

    float ks[] = { 0.66f, 0.66f, 0.66f };
    memcpy(p, ks, 12);
    p += 16;

    float slicingdir[] = { m_slicingDir.x(), m_slicingDir.y(), m_slicingDir.z() };
    memcpy(p, slicingdir, 12);
    p += 16;

    // Light parameters
    float ECLightPosition[] = { lightPos.x(), lightPos.y(), lightPos.z() };
    memcpy(p, ECLightPosition, 12);
    p += 16;

    float att[] = { 1, 0, 0 };
    memcpy(p, att, 12);
    p += 16;

    float color[] = { 1.0f, 1.0f, 1.0f };
    memcpy(p, color, 12);
    p += 12; // next we have two floats which have an alignment of 4, hence 12 only

    float intensity = 0.8f;
    memcpy(p, &intensity, 4);
    p += 4;

    float specularExp = 1500.0f;
    memcpy(p, &specularExp, 4);
    p += 4;

    float slicingScaling = m_slicingScaling;
    memcpy(p, &slicingScaling, 4);
    p += 4;
}

void Renderer::buildDrawCalls()
{
    if (!hasObject) {
        return;
    }

    VkDevice dev = m_window->device(); 
    VkCommandBuffer cb = m_window->currentCommandBuffer();

    m_devFuncs->vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_material.pipeline);
    
    VkDeviceSize vbOffset = 0;
    m_devFuncs->vkCmdBindVertexBuffers(cb, 0, 1,  &m_objectVertexBuf, &vbOffset);
    m_devFuncs->vkCmdBindVertexBuffers(cb, 1, 1, &m_instBuf, &vbOffset);
    

    uint32_t frameUniOffset = m_window->currentFrame() * (m_material.vertUniSize + m_material.fragUniSize); 
    uint32_t frameUniOffsets[] = { frameUniOffset, frameUniOffset };  
    m_devFuncs->vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_material.pipelineLayout, 0, 1,
        &m_material.descSet, 2, frameUniOffsets); 

    if (m_vpDirty) {
        if (m_vpDirty)
            --m_vpDirty;

        QMatrix4x4 vp, model; 
        QMatrix3x3 modelNormal; 
        QVector3D eyePos; 
        getMatrices(&vp, &model, &modelNormal, &eyePos); 

        quint8* p;
        VkResult err = m_devFuncs->vkMapMemory(dev, m_bufMem,
            m_material.uniMemStartOffset + frameUniOffset,
            m_material.vertUniSize + m_material.fragUniSize,
            0, reinterpret_cast<void**>(&p));
        if (err != VK_SUCCESS)
            qFatal("Failed to map memory: %d", err);


        //vertex Uniforms
        memcpy(p, vp.constData(), 64); 
        memcpy(p + 64, model.constData(), 64); 
        const float* mnp = modelNormal.constData(); 
        memcpy(p + 128, mnp, 12); 
        memcpy(p + 128 + 16, mnp + 3, 12); 
        memcpy(p + 128 + 32, mnp + 6, 12);


        //fragment Uniforms
        p += m_material.vertUniSize;
        writeFragUni(p, eyePos); 

        m_devFuncs->vkUnmapMemory(dev, m_bufMem); 
    }

    m_devFuncs->vkCmdDraw(cb, object->getVerticieCount(), 1, 0, 0);
}

void Renderer::resetPipeline()
{
    releaseResources();
    initResources();
    initSwapChainResources(); 
}

void Renderer::moveCam()
{
    int elapsed = QDateTime::currentMSecsSinceEpoch() - lastFrame;
    cam.walk(camVelocity.z() / 500.0f * elapsed);
    cam.strafe(camVelocity.x() / 500.0f * elapsed);
    cam.fly(camVelocity.y() / 500.0f * elapsed);
    markViewProjDirty();
}

void Renderer::startNextFrame() 
{
    m_guiMutex.lock();

    ensureBuffers();
    ensureInstanceBuffer();

    VkCommandBuffer cb = m_window->currentCommandBuffer();
    const QSize sz = m_window->swapChainImageSize();

    VkClearColorValue clearColor = { { 0.1f, 0.1f, 0.1f, 1.0f } };
    VkClearDepthStencilValue clearDS = { 1, 0 };
    VkClearValue clearValues[3];
    memset(clearValues, 0, sizeof(clearValues));

    clearValues[0].color = clearValues[2].color = clearColor; 
    clearValues[1].depthStencil = clearDS;
    
    VkRenderPassBeginInfo rpBeginInfo;
    memset(&rpBeginInfo, 0, sizeof(rpBeginInfo)); 
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO; 
    rpBeginInfo.renderPass = m_window->defaultRenderPass(); 
    rpBeginInfo.framebuffer = m_window->currentFramebuffer(); 
    rpBeginInfo.renderArea.extent.width = sz.width(); 
    rpBeginInfo.renderArea.extent.height = sz.height(); 
    rpBeginInfo.clearValueCount = m_window->sampleCountFlagBits() > VK_SAMPLE_COUNT_1_BIT ? 3 : 2; 
    rpBeginInfo.pClearValues = clearValues; 
    VkCommandBuffer cmdBuf = m_window->currentCommandBuffer();
    m_devFuncs->vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    VkViewport viewport = { 
        0, 0,
        float(sz.width()), float(sz.height()), 
        0, 1
    };
    m_devFuncs->vkCmdSetViewport(cb, 0, 1, &viewport); 

    VkRect2D scissor = {
        { 0, 0 },
        { uint32_t(sz.width()), uint32_t(sz.height()) } 
    };
    m_devFuncs->vkCmdSetScissor(cb, 0, 1, &scissor); 

   
    moveCam();
    buildDrawCalls();

    m_devFuncs->vkCmdEndRenderPass(cmdBuf);

    lastFrame = QDateTime::currentMSecsSinceEpoch();
    m_guiMutex.unlock();
    m_window->frameReady();
    if (fabs(camVelocity.z()) > 0 || fabs(camVelocity.y()) > 0 || fabs(camVelocity.x()) > 0)
        m_window->requestUpdate();
}

void Renderer::addObject(Object* _object)
{
    m_guiMutex.lock();

    if (object != nullptr) {
        delete object;
        VkDevice dev = m_window->device();
        m_devFuncs->vkDestroyBuffer(dev, m_objectVertexBuf, nullptr);
        m_objectVertexBuf = VK_NULL_HANDLE;
    }
    object = _object;
    hasObject = true;
    m_inst = false;

    double large = std::numeric_limits<double>::max(); 
    Vector box_min(large, large, large), box_max(-large, -large, -large); 
    const auto& mesh = object->baseMesh(); 
    for (auto v : object->baseMesh().vertices()) {
        box_min.minimize(mesh.point(v)); 
        box_max.maximize(mesh.point(v)); 
    }

    QVector3D bl = QVector3D(box_min.data()[0], box_min.data()[1], box_min.data()[2]);
    QVector3D tl = QVector3D(box_max.data()[0], box_max.data()[1], box_max.data()[2]);
    
    cam.updateCameraBasedOnBoundingBox(bl, tl);

    lightPos = tl * 10;

    markViewProjDirty();
    m_window->requestUpdate();
    m_guiMutex.unlock();
}

void Renderer::setWireframe(bool _wireframe)
{
    m_guiMutex.lock();

    if (wireframe == _wireframe) {
        m_guiMutex.unlock();
        return;
    }
    wireframe = _wireframe;
    m_inst = false;
    resetPipeline();
    m_window->requestUpdate();
    m_guiMutex.unlock();
}

void Renderer::setCamVelocity(const QVector3D& _vel)
{
    camVelocity = _vel;
    lastFrame = QDateTime::currentMSecsSinceEpoch();
    m_window->requestUpdate();
}

void Renderer::rotateCam(int dx, int dy)
{
    cam.pitch(dy / 8.0f);
    cam.yaw(dx / 8.0f);

    markViewProjDirty();
    m_window->requestUpdate();
}

void Renderer::rotateObject(int dx, int dy)
{
    m_rotation_x += dx / 8.0f;
    m_rotation_y += dy / 8.0f;
    markViewProjDirty();
    m_window->requestUpdate();
}

double* Renderer::getSlicingDir()
{
    double* ret =new double[3];
    ret[0] = m_slicingDir.x();
    ret[1] = m_slicingDir.y();
    ret[2] = m_slicingDir.z();
    return ret;
}

void Renderer::setSlicingDir(double x, double y, double z)
{
    m_slicingDir.setX(x);
    m_slicingDir.setY(y);
    m_slicingDir.setZ(z);
    m_slicingDir.normalize();
    m_window->requestUpdate();
}

void Renderer::setVisType(VisType visType)
{ 
    m_guiMutex.lock();

    if (m_visType == visType && visType != VisType::ISOPHOTES) {
        m_guiMutex.unlock();
        return;
    }
    m_visType = visType;
    resetPipeline();
    m_window->requestUpdate();
    m_guiMutex.unlock();
}

void Renderer::swapOrthoView()
{
    isOrtho = !isOrtho;
    if (isOrtho) {
        m_proj = m_window->clipCorrectionMatrix();
        const QSize s = m_window->swapChainImageSize();
        float aratio = s.width() / (float)s.height();
        float top, bottom, right, left;
        if (s.width() > s.height())
        {
            top = 3.0f;
            bottom = -top;
            right = top * aratio;
            left = -right;
        }
        else
        {
            right = 3.0f;
            left = -right;
            top = right / aratio;
            bottom = -top;
        }
        m_proj.ortho(left, right ,bottom, top, 0.01f, 1000.0f);
    }
    else {
        m_proj = m_window->clipCorrectionMatrix(); 
        const QSize s = m_window->swapChainImageSize(); 
        m_proj.perspective(45.0f, s.width() / (float)s.height(), 0.01f, 1000.0f);
    }
    markViewProjDirty();
    m_window->requestUpdate();
}

Renderer::~Renderer()
{
    delete object;
}
