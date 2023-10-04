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
    const double* getSlicingDir() const { return 0; }
    void setSlicingDir(double x, double y, double z){}
    double getSlicingScaling() const { return 0; }
    void setSlicingScaling(double scaling){}
    void deleteObjects(){}
    bool open(std::string filename);

signals:
    void startComputation(QString message);
    void midComputation(int percent);
    void endComputation();

private: 
    virtual void keyPressEvent(QKeyEvent* e) override;
    virtual void mouseMoveEvent(QMouseEvent* e) override;

    Renderer* renderer;
};