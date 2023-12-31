#include "application.h"
#include <sstream>

Application::Application(GlfwContext* glfwContext_) {
  glfwContext = glfwContext_;
  context.create(glfwContext);

  swapchain.create(&context, glfwContext);

  std::vector<VkFormat> candidates = {VK_FORMAT_D32_SFLOAT,
                                      VK_FORMAT_D32_SFLOAT_S8_UINT,
                                      VK_FORMAT_D24_UNORM_S8_UINT};
  VkFormatFeatureFlagBits features =
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  bool flag = false;
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(context.getPhysicalDevice(), format,
                                        &props);
    if ((props.optimalTilingFeatures & features) == features) {
      depthFormat = format;
      flag = true;
      break;
    }
  }
  if (!flag) throw std::runtime_error("failed to find supported format!");
}

void Application::mainLoop(Timer& timer) {
  timer.reset();
  std::stringstream title;
  while (!glfwWindowShouldClose(glfwContext->window)) {
    timer.tick();
    title.str("");
    title << timer.getDeltaTime() << "ms Per Frame";
    glfwSetWindowTitle(glfwContext->window, title.str().c_str());
    glfwPollEvents();
    update();
    drawFrame();
  }
  vkDeviceWaitIdle(context.getDevice());
}

void Application::resizeWindow() {
  int width = 0, height = 0;
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(glfwContext->window, &width, &height);
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(context.device);
  swapchain.recreateSwapChain();
}

Application::~Application() {
  swapchain.destroy();
  context.destroy();
}