
#pragma once

#include <esat/draw.h>
#include <esat/time.h>
#include <esat/input.h>
#include <esat/window.h>
#include <esat/sprite.h>

#include <esat_extra/imgui.h>

#include <amath_core.hpp>
#include <amath_utils.hpp>

#include <vector>
#include <string>
#include <algorithm>

#include "fuake_mesh.hpp"
#include "fuake_framebuffer.hpp"

using std::string;
using std::vector;
using namespace amath;

namespace fuake {

enum RenderMode {
   kRenderMode_Wireframe,
   kRenderMode_Flat,
   kRenderMode_Gouraud,
};

const char *RENDER_MODES[] = {
    "Wireframe",
    "Flat",
    "Gouraud",
};

struct RenderContext {

   RenderMode mode = kRenderMode_Flat;

   bool show_normals = false;
   bool z_sorting = true;
   bool backface_culling = true;
   bool viewport_culling = true;
   bool color_by_depth = true;
   bool ccw_normals = true;

   float fov_degrees = Rad2Deg(PI / 2);
   float fov = PI / 2;
   float zNear = 0.01f;
   float zFar = 20000.f;
   float aspect;
   Vec2 window_dimensions;

   float normal_length = 100;

   Mat4 persp;
   Mat4 viewport;

   RenderContext(Vec2 window_dimensions)
       : aspect(window_dimensions.x() / window_dimensions.y()),
         window_dimensions(window_dimensions) {
      persp = Mat4::perspective(fov, aspect, zNear, zFar);
      viewport = Mat4::transform({window_dimensions.x() / 2, window_dimensions.y() / 2, 0},
                                 {window_dimensions.x() / 2, window_dimensions.y() / 2, 1},
                                 {0, 0, 0});
   }

   void Update() {
      fov = Deg2Rad(fov_degrees);
      UpdateMatrices();
   }

   void UpdateMatrices() {
      persp = Mat4::perspective(fov, aspect, zNear, zFar);

      // Viewport won't change
      // viewport = generate_transform({window_dimensions.x() / 2, window_dimensions.y() / 2, 0},
      //                               {window_dimensions.x() / 2, window_dimensions.y() / 2, 1},
      //                               {0, 0, 0});
   }
};

void render_mesh_wireframe(const Mesh &mesh, const Mat4 &model, const Mat4 &view,
                           RenderContext context) {

   Mat4 tr = mat_concat({model, view, context.persp, context.viewport});

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
   if (context.color_by_depth) {
      for (size_t i = 0; i < edges.size() / 2; i++) {
         float z = (point_z[2 * i] + point_z[2 * i + 1]) / 2;
         if (z < min_z) min_z = z;
         if (z > max_z) max_z = z;
         edge_z[i] = z;
      }
   }

   vector<size_t> sort_indices(edge_z.size());
   if (context.z_sorting) sort_indices = argsort(edge_z, true);
   else
      for (size_t i = 0; i < edge_z.size(); i++) sort_indices[i] = i;

   float max_x = context.window_dimensions.x();
   float max_y = context.window_dimensions.y();

   for (int i = 0; i < transformed_edges.size() / 2; i++) {
      size_t idx = sort_indices[i];

      Vec4 &pt1 = transformed_edges[2 * idx];
      Vec4 &pt2 = transformed_edges[2 * idx + 1];

      // Viewport culling: Check for x,y ∈ [-1,1] and z ∈ [0,1]
      bool edge_out = (pt1.x() < 0 || pt1.x() > max_x || pt1.y() < 0 || pt1.y() > max_y ||
                       pt1.z() < 0 || pt1.z() > 1) &&
                      (pt2.x() < 0 || pt2.x() > max_x || pt2.y() < 0 || pt2.y() > max_y ||
                       pt2.z() < 0 || pt2.z() > 1);

      if (context.viewport_culling && edge_out) continue;

      float b = context.color_by_depth ? (max_z - edge_z[idx]) / (max_z - min_z) : 1;
      b *= 255;
      esat::DrawSetStrokeColor(b, b, b);

      esat::DrawLine(pt1.x(), pt1.y(), pt2.x(), pt2.y());
   }
}

void render_mesh_flat(const Mesh &mesh, const Mat4 &model, const Mat4 &view, const Vec4 &light_dir,
                      RenderContext context) {

   // Get partial transformation matrices
   Mat4 obj2view = view * model;
   Mat4 view2screen = context.viewport * context.persp;

   // Faces to camera space
   vector<Vec4> faces = generate_faces(mesh);

   // TODO: Frustum clipping 

   faces = obj2view.transform_points(faces);

   // Get cam space normals for backface culling and lighting
   vector<Vec4> normals = get_mesh_face_normals(mesh, faces, context.ccw_normals);
   // Light to camera space to match cam space normals
   Vec4 tr_light = view * light_dir;

   // Get face centers in cam/view space for z-ordering
   vector<Vec4> centers = get_mesh_face_centers(mesh, faces);

   // Scaling brightness by depth
   float min_z = 99999999999, max_z = 0;
   if (context.color_by_depth) {
      for (int i = 0; i < centers.size(); i++) {
         if (centers[i].z() < min_z) min_z = centers[i].z();
         if (centers[i].z() > max_z) max_z = centers[i].z();
      }
   }
   min_z = max(min_z, 0);

   // Get screen-space centers if drawing normals
   vector<Vec4> tr_centers;
   vector<Vec4> tr_normals;
   if (context.show_normals) {
      tr_centers = view2screen.transform_points(centers);
      tr_normals = view2screen.transform_points(normals);
      for (auto &pt : tr_centers) pt *= (1.f / pt.w());
      for (auto &pt : tr_normals) pt *= (1.f / pt.w());
   }

   // Transform faces to screen space
   faces = view2screen.transform_points(faces);
   for (auto &pt : faces) pt *= (1.f / pt.w());

   // Z-sorting of faces
   vector<size_t> indices(centers.size());
   for (size_t i = 0; i < centers.size(); i++) indices[i] = i;

   if (context.z_sorting)
      std::sort(indices.begin(), indices.end(), [&centers](size_t left, size_t right) {
         return centers[left].z() > centers[right].z();
      });

   auto &num_vertices = mesh.num_vertices;

   // Get viewport resolution for culling (assuming no prior frustum culling)
   float max_x = context.window_dimensions.x();
   float max_y = context.window_dimensions.y();

   for (auto i : indices) {

      // NOTE: this could be done in screen space
      // Backface culling
      if (context.backface_culling && dot_product(centers[i], normals[i]) > 0) continue;

      size_t offset = mesh.index_offsets[i];

      // TODO: this would be better as frustum culling or even clipping
      // Get x, y coordinates for DrawSolidPath + viewport culling
      vector<Vec2> points(num_vertices[i]);
      bool cull_xy = true, cull_z = false; 
      for (size_t n = 0; n < num_vertices[i]; n++) {
         Vec4 pt = faces[offset + n];
         points[n] = {pt.x(), pt.y()};
         // Viewport culling XY: x,y ∈ [-1,1] for any point
         bool point_out = (pt.x() < 0 || pt.x() > max_x || pt.y() < 0 || pt.y() > max_y);
         cull_xy = cull_xy && point_out;

         // Viewport culling Z: z ∈ [0,1] for all points
         cull_z = cull_z || (pt.z() < 0 || pt.z() > 1);
      }
      if (context.viewport_culling && (cull_xy || cull_z)) continue;

      float b = dot_product(tr_light, normals[i]);
      uint8_t diffuse = 100;
      uint8_t directional = 255 - diffuse;
      b = max(0, b) * directional + diffuse;

      float depth_multiplier =
          context.color_by_depth ? (max_z - centers[i].z()) / (max_z - min_z) : 1;
      //  context.color_by_depth ? (context.zFar - centers[i].z()) / (context.zFar - context.zNear)
      //  : 1;

      b *= depth_multiplier;

      esat::DrawSetFillColor(b, b, b);
      esat::DrawSetStrokeColor(b, b, b);
      esat::DrawSolidPath((float *)points.data(), points.size(), true);

      if (context.show_normals) {
         esat::DrawSetStrokeColor(255, 0, 0);
         esat::DrawLine(
             tr_centers[i].x(),
             tr_centers[i].y(),
             // (tr_centers[i] + normals[i] * context.normal_length / centers[i].z()).x(),
             // (tr_centers[i] + normals[i] * context.normal_length / centers[i].z()).y());
             (tr_centers[i] + normals[i] * context.normal_length).x(),
             (tr_centers[i] + normals[i] * context.normal_length).y());
      }
   }
}

void render_mesh_smooth(const Mesh &mesh, const Mat4 &model, const Mat4 &view,
                        const Vec4 &light_dir, RenderContext context) {

   // Initialize frame buffer and render texture
   static FrameBufferMono framebuffer(context.window_dimensions);

   static esat::SpriteHandle render_texture = esat::SpriteFromMemory(
       context.window_dimensions.x(), context.window_dimensions.y(), framebuffer.asRGB().data());

   static esat::SpriteTransform tr = {0, 0, 0, 1, 1, 0, 0};

   // SPAN RASTERIZATION
   // Sort edges by scan line

   // Iterate over scan lines
   for (int i = 0; i < context.window_dimensions.y(); i++) { /* code */
   }

   // Update texture and draw

   esat::SpriteUpdateFromMemory(render_texture, framebuffer.asRGB().data());
   esat::DrawSprite(render_texture, tr);
}

void rasterize_triangle(vector<Vec2> pts, FrameBufferMono fb) {
   if (pts.size() != 3) {
      printf("WARNING: Non-triangular shaped passed to rasterize_triangle");
      return;
   }

   // Caculate bounding box
   u32 max_x = 0, min_x = UINT32_MAX, max_y = 0, min_y = UINT32_MAX;
}

} // namespace fuake