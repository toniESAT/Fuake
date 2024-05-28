#include "fuake_maploader.hpp"
#include <iostream>
int main() {
   fuake::QuakeMap map("assets/quake_maps/DM2.MAP");
   for (auto e : map.entities) {
      for (auto b : e.brushes) {
         for (auto p : b.planes) { p.print(); }
      }
   };
}