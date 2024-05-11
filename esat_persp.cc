#include <esat/math.h>
#include <stdio.h>

int main() {
   esat::Mat4 persp = esat::Mat4Projection();

   for (int i = 0; i < 16; i++) {
      if (i % 4 == 0) printf("\n");
      printf("%.2f", persp.d[i]);
   }

   esat::Mat4 translation = esat::Mat4Translate(800, 600, 0);

   for (int i = 0; i < 16; i++) {
      if (i % 4 == 0) printf("\n");
      printf("%.2f", translation.d[i]);
   }
}