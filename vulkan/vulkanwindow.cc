#pragma once

#include "vulkan/vulkanwindow.hh"
#include "testrenderer.hh"

QVulkanWindowRenderer* VulkanWindow::createRenderer()
{
    return new TestRenderer(this);
}