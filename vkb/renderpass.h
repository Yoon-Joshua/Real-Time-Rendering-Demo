#pragma once
#include <vulkan/vulkan.h>

#include <array>
#include <memory>
#include <vector>

#include "swapchain.h"

class Framebuffer {
 public:
  VkFramebuffer handle;
};

class RenderPass {
 public:
  void create(Context* context_, VkFormat colorFormat, VkFormat depthFormat,
              VkSampleCountFlagBits sampleCount);

  /// \brief generate shadow map
  void create(Context* context_, VkFormat depthFormat);

  /// \brief generate G-Buffer
  void create(Context* context_, VkFormat positionFormat, VkFormat normalFormat,
              VkFormat colorFormat, VkFormat depthFormat);

  void destroy();

  void createFramebuffers(SwapChain& swapchain, Image& color, Image& depth,
                          uint32_t width, uint32_t height);

  void createFramebuffers(Image& depth, uint32_t width, uint32_t height);

  // \brief create framebuffers of G-Buffer
  void createFramebuffers(Image& color, Image& worldPos, Image& normal,
                          Image& depth, uint32_t width, uint32_t height);

  void destroyFramebuffers();

  VkFramebuffer getFramebuffer(uint32_t index);

  VkRenderPass handle{VK_NULL_HANDLE};

 protected:
  Context* context{nullptr};
  std::vector<VkFramebuffer> framebuffers;
};