#pragma once

#include "vulkan/vulkanwindow.hh"
#include "vulkan/renderer.hh"
#include <QtGui/QKeyEvent>
#include <bezier.hh>
#include <mesh.hh>

VulkanWindow::VulkanWindow() :QVulkanWindow()
{
    renderer = new Renderer(this, false);
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
            renderer->setWireframe(false);
            break;
        case Qt::Key_W:
            renderer->setWireframe(true);
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
        case Qt::Key_Up:
            renderer->setCamVelocity(QVector3D(0, 1, 0));
            break;
        case Qt::Key_Down:
            renderer->setCamVelocity(QVector3D(0, -1, 0));
            break;
        case Qt::Key_Left:
            renderer->setCamVelocity(QVector3D(-1, 0, 0));
            break;
        case Qt::Key_Right:
            renderer->setCamVelocity(QVector3D(1, 0, 0));
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

void VulkanWindow::keyReleaseEvent(QKeyEvent* e)
{
    renderer->setCamVelocity(QVector3D(0, 0, 0));
}

void VulkanWindow::mouseMoveEvent(QMouseEvent* e)
{
    if (!m_ispressed)
        return;

    int dx = e->position().toPoint().x() - m_lastpos.x();
    int dy = e->position().toPoint().y() - m_lastpos.y();

    renderer->rotateCam(dx, dy);

    m_lastpos = e->position().toPoint();
}

void VulkanWindow::mousePressEvent(QMouseEvent* e)
{
    m_ispressed = true;
    m_lastpos = e->position().toPoint();
}

void VulkanWindow::mouseReleaseEvent(QMouseEvent* e)
{
    m_ispressed = false;
}
