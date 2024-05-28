#pragma once

#include <vector>
#include <string>
#include <amath_core.hpp>
#include <amath_utils.hpp>

using std::string;
using std::vector;
using namespace amath;

namespace fuake {

struct FrameBufferMono {
   vector<u8> data;
   Vec2 dims;
   size_t size;

   FrameBufferMono(Vec2 viewport_dimensions) : dims(viewport_dimensions) {
      size = (size_t)dims.x() * (size_t)dims.y();
      data = vector<u8>(size, 0);
   }

   vector<u8> asRGB() {
      vector<u8> rgba_data(size, 0);

      for (int i = 0; i < size; i += 4) {
         
      }
      return rgba_data;
   }
};
} // namespace fuake