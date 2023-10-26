#pragma once
#include <qvulkanwindow.h>
#include <qdatetime.h>
#include <QVulkanDeviceFunctions>
#include <QMutex>
#include "vulkan/camera.hh"
#include "vulkan/shader.hh"
#include "object.hh"
#include "texture.hh"
#include "objectstore.hh"


class Renderer : public QVulkanWindowRenderer {
public:
    Renderer(QVulkanWindow* w, bool _wireframe);

    void preInitResources() override;
    void initResources() override;
    void initSwapChainResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;
    void startNextFrame() override;

    void addObject(Object* object);
    void setWireframe(bool _wireframe);
    void setCamVelocity(const QVector3D& _vel);
    void rotateCam(int dx, int dy);
    void rotateObject(int dx, int dy);
    double* getSlicingDir();
    double getSlicingScaling() const { return m_slicingScaling; }
    void setSlicingDir(double x, double y, double z);
    void setSlicingScaling(double scaling) { m_slicingScaling = scaling;}
    void setVisType(VisType visType);
    QVector3D getCamForward() const { return cam.getForward(); }
    void swapOrthoView();
    void setEnvironmentOrtho(bool value) { isEnvironment = value; }
    void standardX() { cam.standardX(); }
    void standardY() { cam.standardY(); }
    void standardZ() { cam.standardZ(); }
    void deleteObjects() { objects.clear(); }
    void update();

    ~Renderer();

    ObjectStore objects;
    Camera cam;
private:

    void createObjectPipeline();
    void ensureBuffers();
    void ensureInstanceBuffer();
    void getMatrices(QMatrix4x4* vp, QMatrix4x4* model, QMatrix3x3* modelNormal, QVector3D* eyePos);
    void writeFragUni(quint8* p, const QVector3D& eyePos);
    void buildDrawCalls();
    void resetPipeline();
    void markViewProjDirty() { m_vpDirty = m_window->concurrentFrameCount(); }
    void moveCam();


    QVulkanWindow* m_window;
    QVulkanDeviceFunctions* m_devFuncs;

    bool hasObject = true;

    bool isOrtho = false;

    Texture m_sphereTexture;
    bool isEnvironment = true;


    QVector3D m_slicingDir = QVector3D(1.0f, 0, 0);
    double m_slicingScaling = 1.0;

    VisType m_visType = VisType::PLAIN;

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

    QMutex m_guiMutex;

    int lastFrame = QDateTime::currentMSecsSinceEpoch();
};