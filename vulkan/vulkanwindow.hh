#pragma once
#include <qwidget.h>
#include <QVulkanWindow>
#include "vulkan/renderer.hh"
#include <qwidget.h>
#include "object.hh"


class VulkanWindow : public QVulkanWindow
{
public:
    VulkanWindow();
    QVulkanWindowRenderer* createRenderer() override;


    double getCutoffRatio() const { return renderer->objects.getCutoffRatio(); }
    void setCutoffRatio(double ratio) { renderer->objects.setCutoffRatio(ratio); renderer->update(); }
    double getMeanMin() const { return renderer->objects.getMeanMin(); }
    void setMeanMin(double min) { renderer->objects.setMeanMin(min); renderer->update();}
    double getMeanMax() const { return renderer->objects.getMeanMax(); }
    void setMeanMax(double max) { renderer->objects.setMeanMax(max); renderer->update();}
    const double* getSlicingDir() const;
    void setSlicingDir(double x, double y, double z) { renderer->setSlicingDir(x, y, z); }
    double getSlicingScaling() const;
    void setSlicingScaling(double scaling) { renderer->setSlicingScaling(scaling); }
    void deleteObjects() { renderer->deleteObjects(); }
    bool open(std::string filename);

private: 
    virtual void keyPressEvent(QKeyEvent* e) override;
    virtual void keyReleaseEvent(QKeyEvent* e) override;
    virtual void mouseMoveEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

    QPoint m_lastpos;
    bool m_ispressed = false;
    Renderer* renderer;
};