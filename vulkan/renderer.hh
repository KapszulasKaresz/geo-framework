#pragma once
#include <qvulkanwindow.h>
#include <qdatetime.h>
#include <QVulkanDeviceFunctions>
#include <QMutex>
#include "vulkan/camera.hh"
#include "vulkan/shader.hh"
#include "object.hh"

class Renderer : public QVulkanWindowRenderer {
public:
    Renderer(QVulkanWindow* w, bool _wireframe);

    void preInitResources() override;
    void initResources() override;
    void initSwapChainResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;
    void startNextFrame() override;

    void addObject(std::shared_ptr<Object> object);
    void setWireframe(bool _wireframe);
    void setCamVelocity(const QVector3D& _vel);
    void rotateCam(int dx, int dy);
    void rotateObject(int dx, int dy);

    ~Renderer() {}
private:

    void createObjectPipeline();
    void ensureBuffers();
    void ensureInstanceBuffer();
    void getMatrices(QMatrix4x4* vp, QMatrix4x4* model, QMatrix3x3* modelNormal, QVector3D* eyePos);
    void writeFragUni(quint8* p, const QVector3D& eyePos);
    void buildDrawCalls();
    void markViewProjDirty() { m_vpDirty = m_window->concurrentFrameCount(); }
    void moveCam();


    QVulkanWindow* m_window;
    QVulkanDeviceFunctions* m_devFuncs;
   
    std::shared_ptr<Object> object;

    bool hasObject = false;

    Camera cam;
    QVector3D camVelocity = QVector3D(0,0,0);
    QVector3D lightPos;

    float m_rotation_x = 0.0f;
    float m_rotation_y = 0.0f;

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
    bool wireframe = false;
    int m_vpDirty = 0;

    int lastFrame = QDateTime::currentMSecsSinceEpoch();
};