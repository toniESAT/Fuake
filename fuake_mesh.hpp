#pragma once

#include <esat/draw.h>
#include <esat/time.h>
#include <esat/input.h>
#include <esat/window.h>

#include <esat_extra/imgui.h>

#include <amath_core.hpp>
#include <amath_utils.hpp>

#include <vector>
#include <string>
#include <iostream>

using std::string;
using std::vector;
using namespace amath;

namespace fuake {

struct Mesh final {
   string name;
   vector<Vec3> vertices;        // Vertex positions (access with vertex_idx)
   vector<Vec3> normals;         // Vertex normals (access with vertex_idx)
   vector<size_t> indices;       // Vertex indeces (access with index_offsets + i)
   vector<uint8_t> num_vertices; // Num of vertices per face (access with face_idx)
   vector<size_t>
       index_offsets; // 1st vertex index per face (access with face_idx, use to access indices)

   // Calculate vertex position offsets to quickly index into them
   void calculate_offsets() {

      index_offsets.resize(num_vertices.size());
      index_offsets[0] = 0;
      for (size_t i = 1; i < num_vertices.size(); i++)
         index_offsets[i] = index_offsets[i - 1] + num_vertices[i - 1];
   }
};

void triangulate(Mesh &mesh) {
   vector<size_t> new_indices;
   vector<uint8_t> new_num_vertices;

   for (int f = 0; f < mesh.num_vertices.size(); f++) {
      vector<size_t> sequence(6);
      Vec3 verts[4];
      size_t offset = mesh.index_offsets[f];
      switch (mesh.num_vertices[f]) {
      case 3:
         // Tris go untouched
         new_num_vertices.push_back(3);
         for (int i = 0; i < 3; i++) new_indices.push_back(mesh.indices[offset + i]);
         break;
      case 4:
         // Get coordinates for each vertex of the quad
         for (int i = 0; i < 4; i++) verts[i] = mesh.vertices[mesh.indices[offset + i]];

         // Split by short diagonal
         if ((verts[0] - verts[2]).length() < (verts[1] - verts[2]).length())
            sequence = {0, 1, 3, 1, 2, 3};
         else sequence = {0, 1, 2, 0, 2, 3};

         for (int i = 0; i < 6; i++) new_indices.push_back(mesh.indices[offset + sequence[i]]);

         new_num_vertices.push_back(3);
         new_num_vertices.push_back(3);

         break;
      default:
         std::cout << "Wrong number of vertices " << mesh.num_vertices[f]
                   << ". Only tris and quads are currently supported.";
         return;
      }
   }
   mesh.indices = new_indices;
   mesh.num_vertices = new_num_vertices;
   mesh.calculate_offsets();
}

vector<Vec4> generate_edges(const Mesh &mesh) {

   size_t total_edges = 0;
   for (auto n : mesh.num_vertices) total_edges += n;

   vector<Vec4> edges;
   edges.reserve(total_edges);

   size_t idx_first, idx_second;

   for (size_t face_idx = 0; face_idx < mesh.num_vertices.size(); face_idx++) {
      size_t offset = mesh.index_offsets[face_idx];
      size_t num_vertices = mesh.num_vertices[face_idx];

      for (int vert_idx = 0; vert_idx < num_vertices; vert_idx++) {

         idx_first = mesh.indices[offset + vert_idx];
         idx_second = mesh.indices[offset + ((vert_idx + 1) % num_vertices)];

         edges.push_back(Vec4{mesh.vertices[idx_first].x(),
                              mesh.vertices[idx_first].y(),
                              mesh.vertices[idx_first].z(),
                              1});
         edges.push_back(Vec4{mesh.vertices[idx_second].x(),
                              mesh.vertices[idx_second].y(),
                              mesh.vertices[idx_second].z(),
                              1});
      }
   }
   return edges;
}

vector<Vec4> generate_faces(const Mesh &mesh) {

   size_t total_edges = 0;
   for (auto n : mesh.num_vertices) total_edges += n;

   vector<Vec4> faces;
   faces.reserve(total_edges);

   for (size_t face_idx = 0; face_idx < mesh.num_vertices.size(); face_idx++) {

      for (int vert_idx = 0; vert_idx < mesh.num_vertices[face_idx]; vert_idx++) {

         size_t idx = mesh.indices[mesh.index_offsets[face_idx] + vert_idx];
         faces.push_back(
             Vec4{mesh.vertices[idx].x(), mesh.vertices[idx].y(), mesh.vertices[idx].z(), 1});
      }
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