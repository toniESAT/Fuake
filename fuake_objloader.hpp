#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "fuake_mesh.hpp"

using namespace std;

namespace fuake {

enum ObjToken {
   kToken_Unknown,        // for not recognized tokens
   kToken_Comment,        // #
   kToken_Vertex,         // v
   kToken_Normal,         // vn
   kToken_Texture,        // vt
   kToken_Face,           // f
   kToken_ObjectName,     // o
   kToken_GroupName,      // g
   kToken_SmoothingGroup, // s
};

ObjToken get_token(const string token) {
   if (token == "#") return kToken_Comment;
   if (token == "v") return kToken_Vertex;
   if (token == "vn") return kToken_Normal;
   if (token == "vt") return kToken_Texture;
   if (token == "f") return kToken_Face;
   if (token == "g") return kToken_GroupName;
   if (token == "s") return kToken_SmoothingGroup;
   return kToken_Unknown;
}

Mesh read_obj(const string filepath) {

   Mesh mesh;
   string buf;
   ObjToken token;

   std::fstream fs(filepath);

   while (fs >> buf) {

      // Check token
      token = get_token(buf);
      
      switch (token) {
      case kToken_GroupName:
      case kToken_SmoothingGroup:
      case kToken_Comment: continue; break;
      case kToken_ObjectName: getline(fs, mesh.name); break;
      case kToken_Vertex:
         for (int i = 0; i < 3; i++) {
            fs >> buf;
            mesh.vertices.push_back(stof(buf));
         }
         break;
      case kToken_Face:
         uint8_t num_vert = 0;
         getline(fs, buf);
         stringstream ss(buf);
         while (ss >> buf) {
            mesh.indices.push_back(stoi(buf) - 1);
            num_vert++;
         }
         mesh.num_vertices.push_back(num_vert);
         break;

         // We're not handling 'vn' and 'vt' tokens for now
         // case kToken_Normal: break;
         // case kToken_Texture: break;
      }
   }
   return mesh;
}

} // namespace fuake
