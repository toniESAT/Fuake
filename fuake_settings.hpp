
#pragma once

#include <math.h>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>

#include <amath_core.hpp>
#include <amath_utils.hpp>

using std::string;
using std::vector;
using namespace amath;
namespace fs = std::filesystem;

namespace fuake {

const char *MODEL_PATHS[] = {"assets/demo_objects", "assets/quake_objs"};
const char *MODEL_GROUPS[] = {"Demo", "Quake"};

struct FuakeSettings {
   u32 model_group = 0;
   u32 model_object = 1;
   vector<vector<string>> model_names;

   float cam_speed = 0.1;
   float cam_sensitivity = 0.1 / 2 / PI; // 0.016

   float mouse_sensitivity = 1.2;

   bool exchange_axes = true;

   FuakeSettings() {
      model_names.resize(2);
      update_model_list();
   }

   void update_model_list() {
      for (int i = 0; i < 2; i++) {
         model_names[i].clear();
         for (auto &entry : fs::directory_iterator(MODEL_PATHS[i])) {
            string path = entry.path().string();
            std::replace(path.begin(), path.end(), '\\', '/');
            model_names[i].push_back(path);
         }
      }
   }

   string get_mesh_name() { return model_names[model_group][model_object]; }
};
}; // namespace fuake