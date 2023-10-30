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
  explicit Window(QApplication *parent, VulkanWindow* window);

private slots:
  void open(bool clear_others);
  void setCutoff();
  void setRange();
  void setSlicing();
  void startComputation(QString message);
  void midComputation(int percent);
  void endComputation();

  void closeEvent(QCloseEvent* event);

private:
  QApplication *parent;
  VulkanWindow *viewer;
  QVulkanInstance inst;
  QProgressBar *progress;
  QString last_directory;
};
