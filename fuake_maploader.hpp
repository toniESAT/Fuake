#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>

#include "fuake_mesh.hpp"
#include <amath_eq.h>
#include <amath_geometry.h>

using namespace std;

namespace fuake {

struct QuakeBrush {
   vector<amath::Plane> planes;

   Mesh to_mesh() {
      Mesh mesh;

      // TODO

      return mesh;
   }
};

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
               cout << "Map parsing error: max depth level should be 2, but 3 reached.\n";
               return;
            }
            if (state == 1) tmp_brush = QuakeBrush();
            if (state == 0) tmp_entity = QuakeEntity();
            state++;
            break;

         case '}':
            if (state == 0) {
               cout << "Map parsing error: found '}' token while at root level.\n";
               return;
            }
            if (state == 1) entities.push_back(tmp_entity);
            if (state == 2) tmp_entity.brushes.push_back(tmp_brush);
            state--;
            break;
         case '"':
            if (state != 1) {
               cout << "Map parsing error: found entity property at incorrect level.\n";
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
               cout << "Map parsing error: found brush plane at incorrect level.\n";
               return;
            }

            for (int i = 0, start = 1; i < 3; i++) {
               int end = line.find_first_of(')', start);
               stringstream ss(line.substr(start, end));
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