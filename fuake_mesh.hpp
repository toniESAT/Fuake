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
#include <iostream>

using std::string;
using std::vector;
using namespace amath;

namespace fuake {

struct Mesh final {
   string name;
   vector<float> vertices;       // Vertex opsitions
   vector<float> normals;        // Vertex normals
   vector<size_t> indices;       // Vertex indeces
   vector<uint8_t> num_vertices; // Num of vertices per face
};

void triangulate(Mesh &mesh) {
   vector<size_t> new_indices;
   vector<uint8_t> new_num_vertices;

   size_t offset = 0;
   for (int i = 0; i < mesh.num_vertices.size(); i++) {
      vector<size_t> sequence(6);
      Vec4 verts[4];

      switch (mesh.num_vertices[i]) {
      case 3:
         new_num_vertices.push_back(mesh.num_vertices[i]);
         for (int i = 0; i < 3; i++) new_indices.push_back(mesh.indices[offset + i]);
         break;
      case 4:
         for (int i = 0; i < 4; i++) {
            verts[i] = {mesh.vertices[3 * (offset + i) + 0],
                        mesh.vertices[3 * (offset + i) + 1],
                        mesh.vertices[3 * (offset + i) + 2],
                        1};
         }

         if ((verts[0] - verts[2]).length() < (verts[1] - verts[2]).length())
            sequence = {0, 1, 3, 1, 2, 3};
         else sequence = {0, 1, 2, 0, 2, 3};

         for (int i = 0; i < 6; i++) new_indices.push_back(mesh.indices[offset + sequence[i]]);

         new_num_vertices.push_back(3);
         new_num_vertices.push_back(3);

         break;
      default:
         std::cout << "Wrong number of vertices " << mesh.num_vertices[i]
                   << ". Only tris and quads are currently supported.";
      }
      offset += mesh.num_vertices[i];
   }
   mesh.indices = new_indices;
   mesh.num_vertices = new_num_vertices;
}

vector<Vec4> generate_edges(const Mesh &mesh) {
   auto &indices = mesh.indices;
   auto &num_vertices = mesh.num_vertices;
   auto &vertices = mesh.vertices;

   size_t total_edges = 0;
   for (auto n : num_vertices) total_edges += n;

   vector<Vec4> edges;
   edges.reserve(total_edges);
   size_t offset = 0, idx_first, idx_second;
   for (size_t n : num_vertices) {
      for (int i = 0; i < n; i++) {

         idx_first = 3 * indices[offset + i];
         idx_second = 3 * indices[offset + ((i + 1) % n)];

         edges.push_back(
             Vec4{vertices[idx_first], vertices[idx_first + 1], vertices[idx_first + 2], 1});
         edges.push_back(
             Vec4{vertices[idx_second], vertices[idx_second + 1], vertices[idx_second + 2], 1});
      }
      offset += n;
   }
   return edges;
}

vector<Vec4> generate_faces(const Mesh &mesh) {
   auto &indices = mesh.indices;
   auto &num_vertices = mesh.num_vertices;
   auto &vertices = mesh.vertices;

   vector<Vec4> faces;
   faces.reserve(num_vertices.size());

   size_t offset = 0;
   for (size_t n : num_vertices) {
      for (int i = 0; i < n; i++) {

         size_t idx = 3 * indices[offset + i];
         faces.push_back(Vec4{vertices[idx], vertices[idx + 1], vertices[idx + 2], 1});
      }
      offset += n;
   }
   return faces;
}

Vec4 get_face_normal(const Vec4 *face, uint8_t num_vertices, bool ccw_winding = false) {

   // Could check for incorrect number of vertices
   // if (face.size() < 3) throw std::runtime_error("Face had less than 3 vertices.");

   Vec4 v1 = face[1] - face[0];
   Vec4 v2 = face[2] - face[0];
   Vec4 n = cross_product(v1, v2);
   n *= (2 * ccw_winding - 1);
   return n.normalized();
}

vector<Vec4> get_mesh_face_normals(Mesh mesh, const vector<Vec4> &faces, bool ccw_winding = false) {

   vector<Vec4> normals;
   normals.reserve(mesh.num_vertices.size());
   size_t offset = 0;
   for (auto n : mesh.num_vertices) {
      normals.push_back(get_face_normal(&faces[offset], n, ccw_winding));
      offset += n;
   }
   return normals;
}

Vec4 get_face_center(const Vec4 *face, uint8_t num_vertices) {

   // if (face.size() < 3) throw std::runtime_error("Face had less than 3 vertices.");
   Vec4 center(0);
   for (int i = 0; i < num_vertices; i++) center = center + face[i];
   return center * (1.f / num_vertices);
}

// * This could be done with SIMD
vector<Vec4> get_mesh_face_centers(Mesh mesh, const vector<Vec4> &faces) {

   vector<Vec4> centers;
   centers.reserve(mesh.num_vertices.size());
   size_t offset = 0;
   for (auto n : mesh.num_vertices) {
      centers.push_back(get_face_center(&faces[offset], n));
      offset += n;
   }
   return centers;
}

} // namespace fuake