#pragma once
#include <qvulkanwindow.h>
#include <QVulkanDeviceFunctions>
#include <QMutex>
#include "vulkan/camera.hh"
#include "vulkan/shader.hh"
#include "object.hh"

class Renderer : public QVulkanWindowRenderer {
public:
    Renderer(QVulkanWindow* w);

    void preInitResources() override;
    void initResources() override;
    void initSwapChainResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;

    void startNextFrame() override;

    void addObject(std::shared_ptr<Object> object);
    ~Renderer() {}
private:

    void createObjectPipeline();
    void ensureBuffers();
    void ensureInstanceBuffer();
    void getMatrices(QMatrix4x4* vp, QMatrix4x4* model, QMatrix3x3* modelNormal, QVector3D* eyePos);
    void writeFragUni(quint8* p, const QVector3D& eyePos);
    void buildDrawCalls();
    void markViewProjDirty() { m_vpDirty = m_window->concurrentFrameCount(); }


    QVulkanWindow* m_window;
    QVulkanDeviceFunctions* m_devFuncs;
   
    std::shared_ptr<Object> object;

    bool hasObject = false;

    Camera cam;
    QVector3D lightPos;

    float m_rotation = 0.0f;

    VkBuffer m_objectVertexBuf = VK_NULL_HANDLE;

    struct {
        VkDeviceSize vertUniSize;
        VkDeviceSize fragUniSize;
        VkDeviceSize uniMemStartOffset;
        Shader vs;
        Shader fs;
        VkDescriptorPool descPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
        VkDescriptorSet descSet;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
    } m_material;

    VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;
    
    VkBuffer m_uniBuf = VK_NULL_HANDLE; 
    VkBuffer m_instBuf = VK_NULL_HANDLE;
    VkDeviceMemory m_bufMem = VK_NULL_HANDLE;
    VkDeviceMemory m_instBufMem = VK_NULL_HANDLE;

    QMatrix4x4 m_proj;
    bool m_inst = false;
    int m_vpDirty = 0;
};