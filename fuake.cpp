#include <esat/draw.h>
#include <esat/input.h>
#include <esat/window.h>

#include <amath_core.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "fuake_camera.hpp"
#include "fuake_fpsmeter.hpp"
#include "fuake_gui.hpp"
#include "fuake_lighting.hpp"
#include "fuake_mesh.hpp"
#include "fuake_objloader.hpp"
#include "fuake_render.hpp"
#include "fuake_settings.hpp"

using namespace std;
using namespace fuake;
using namespace amath;

// unsigned char kFPS = 60;

int esat::main(int argc, char **argv) {
  amath::Vec2 window_dims = {1600, 1200};
  WindowInit(window_dims.x(), window_dims.y());
  WindowSetMouseVisibility(true);

  //* Settings
  FuakeSettings settings;

  //* Graphics settings
  RenderContext render_ctxt(window_dims);

  //* Mouse controls
  float mouse_x = (float)esat::MousePositionX();
  float mouse_y = (float)esat::MousePositionY();

  bool gui = false;

  // Initial camera position and direction
  amath::Vec4 cam_position = {0, 0, 0, 1};
  amath::Vec4 cam_forward = {0, 0, 1, 0};
  Camera camera(cam_position, cam_forward);
  Light light(kLightType_Directional, amath::Vec4(1, -1, -1, 0).normalized());

  // FPS meter
  FPSMeter fps_meter(1 << 7);

  int current_group = 0;
  int current_obj = 0;

  Mesh mesh;
  while (WindowIsOpened() && !IsSpecialKeyDown(kSpecialKey_Escape)) {
    //* Load mesh if model changed
    if (settings.model_group != current_group ||
        settings.model_object != current_obj) {
      // if (settings.model_group > 0) settings.exchange_axes = true;
      mesh = read_obj(settings.get_mesh_name(), settings.exchange_axes);
      current_group = settings.model_group;
      current_obj = settings.model_object;
      triangulate(mesh);
    }

    //* Input
    if (esat::IsSpecialKeyDown(kSpecialKey_Tab)) gui ^= true;  // Toggle GUI

    // Cycle rendering mode
    if (esat::IsKeyDown('T'))
      render_ctxt.mode = (RenderMode)((render_ctxt.mode + 1) % 3);
    if (esat::IsKeyDown('Y'))
      render_ctxt.mode = (RenderMode)((render_ctxt.mode + 2) % 3);

    // Camera controls
    float cam_x_rot = 0, cam_y_rot = 0;
    if (esat::IsSpecialKeyPressed(kSpecialKey_Up))
      camera.change_pitch(-settings.cam_sensitivity);
    if (esat::IsSpecialKeyPressed(kSpecialKey_Down))
      camera.change_pitch(settings.cam_sensitivity);
    if (esat::IsSpecialKeyPressed(kSpecialKey_Right))
      camera.change_yaw(-settings.cam_sensitivity);
    if (esat::IsSpecialKeyPressed(kSpecialKey_Left))
      camera.change_yaw(settings.cam_sensitivity);

    if (esat::IsKeyPressed('W'))
      camera.move(camera.forward * settings.cam_speed);
    if (esat::IsKeyPressed('S'))
      camera.move(-camera.forward * settings.cam_speed);
    if (esat::IsKeyPressed('D'))
      camera.move(camera.right() * settings.cam_speed);
    if (esat::IsKeyPressed('A'))
      camera.move(-camera.right() * settings.cam_speed);
    if (esat::IsKeyPressed('Q')) camera.move(camera.up() * settings.cam_speed);
    if (esat::IsKeyPressed('E')) camera.move(-camera.up() * settings.cam_speed);

    float new_mouse_x = (float)esat::MousePositionX();
    float new_mouse_y = (float)esat::MousePositionY();
    float delta_x = (new_mouse_x - mouse_x) / 1000;
    float delta_y = (new_mouse_y - mouse_y) / 1000;
    if (!gui) {
      camera.change_pitch(settings.mouse_sensitivity * delta_y);
      camera.change_yaw(-settings.mouse_sensitivity * delta_x);
    }
    mouse_x = new_mouse_x;
    mouse_y = new_mouse_y;

    //* Render
    amath::Mat4 model = amath::Mat4::transform({0, 0, 5}, {1, 1, 1}, {0, 0, 0});
    amath::Mat4 view = camera.get_view_matrix();

    DrawBegin();
    DrawClear(0, 0, 0);

    switch (render_ctxt.mode) {
      case kRenderMode_Wireframe:
        render_mesh_wireframe(mesh, model, view, render_ctxt);
        break;
      case kRenderMode_Flat:
        render_mesh_flat(mesh, model, view, light.direction, render_ctxt);
        break;
      case kRenderMode_Gouraud:
        render_mesh_smooth(mesh, model, view, light.direction, render_ctxt);
        break;
    }
    // draw_mesh_edges(mesh, tr);

    if (gui) DrawGui(render_ctxt, settings, fps_meter, camera);

    DrawEnd();

    // while ((Time() - last_draw) <= 1000.0 / kFPS) {}
    WindowFrame();

    // FPS meter
    fps_meter.Update();
  }
  WindowDestroy();
  return 0;
}
