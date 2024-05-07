
#include <esat_extra/tiny_obj_loader.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace tinyobj;

// unsigned char kFPS = 60;

struct obj_t {
   vector<shape_t> shapes;
   vector<material_t> materials;
};

obj_t load_obj(const char *filename, const char *mtl_basepath, bool triangulate) {
   obj_t new_obj;
   string err;

   bool r = LoadObj(new_obj.shapes, new_obj.materials, err, filename, mtl_basepath, triangulate);

   if (!r) cout << err;
   
   return new_obj;
}
