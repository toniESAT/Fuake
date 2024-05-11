#include <esat/draw.h>
#include <esat/time.h>
#include <esat/input.h>
#include <esat/window.h>

#include <amath_core.h>

#include <vector>
#include <string>
#include <iostream>

#include "fuake_objloader.hpp"
#include "fuake_mesh.hpp"
#include "fuake_render.hpp"

// #include <algorithm>

using namespace std;
using namespace fuake;
using namespace amath;

#define PI 3.14159

// unsigned char kFPS = 60;

void Vec4Print(Vec4 v) { printf("Vec4: [%.2f,%.2f, %.2f, %.2f]\n", v.x(), v.y(), v.z(), v.w()); }

Mat4 generate_transform(Vec3 translate = {0, 0, 0}, Vec3 scale = {1, 1, 1}, Vec3 rot = {0, 0, 0}) {

   Mat4 tr = Mat4::identity();
   tr = mat_mul(Mat4::scaling(scale.x(), scale.y(), scale.z()), tr);
   tr = mat_mul(Mat4::rotationX(rot.x()), tr);
   tr = mat_mul(Mat4::rotationY(rot.y()), tr);
   tr = mat_mul(Mat4::rotationZ(rot.z()), tr);
   tr = mat_mul(Mat4::translation(translate.x(), translate.y(), translate.z()), tr);
   return tr;
}

int esat::main(int argc, char **argv) {
   using namespace esat;

   Vec2 window_dims = {1600, 1200};
   WindowInit(window_dims.x(), window_dims.y());
   WindowSetMouseVisibility(true);

   Mesh mesh = read_obj("assets/monkey.obj");
   triangulate(mesh);

   //* Graphics settings
   float fov = PI / 2;
   float aspect = 1600.f / 1200;
   float zNear = 0.5f;
   float zFar = 100.f;

   //* Controls
   float y_rot = 0;
   float x_rot = 0;
   float zoom = 300;
   float wheel_x = esat::MouseWheelY();
   float mouse_x = esat::MousePositionX();
   float mouse_y = esat::MousePositionY();

   Vec4 cam_dir = {0, 0, 1, 0};
   Vec4 cam_pos = {0, 0, 0, 1};

   float speed = 0.1;
   float mouse_sensitivity = 0.01;
   float cam_sensitivity = 10;

   double last_draw = 0;
   while (WindowIsOpened() && !IsSpecialKeyDown(kSpecialKey_Escape)) {

      //* Input

      // Camera controls
      zoom += 25 * (esat::MouseWheelY() - wheel_x);
      wheel_x = esat::MouseWheelY();

      float cam_x_rot = 0, cam_y_rot = 0;
      if (esat::IsSpecialKeyPressed(kSpecialKey_Up)) cam_x_rot = -cam_sensitivity;
      if (esat::IsSpecialKeyPressed(kSpecialKey_Down)) cam_x_rot = cam_sensitivity;
      if (esat::IsSpecialKeyPressed(kSpecialKey_Right)) cam_y_rot = -cam_sensitivity;
      if (esat::IsSpecialKeyPressed(kSpecialKey_Left)) cam_y_rot = cam_sensitivity;

      cam_dir = mat_mul(Mat4::rotationY(cam_y_rot * mouse_sensitivity / 2 / PI), cam_dir);
      cam_dir = mat_mul(Mat4::rotationX(cam_x_rot * mouse_sensitivity / 2 / PI), cam_dir);

      Vec4 cam_right = cross_product({0, 1, 0, 0}, cam_dir);
      Vec4 cam_up = cross_product(cam_dir, cam_right);

      if (esat::IsKeyPressed('W')) cam_pos += (cam_dir.normalized() * speed);
      if (esat::IsKeyPressed('S')) cam_pos -= (cam_dir.normalized() * speed);
      if (esat::IsKeyPressed('D')) cam_pos += (cam_right.normalized() * speed);
      if (esat::IsKeyPressed('A')) cam_pos -= (cam_right.normalized() * speed);

      //* Render
      Vec4 light_dir = {1, -1, -1, 0};
      light_dir = light_dir.normalized();

      // Mat4 model = generate_transform(
      //     {z * mouse_x_rel, z * mouse_y_rel, z}, {5, 5, 5}, {0, (float)(PI + Time() / 500), PI});
      Mat4 model = generate_transform({0, 0, 5}, {1, 1, 1}, {x_rot, y_rot, 0});
      Mat4 view = get_view_matrix(cam_dir, cam_pos);
      Mat4 persp = Mat4::perspective(fov, aspect, zNear, zFar);
      Mat4 viewport = generate_transform({window_dims.x() / 2, window_dims.y() / 2, 0},
                                         {window_dims.x() / 2, window_dims.y() / 2, 1},
                                         {0, 0, 0});

      DrawBegin();
      DrawClear(0, 0, 0);
      // draw_mesh_edges(mesh, tr);
      // render_mesh_flat(mesh, tr, model, light_dir, cam_pos);
      render_mesh_flat(mesh, model, view, persp, viewport, light_dir, cam_pos);

      DrawEnd();

      // while ((Time() - last_draw) <= 1000.0 / kFPS) {}
      WindowFrame();
      last_draw = Time();
   }
   WindowDestroy();
   return 0;
}
