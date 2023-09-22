#pragma once
#include <qwidget.h>
#include <QVulkanWindow>


class VulkanWindow : public QVulkanWindow
{
public:
    QVulkanWindowRenderer* createRenderer() override;
};