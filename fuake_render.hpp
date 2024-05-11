
#pragma once

#include <esat/draw.h>
#include <esat/time.h>
#include <esat/input.h>
#include <esat/window.h>

#include <esat_extra/imgui.h>

#include <amath_core.h>
#include <amath_utils.h>

#include <vector>
#include <string>
#include <algorithm>

#include "fuake_mesh.hpp"

using std::string;
using std::vector;
using namespace amath;

namespace fuake {

void draw_mesh_edges(const Mesh &mesh, const Mat4 tr) {

   vector<Vec4> edges = generate_edges(mesh);

   size_t total_edges = edges.size() / 2;

   vector<float> point_z(total_edges * 2);
   vector<float> edge_z(total_edges);
   vector<float> edge_z_sorted(total_edges);

   vector<Vec4> transformed_edges;
   transformed_edges.reserve(total_edges);
   for (size_t i = 0; i < edges.size(); i++) {
      Vec4 transformed_pt = mat_mul(tr, edges[i]);
      point_z[i] = fabs(transformed_pt.z());
      float d = 1.f / transformed_pt.w();
      transformed_edges.push_back(transformed_pt * d);
   }

   float min_z = 99999999999, max_z = 0;
   for (int i = 0; i < edges.size() / 2; i++) {
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
      esat::DrawSetStrokeColor(255 * b, 255 * b, 255 * b);

      esat::DrawLine(edge1.x(), edge1.y(), edge2.x(), edge2.y());
   }
}

void render_mesh_flat(const Mesh &mesh, const Mat4 &model, const Mat4 &view, const Mat4 &persp,
                      const Mat4 &viewport, const Vec4 &light_dir, Vec4 &cam_pos,
                      bool show_normals = false) {

   // Get partial transformation matrices
   Mat4 obj2view = mat_mul(view, model);
   Mat4 view2screen = mat_mul(viewport, persp);

   // Faces to camera space
   vector<Vec4> faces = generate_faces(mesh);
   faces = obj2view.transform_points(faces);

   // Get cam space normals culling and lighting
   vector<Vec4> normals = get_mesh_face_normals(mesh, faces, true);
   // Light to camera space to match cam space normals
   Vec4 tr_light = mat_mul(view, light_dir);

   // Get face centers in cam space for z-ordering
   vector<Vec4> centers = get_mesh_face_centers(mesh, faces);

   // Get screen-space centers if drawing normals
   vector<Vec4> tr_centers;
   if (show_normals) tr_centers = view2screen.transform_points(centers);

   // Transform faces to screen space
   faces = view2screen.transform_points(faces);
   for (auto &pt : faces) pt *= (1.f / pt.w());

   // Z-ordering
   vector<size_t> indices(centers.size());
   for (size_t i = 0; i < centers.size(); i++) indices[i] = i;
   std::sort(indices.begin(), indices.end(), [&centers](int left, int right) {
      return centers[left].z() > centers[right].z();
   });

   // Calculate vertex position offsets to quickly index into them
   auto &num_vertices = mesh.num_vertices;
   vector<size_t> offsets(num_vertices.size(), 0);
   for (size_t i = 1; i < num_vertices.size(); i++)
      offsets[i] = offsets[i - 1] + num_vertices[i - 1];

   // Get viewport resolution for culling (assuming no prior frustum culling)
   Vec4 clip = mat_mul(viewport, Vec4{1, 1, 1, 1});
   float max_x = clip.x();
   float max_y = clip.y();

   for (auto i : indices) {

      // Backface culling
      // if (dot_product((centers[i]).normalized_homogeneous(), normals[i]) > 0) continue;

      size_t offset = offsets[i];
      uint8_t n = num_vertices[i];

      // Viewport culling: Cull for x,y E [-1,1] and z E [0,1]
      vector<Vec2> points(n);
      bool cull = true;
      for (int j = 0; j < n; j++) {
         Vec4 pt = faces[offset + j];
         points[j] = {pt.x(), pt.y()};
         // If all points out, cull
         cull &= pt.x() < 0 || pt.x() > max_x || pt.y() < 0 || pt.y() > max_y || pt.z() < 0 ||
                 pt.z() > 1;
      }
      if (cull) continue;

      float b = dot_product(tr_light, normals[i]);
      uint8_t diffuse = 100;
      uint8_t directional = 255 - diffuse;
      b = max(0, b) * directional + diffuse;

      esat::DrawSetFillColor(b, b, b);
      esat::DrawSetStrokeColor(b, b, b);
      esat::DrawSolidPath((float *)points.data(), points.size(), true);

      if (show_normals) {
         esat::DrawSetStrokeColor(255, 0, 0);
         esat::DrawLine(tr_centers[i].x(),
                        tr_centers[i].y(),
                        (tr_centers[i] + normals[i] * 15).x(),
                        (tr_centers[i] + normals[i] * 15).y());
      }
   }
}

Mat4 get_view_matrix(Vec4 cam_dir, Vec4 cam_pos) {
   Mat4 rotation;
   Vec4 cam_right = cross_product({0, 1, 0, 0}, cam_dir);
   Vec4 cam_up = cross_product(cam_dir, cam_right);
   rotation.set_row(0, cam_right);
   rotation.set_row(1, cam_up);
   rotation.set_row(2, cam_dir);
   rotation.set_row(3, {0, 0, 0, 1});

   Mat4 translation = Mat4::identity();
   translation.set_column(3, {-cam_pos.x(), -cam_pos.y(), -cam_pos.z(), 1});

   return mat_mul(rotation, translation);
}
} // namespace fuake