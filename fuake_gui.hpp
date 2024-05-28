#pragma once

#include <esat_extra/imgui.h>
#include "fuake_render.hpp"
#include "fuake_settings.hpp"
#include "fuake_utils.hpp"
#include "fuake_fpsmeter.hpp"

namespace fuake {

string join_strings_with_zeros(vector<string> string_sequence) {
   string r;
   for (vector<string>::const_iterator p = string_sequence.begin(); p != string_sequence.end() - 1;
        p++) {
      r += *p;
      r += '\0';
   }
   r += string_sequence.back();
   r += '\0';
   r += '\0';
   return r;
}

void ImGuiSpacer() { ImGui::Dummy(ImVec2(0.0f, 10.0f)); }

void DrawGui(RenderContext &ctxt, FuakeSettings &settings, FPSMeter &fps_meter, Camera &camera) {

   ImGui::Begin("FUAKE RENDERING OPTIONS");

   if (ImGui::TreeNodeEx("Camera info", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Text("Camera position: [%f, %f, %f]",
                  camera.position.x(),
                  camera.position.y(),
                  camera.position.z());
      ImGui::Text("Camera direction: [%f, %f, %f]",
                  camera.forward.x(),
                  camera.forward.y(),
                  camera.forward.z());

      ImGui::TreePop();
   }
   ImGuiSpacer();

   if (ImGui::TreeNodeEx("Rendering info", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Text("FPS: %.2f", fps_meter.fps);
      ImGui::Text("Frame counter: %d", fps_meter.frame_counter);
      ImGui::Text("Window length: %d", fps_meter.window_length);

      ImGui::TreePop();
   }
   ImGuiSpacer();

   if (ImGui::TreeNodeEx("3D model selection", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Combo("Type:", (int *)&settings.model_group, MODEL_GROUPS, 2, 2);

      string list_objects = join_strings_with_zeros(settings.model_names[settings.model_group]);

      ImGui::Combo(
          "Object", (int *)&settings.model_object, list_objects.data(), list_objects.size());
      ImGui::TreePop();
   }
   ImGuiSpacer();

   if (ImGui::TreeNodeEx("Rendering parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
      bool render_mode_changed =
          ImGui::Combo("Shading mode", (int *)&ctxt.mode, RENDER_MODES, 3, 3);
      if (render_mode_changed) printf("Rendering mode changed to %s.\n", RENDER_MODES[ctxt.mode]);

      // float zNear = 0.5f;
      ImGui::DragFloat("zNear", &ctxt.zNear, 0.01f, 0.01f, 1.f, "%.1f");

      // float zFar = 100.f;
      ImGui::DragFloat("zFar", &ctxt.zFar, 100.f, 100.f, 100000.0f, "%.0f");

      // float fov = PI / 2;
      ImGui::DragFloat("FOV", &ctxt.fov_degrees, 1.f, 50.f, 180.f, "%.0f°");

      ImGui::DragFloat("Length of face normals", &ctxt.normal_length, 1.f, 10.f, 500.f, "%.0f");

      ImGui::Checkbox("Anti-clockwise normals", &ctxt.ccw_normals);

      ImGui::Checkbox("Color by depth", &ctxt.color_by_depth);

      ImGui::Checkbox("Show normals", &ctxt.show_normals);

      ImGui::Checkbox("Swap axes (for Quake OBJ files)", &settings.exchange_axes);

      ImGui::TreePop();
   }
   ImGuiSpacer();

   if (ImGui::TreeNodeEx("Visibility parameters", ImGuiTreeNodeFlags_DefaultOpen)) {

      ImGui::Checkbox("Backface culling", &ctxt.backface_culling);
      ImGui::Checkbox("Z sorting", &ctxt.z_sorting);
      ImGui::Checkbox("Viewport culling", &ctxt.viewport_culling);

      ImGui::TreePop();
   }
   ImGuiSpacer();

   if (ImGui::TreeNodeEx("Control settings", ImGuiTreeNodeFlags_DefaultOpen)) {

      ImGui::DragFloat(
          "Camera sensitivity", &settings.cam_sensitivity, 0.001f, 0.001f, 0.100, "%.3f");

      ImGui::DragFloat("Mouse sensitivity", &settings.mouse_sensitivity, 0.01f, 0.01f, 2.f, "%.3f");

      ImGui::DragFloat("Camera speed", &settings.cam_speed, 0.1f, 0.1f, 100, "%.1f");

      ImGui::TreePop();
   }
   ImGuiSpacer();

   ImGui::End();

   ctxt.Update();
}

} // namespace fuake
