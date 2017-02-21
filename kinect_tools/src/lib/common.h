#ifndef K3DLIC_COMMON_H_
#define K3DLIC_COMMON_H_

#include <cstddef>
#include <cstdint>

using uchar = std::uint8_t;
using ushort = std::uint16_t;
using real = double;

constexpr std::size_t depth_width = 512;
constexpr std::size_t depth_height = 424;
constexpr std::size_t texture_width = 1920;
constexpr std::size_t texture_height = 1080;

#endif
