#pragma once

#include "vulkan/vulkanwindow.hh"
#include "vulkan/renderer.hh"
#include <QtGui/QKeyEvent>
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

void VulkanWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->modifiers() == Qt::NoModifier)
        switch (e->key()) {
        case Qt::Key_R:
            //TODO UPDATE OBJECTS
            break;
        case Qt::Key_O:
            //TODO ORTOGONAL PERSPECTIVE
            break;
        case Qt::Key_P:
            //TODO PLAIN VIS TYPE
            break;
        case Qt::Key_M:
            //TODO MEAN VIS TYPE
            break;
        case Qt::Key_L:
            //TODO SLICING VIS TYPE
            break;
        case Qt::Key_I:
            //TODO ISOPHOTE TEXTURE
            break;
        case Qt::Key_E:
            //TODO ENVIORMENT TEXTURE
            break;
        case Qt::Key_C:
            //TODO SHOW CONTROL POINTS
            break;
        case Qt::Key_S:
            //TODO SHOW SOLID
            break;
        case Qt::Key_W:
            //TODO SHOW WIREFRAME
            break;
        case Qt::Key_X:
            //TODO X STANDARD POS
            break;
        case Qt::Key_Y:
            //TODO Y STANDARD POS
            break;
        case Qt::Key_Z:
            //TODO Z STANDARD POS;
            break;
        default:
           ;
        }
    else if (e->modifiers() == Qt::KeypadModifier)
        switch (e->key()) {
        case Qt::Key_Plus:
            //TODO INCREASE SLICING
            break;
        case Qt::Key_Minus:
            //TODO DECREASE SLICING
            break;
        case Qt::Key_Asterisk:
            //TODO CHANGE SLICING DIR
            break;
        }
}

void VulkanWindow::mouseMoveEvent(QMouseEvent* e)
{
}