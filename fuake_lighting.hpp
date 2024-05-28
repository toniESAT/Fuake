#pragma once

#include <vector>
#include <string>
#include <amath_core.hpp>
#include <amath_utils.hpp>

using std::string;
using std::vector;
using namespace amath;

namespace fuake {

enum LightType {
   kLightType_Directional,
   kLightType_Point,
   kLightType_Diffuse
};

struct Light {

   LightType type;
   Vec4 direction;

   Light(LightType type, Vec4 direction) : direction(direction), type(type) {}
};

} // namespace fuake