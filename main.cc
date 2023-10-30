#include <QtWidgets/QApplication>

#include "window.hh"

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  QVulkanInstance inst;
  //inst.setLayers({ "VK_LAYER_KHRONOS_validation" });
  if (!inst.create())
      qFatal("Failed to create Vulkan instance: %d", inst.errorCode());

  VulkanWindow* vulkanWindow = new VulkanWindow();
  vulkanWindow->setVulkanInstance(&inst);

  Window window(&app, vulkanWindow);
  window.resize(800, 600);
  window.show();
  return app.exec();
}
