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

const double* VulkanWindow::getSlicingDir() const
{
    return renderer->getSlicingDir();
}

double VulkanWindow::getSlicingScaling() const
{
    return renderer->getSlicingScaling();
}

bool VulkanWindow::open(std::string filename)
{
    Object* surface;
    if (filename.ends_with(".bzr"))
        surface = new Bezier(filename);
    else
        surface = new Mesh(filename);
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
            renderer->swapOrthoView();
            break;
        case Qt::Key_P:
            renderer->setVisType(VisType::PLAIN);
            break;
        case Qt::Key_M:
            //TODO
            renderer->setVisType(VisType::MEAN); 
            break;
        case Qt::Key_L:
            renderer->setVisType(VisType::SLICING);
            break;
        case Qt::Key_I:
            renderer->setEnvironmentOrtho(false);
            renderer->setVisType(VisType::ISOPHOTES);
            break;
        case Qt::Key_E:
            renderer->setEnvironmentOrtho(true);
            renderer->setVisType(VisType::ISOPHOTES);
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
            renderer->standardX();
            break;
        case Qt::Key_Y:
            renderer->standardY();
            break;
        case Qt::Key_Z:
            renderer->standardZ();
            break;
        case Qt::Key_Up:
            renderer->setCamVelocity(QVector3D(0, 0, 5));
            break;
        case Qt::Key_Down:
            renderer->setCamVelocity(QVector3D(0, 0, -5));
            break;
        case Qt::Key_Left:
            renderer->setCamVelocity(QVector3D(-5, 0, 0));
            break;
        case Qt::Key_Right:
            renderer->setCamVelocity(QVector3D(5, 0, 0));
            break;
        case Qt::Key_PageUp:
            renderer->setCamVelocity(QVector3D(0, 5, 0));
            break;
        case Qt::Key_PageDown:
            renderer->setCamVelocity(QVector3D(0, -5, 0));
            break;
        default:
            ;
        }
    else if (e->modifiers() == Qt::KeypadModifier)
        switch (e->key()) {
        case Qt::Key_Plus:
            renderer->setSlicingScaling(renderer->getSlicingScaling() * 2);
            break;
        case Qt::Key_Minus:
            renderer->setSlicingScaling(renderer->getSlicingScaling() / 2);
            break;
        case Qt::Key_Asterisk:
            QVector3D dir = renderer->getCamForward();
            dir.normalize();
            renderer->setSlicingDir(dir.x(), dir.y(), dir.z());
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
