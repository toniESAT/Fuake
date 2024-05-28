#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#include "fuake_mesh.hpp"
#include <amath_eq.hpp>
#include <amath_geometry.hpp>

using std::vector;

namespace fuake {

vector<Vec3> &sort_points_CW(vector<Vec3> &points);

struct Face {
   vector<size_t> vert_indices;
};

struct QuakeBrush {
   vector<amath::Plane> planes;

   Mesh to_mesh() {

      // Algorithm adapted from:
      // https://merlin3d.wordpress.com/2018/09/10/importing-quake-1-levels-from-map-files/
      // and Michael Abrash

      // Start with a big cube spanning from -4096 to 4096 in each dimension
      vector<uint8_t> num_vertices = {4, 4, 4, 4};

      vector<Vec3> vertices = {{1.000000, 1.000000, -1.000000},
                               {1.000000, -1.000000, -1.000000},
                               {1.000000, 1.000000, 1.000000},
                               {1.000000, -1.000000, 1.000000},
                               {-1.000000, 1.000000, -1.000000},
                               {-1.000000, -1.000000, -1.000000},
                               {-1.000000, 1.000000, 1.000000},
                               {-1.000000, -1.000000, 1.000000}};

      for (auto &v : vertices) v *= 4096;

      size_t face_indexes[6][4] = {
          {1, 5, 7, 3}, {4, 3, 7, 8}, {8, 7, 5, 6}, {6, 2, 4, 8}, {2, 1, 3, 4}, {6, 5, 1, 2}};

      vector<Face> faces;
      for (size_t i = 0; i < 6; i++) faces.emplace_back(face_indexes[i]);

      // Loop over all planes (planes face outwards)
      for (const auto &plane : planes) {
         // C: Clipped face indexes, newly created by intersecting current plane
         vector<size_t> clipped_face;

         // Newly created vertices -> just insert them in mesh.vertices
         // vector<Vec3> new_vertices;

         // Loop over current faces
         // size_t num_faces = mesh.num_vertices.size();

         for (auto &face : faces) {
            // V: Vertices for this face after clipping
            vector<size_t> updated_face;
            u32 num_verts = face.vert_indices.size();

            vector<Vec3> face_coords;
            for (int v = 0; v < num_verts; v++)
               face_coords.push_back(vertices[face.vert_indices[v]]);

            float curdot = dot_product(face_coords[0], plane.normal());
            bool curin = (curdot >= plane.D); // Is this point inside the plane?

            // Loop over vertices of face
            for (int vert_idx = 0; vert_idx < num_verts; vert_idx++) {
               u32 nextvert = (vert_idx + 1) % num_verts;

               // Keep the current vertex if it's inside the plane
               if (curin) updated_face.push_back(face.vert_indices[vert_idx]);

               float nextdot = dot_product(face_coords[nextvert], plane.normal());
               bool nextin = (nextdot >= plane.D);

               // Add clipped vertex if current and next vert on different sides of plane
               if (curin != nextin) {
                  float scale = (plane.D - curdot) / (nextdot - curdot);
                  Vec3 new_vert = face_coords[vert_idx] +
                                  (face_coords[nextvert] - face_coords[vert_idx]) * scale;

                  // After new vertex is created, add it to vertices list and get its index
                  bool is_new_vert = true;
                  size_t new_vert_idx;
                  for (size_t i = 0; i < vertices.size(); i++) {
                     if (vec_equal(new_vert, vertices[i], 1E-5f)) {
                        is_new_vert = false;
                        new_vert_idx = i;
                        break;
                     }
                  }

                  if (is_new_vert) {
                     vertices.push_back(new_vert);
                     new_vert_idx = vertices.size() - 1;
                  }

                  updated_face.push_back(new_vert_idx);
                  clipped_face.push_back(new_vert_idx);
               }

               curdot = nextdot;
               curin = nextin;
            }

            // When we're done clipping this face, replace  vertex indexes with updated ones
            face.vert_indices = updated_face;
         }

         if (!clipped_face.empty()) {
            Face new_face;
            // Loop over new face vertices, some are duplicates, we need to dedup and order CW
            for (size_t vert_idx = 0; vert_idx < clipped_face.size(); vert_idx++) {
               // If vertex not already in new face, insert it in correct winding order
            }
            faces.push_back(new_face);
         }
      }

      // Create mesh from faces and vertices, dropping unused vertices
      vector<size_t> old_indices;
      vector<size_t> new_indices;

      return mesh;
   }
}; // namespace fuake

struct QuakeEntityParam {
   string key;
   string value;

   QuakeEntityParam(string key, string value) : key(key), value(value) {}
};

struct QuakeEntity {
   vector<QuakeEntityParam> properties;
   vector<QuakeBrush> brushes;
};

struct QuakeMap {
   vector<QuakeEntity> entities;

   QuakeMap(const string filepath) {
      bool in_entity = false, in_brush = false;

      std::fstream fs(filepath);
      string line;
      int state = 0; // 0 - root, 1 = in entity, 2 = in brush
      QuakeEntity tmp_entity;
      QuakeBrush tmp_brush;
      vector<Vec3> tmp_pts(3);

      int endkey, startval, endval;

      while (getline(fs, line)) {

         switch (line[0]) {

         case '{':
            if (state == 2) {
               std::cout << "Map parsing error: max depth level should be 2, but 3 reached.\n";
               return;
            }
            if (state == 1) tmp_brush = QuakeBrush();
            if (state == 0) tmp_entity = QuakeEntity();
            state++;
            break;

         case '}':
            if (state == 0) {
               std::cout << "Map parsing error: found '}' token while at root level.\n";
               return;
            }
            if (state == 1) entities.push_back(tmp_entity);
            if (state == 2) tmp_entity.brushes.push_back(tmp_brush);
            state--;
            break;
         case '"':
            if (state != 1) {
               std::cout << "Map parsing error: found entity property at incorrect level.\n";
               return;
            }

            endkey = line.find_first_of('"', 1);
            startval = line.find_first_of('"', endkey + 1);
            endval = line.find_last_of('"');

            tmp_entity.properties.emplace_back(line.substr(0, endkey),
                                               line.substr(startval + 1, endval));
            break;
         case '(':
            if (state != 2) {
               std::cout << "Map parsing error: found brush plane at incorrect level.\n";
               return;
            }

            for (int i = 0, start = 1; i < 3; i++) {
               int end = line.find_first_of(')', start);
               std::stringstream ss(line.substr(start, end));
               ss >> tmp_pts[i].x() >> tmp_pts[i].y() >> tmp_pts[i].z();
               start = line.find_first_of('(', end + 1) + 1;
            }

            tmp_brush.planes.emplace_back(tmp_pts[0], tmp_pts[1], tmp_pts[2]);
            break;
         }
      }
   }
};

} // namespace fuake