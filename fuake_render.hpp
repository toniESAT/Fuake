
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

void draw_mesh_faces(const Mesh &mesh, const Mat4 tr, const Mat4 model, Vec4 light_dir, Vec4 cam_pos) {

   vector<Vec4> faces = generate_faces(mesh);

   vector<Vec4> transformed_faces = tr.transform_points(faces);
   for (auto &pt : transformed_faces) pt = pt.normalized_homogeneous();

   // Get normals an polygon centers
   vector<Vec4> normals = get_mesh_face_normals(mesh, faces, true);
   normals = model.transform_points(normals);
   for (auto &pt : normals) pt = pt.normalized_homogeneous();

   vector<Vec4> centers = get_mesh_face_centers(mesh, faces);
   vector<Vec4> tr_centers = tr.transform_points(centers);
   for (auto &pt : tr_centers) pt = pt.normalized_homogeneous();

   centers = model.transform_points(centers);

   // Get centers z
   vector<float> centers_z(centers.size());
   for (int i = 0; i < centers.size(); i++) centers_z[i] = centers[i].z();

   for (auto &pt : centers) pt = pt.normalized_homogeneous();

   auto &num_vertices = mesh.num_vertices;

   vector<size_t> sort_indices = argsort(centers_z, true);
   vector<size_t> offsets(num_vertices.size());
   offsets[0] = 0;
   for (size_t i = 1; i < num_vertices.size(); i++)
      offsets[i] = offsets[i - 1] + num_vertices[i - 1];

   for (auto i : sort_indices) {

      // Backface culling
      if (dot_product((centers[i] - cam_pos).normalized(), normals[i]) > 0) continue;

      size_t offset = offsets[i];
      uint8_t n = num_vertices[i];

      vector<Vec2> points(n);
      for (int j = 0; j < n; j++)
         points[j] = {transformed_faces[offset + j].x(), transformed_faces[offset + j].y()};

      float b = dot_product(light_dir, normals[i]);
      uint8_t diffuse = 100;
      uint8_t directional = 255 - diffuse;
      b = max(0, b) * directional + diffuse;

      esat::DrawSetFillColor(b, b, b);
      esat::DrawSetStrokeColor(b, b, b);
      esat::DrawSolidPath((float *)points.data(), points.size(), true);

      esat::DrawSetStrokeColor(255, 0, 0);
      esat::DrawLine(tr_centers[i].x(),
                     tr_centers[i].y(),
                     (tr_centers[i] + normals[i] * 15).x(),
                     (tr_centers[i] + normals[i] * 15).y());
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