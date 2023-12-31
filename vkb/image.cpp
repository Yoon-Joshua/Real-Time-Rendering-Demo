#include "image.h"

#include "command_buffer.h"
#define VMA_IMPLEMENTATION

void Image::create(Context* context_, uint32_t width, uint32_t height,
                   uint32_t mipLevels, VkSampleCountFlagBits numSamples,
                   VkFormat format, VkImageUsageFlags usage,
                   VkImageAspectFlagBits aspectFlags, bool isCube) {
  context = context_;
  this->format = format;
  this->mipLevels = mipLevels;
  this->width = width;
  this->height = height;
  this->numSamples = numSamples;
  this->usage = usage;
  this->aspectFlags = aspectFlags;
  this->layers = isCube ? 6 : 1;

  // create image
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = mipLevels;
  imageInfo.arrayLayers = this->layers;
  imageInfo.format = format;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = numSamples;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.flags = isCube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
  VmaAllocationCreateInfo allocInfo{};
  allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  if (vmaCreateImage(context->allocator, &imageInfo, &allocInfo, &handle,
                     &allocation, nullptr) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  // create view
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = handle;
  viewInfo.viewType = isCube ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = mipLevels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = layers;
  if (vkCreateImageView(context->device, &viewInfo, nullptr, &readView) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create image view!");
  }

  viewInfo.subresourceRange.levelCount = 1;
  if (vkCreateImageView(context->device, &viewInfo, nullptr, &writeView) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create image view!");
  }
}

void Image::destroy() {
  vkDestroyImageView(context->device, readView, nullptr);
  vkDestroyImageView(context->device, writeView, nullptr);
  vmaDestroyImage(context->allocator, handle, allocation);
}

void Image::transitionImageLayout(VkImageLayout oldLayout,
                                  VkImageLayout newLayout) {
  CommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(context);

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = handle;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = mipLevels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = this->layers;

  // if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
  //	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  //	if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format ==
  // VK_FORMAT_D24_UNORM_S8_UINT) { 		barrier.subresourceRange.aspectMask
  // |= VK_IMAGE_ASPECT_STENCIL_BIT;
  //	}
  // }
  // else {
  //	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  // }

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;
  barrier.subresourceRange.aspectMask = this->aspectFlags;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  // ----���Ǳ���ģ�vulkan������ʽ��ִ��-------------
  else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
           newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  }
  // -----------------------------------------------
  else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  vkCmdPipelineBarrier(commandBuffer.handle, sourceStage, destinationStage, 0,
                       0, nullptr, 0, nullptr, 1, &barrier);

  CommandBuffer::endSingleTimeCommands(commandBuffer);
}

void Image::copyToImage(Image& image) {
  CommandBuffer cb = CommandBuffer::beginSingleTimeCommands(context);
  VkImageBlit blitInfo{};
  blitInfo.srcOffsets[0] = {0, 0, 0};
  blitInfo.srcOffsets[1] = {static_cast<int>(width), static_cast<int>(height),
                            1};
  blitInfo.dstOffsets[0] = {0, 0, 0};
  blitInfo.dstOffsets[1] = {static_cast<int>(width), static_cast<int>(height),
                            1};
  blitInfo.dstSubresource.layerCount = 1;
  blitInfo.srcSubresource.layerCount = 1;
  blitInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  blitInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

  vkCmdBlitImage(cb.handle, handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                 image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                 &blitInfo, VK_FILTER_NEAREST);

  CommandBuffer::endSingleTimeCommands(cb);
}

void Image::resize(uint32_t width_, uint32_t height_) {
  destroy();
  create(context, width_, height_, mipLevels, numSamples, format, usage, aspectFlags);
  this->width = width_;
  this->height = height_;
}
