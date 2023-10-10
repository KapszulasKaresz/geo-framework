#pragma once
#include <qwidget.h>
#include <QVulkanWindow>
#include "vulkan/renderer.hh"
#include <qwidget.h>
#include "object.hh"


class VulkanWindow : public QVulkanWindow
{
    Q_OBJECT
public:
    VulkanWindow();
    QVulkanWindowRenderer* createRenderer() override;


    double getCutoffRatio() const { return 0; }
    void setCutoffRatio(double ratio) {}
    double getMeanMin() const { return 0; }
    void setMeanMin(double min){}
    double getMeanMax() const { return 0; }
    void setMeanMax(double max){}
    const double* getSlicingDir() const;
    void setSlicingDir(double x, double y, double z) { renderer->setSlicingDir(x, y, z); }
    double getSlicingScaling() const;
    void setSlicingScaling(double scaling) { renderer->setSlicingScaling(scaling); }
    void deleteObjects(){}
    bool open(std::string filename);

signals:
    void startComputation(QString message);
    void midComputation(int percent);
    void endComputation();

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