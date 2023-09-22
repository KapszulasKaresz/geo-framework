#pragma once

#include <QtWidgets/QMainWindow>
#include <qvulkaninstance.h>
#include "vulkan/vulkanwindow.hh"
#include "viewer.hh"

class QApplication;
class QProgressBar;

class Window : public QMainWindow {
  Q_OBJECT

public:
  explicit Window(QApplication *parent);

private slots:
  void open(bool clear_others);
  void setCutoff();
  void setRange();
  void setSlicing();
  void startComputation(QString message);
  void midComputation(int percent);
  void endComputation();

private:
  QApplication *parent;
  Viewer *viewer;
  QVulkanInstance inst;
  VulkanWindow *w;
  QProgressBar *progress;
  QString last_directory;
};
