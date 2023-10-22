#pragma once

#include <vector>
#include <fstream>
#include <string>
#include "image.h"
#include "buffer.h"

std::vector<char> readFile(const std::string& filename);

void copy(Context* context, Image& dst, Buffer& src, uint32_t width, uint32_t height);

void copy(Context* context, Buffer& dst, Image& src, uint32_t mipLevel, uint32_t width, uint32_t height);