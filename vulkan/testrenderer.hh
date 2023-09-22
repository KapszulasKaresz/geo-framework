#pragma once
#include <qvulkanwindow.h>
#include <QVulkanDeviceFunctions>


class TestRenderer : public QVulkanWindowRenderer {
public:
    TestRenderer(QVulkanWindow* w);

    void initResources() override;
    void initSwapChainResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;

    void startNextFrame() override;

private:
    QVulkanWindow* m_window;
    QVulkanDeviceFunctions* m_devFuncs;
    float m_green = 0;
};