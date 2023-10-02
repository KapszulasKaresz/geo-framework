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
    void markViewProjDirty() { m_vpDirty = m_window->concurrentFrameCount(); }


    QVulkanWindow* m_window;
    QVulkanDeviceFunctions* m_devFuncs;

    std::vector<std::shared_ptr<Object>> objects;
    Camera cam;
    QVector3D lightPos;

    VkBuffer objectVertexBuffer = VK_NULL_HANDLE;

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

    QMatrix4x4 m_proj;
    int m_vpDirty = 0;
};