#include <esat/draw.h>
#include <esat/time.h>
#include <esat/input.h>
#include <esat/window.h>

#include <esat_extra/imgui.h>

#include <amath_core.h>
#include <amath_utils.h>

#include <vector>
#include <string>
#include <iostream>

#include "fuake_objloader.hpp"

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

   WindowInit(800, 600);
   WindowSetMouseVisibility(true);

   Mesh mesh = read_obj("assets/monkey.obj");

   // rapidobj::Triangulate(monkey);

   auto &indices = mesh.indices;
   auto &num_vertices = mesh.num_vertices;
   auto &vertices = mesh.vertices;

   // Reserve space for edges
   size_t total_edges = 0;
   for (auto n : num_vertices) total_edges += n;
   vector<Vec4> monkey_edges;
   monkey_edges.reserve(total_edges);

   size_t offset = 0, idx_first, idx_second;
   for (size_t n : num_vertices) {
      for (int i = 0; i < n; i++) {

         idx_first = 3 * indices[offset + i];
         idx_second = 3 * indices[offset + ((i + 1) % n)];

         monkey_edges.push_back(
             Vec4{vertices[idx_first], vertices[idx_first + 1], vertices[idx_first + 2], 1});
         monkey_edges.push_back(
             Vec4{vertices[idx_second], vertices[idx_second + 1], vertices[idx_second + 2], 1});
      }
      offset += n;
   }
   vector<float> point_z(total_edges * 2);
   vector<float> edge_z(total_edges);
   vector<float> edge_z_sorted(total_edges);

   double last_draw = 0;
   while (WindowIsOpened() && !IsSpecialKeyDown(kSpecialKey_Escape)) {

      float mouse_x = MousePositionX();
      float mouse_y = MousePositionY();

      float z_pos = 10;

      Mat4 model = generate_transform(
          {z_pos * (mouse_x - 400) / 800.f, z_pos * (mouse_y - 300) / 600.f, z_pos},
          {2, 2, 2},
          {0, (float)(PI + Time() / 500), PI});

      Mat4 m = Mat4::identity();
      m = mat_mul(model, m);
      m = mat_mul(Mat4::perspective(PI / 2, 800.f / 600.f), m);
      m = mat_mul(Mat4::scaling(300, 300, 1), m);
      m = mat_mul(Mat4::translation(mouse_x, mouse_y, 0), m);

      DrawBegin();
      DrawClear(0, 0, 0);

      vector<Vec4> transformed_edges;
      for (size_t i = 0; i < monkey_edges.size(); i++) {
         Vec4 transformed_pt = mat_mul(m, monkey_edges[i]);
         point_z[i] = fabs(transformed_pt.z());
         float d = 1.f / transformed_pt.w();
         transformed_edges.push_back(transformed_pt * d);
      }

      float min_z = 99999999999, max_z = 0;
      for (int i = 0; i < monkey_edges.size() / 2; i++) {
         float z = (point_z[2 * i] + point_z[2 * i + 1]) / 2;
         if (z < min_z) min_z = z;
         if (z > max_z) max_z = z;
         edge_z[i] = z;
      }

      vector<size_t> sort_indices = argsort(edge_z, true);
      vector<Vec4> sorted_edges(transformed_edges.size());
      for (int i = 0; i < edge_z.size(); i++) {
         sorted_edges[2 * i] = transformed_edges[sort_indices[i] * 2];
         sorted_edges[2 * i + 1] = transformed_edges[sort_indices[i] * 2 + 1];
         edge_z_sorted[i] = edge_z[sort_indices[i]];
      }

      for (int i = 0; i < sorted_edges.size() / 2; i++) {

         Vec4 &edge1 = sorted_edges[2 * i];
         Vec4 &edge2 = sorted_edges[2 * i + 1];

         float b = (max_z - edge_z_sorted[i]) / (max_z - min_z);
         DrawSetStrokeColor(255 * b, 255 * b, 255 * b);

         DrawLine(edge1.x(), edge1.y(), edge2.x(), edge2.y());
      }

      DrawEnd();

      // while ((Time() - last_draw) <= 1000.0 / kFPS) {}
      WindowFrame();
      last_draw = Time();
   }
   WindowDestroy();
   return 0;
}
