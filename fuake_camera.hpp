#pragma once

#include <math.h>
#include <vector>
#include <string>

#include <amath_core.hpp>
#include <amath_utils.hpp>

using std::string;
using std::vector;
using namespace amath;

namespace fuake {

struct Camera {

   Vec4 position;
   Vec4 forward;
   float pitch_clamp = 0.0174533f; // 1 deg

   Vec4 right() { return cross_product({0, 1, 0, 0}, forward).normalized(); }
   Vec4 up() { return cross_product(forward, right()).normalized(); }
   float get_pitch_cos() { return dot_product(Vec4::up(), forward); }
   float get_pitch() { return acosf(get_pitch_cos()); }

   Camera(Vec4 position, Vec4 forward) : position(position), forward(forward){};

   /* Set max pitch angle in degrees */
   void set_pitch_clamp(float angle) { pitch_clamp = fabs(cosf(Deg2Rad(angle))); }

   void change_pitch(float angle) {
      // PITCH: Rotate around camera right axis (look up-down)
      float new_angle = get_pitch() - angle; // Fix for left-handed coord system
      if (new_angle < pitch_clamp || new_angle > PI - pitch_clamp) return;

      forward = (Mat4::rotation_around_axis(right(), angle) * forward).normalized();
   }

   void change_yaw(float angle) {
      // YAW: Rotate around world up axis (look left-right)
      forward = (Mat4::rotation_around_axis(up(), angle) * forward).normalized();
   }

   void move(Vec4 step) { position += step; }

   Mat4 get_view_matrix() {
      Mat4 rotation;
      rotation.set_row(0, right());
      rotation.set_row(1, up());
      rotation.set_row(2, forward);
      rotation.set_row(3, {0, 0, 0, 1});

      Mat4 translation = Mat4::identity();
      translation.set_column(3, {-position.x(), -position.y(), -position.z(), 1});

      return rotation * translation;
   }
};

} // namespace fuake
