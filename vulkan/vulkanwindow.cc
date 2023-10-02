#pragma once

#include "vulkan/vulkanwindow.hh"
#include "vulkan/renderer.hh"
#include <bezier.hh>
#include <mesh.hh>

VulkanWindow::VulkanWindow() :QVulkanWindow()
{
    renderer = new Renderer(this);
}

QVulkanWindowRenderer* VulkanWindow::createRenderer()
{
    return renderer;
}

bool VulkanWindow::open(std::string filename)
{
    std::shared_ptr<Object> surface;
    if (filename.ends_with(".bzr"))
        surface = std::make_shared<Bezier>(filename);
    else
        surface = std::make_shared<Mesh>(filename);
    if (!surface->valid())
        return false;
    
    renderer->addObject(surface);
    return true;
}