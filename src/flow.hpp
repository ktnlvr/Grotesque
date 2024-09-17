#pragma once

#include "./engine.hpp"

namespace grot {

struct FlowMap {
  FlowMap(vi2d size = vi2d(8, 8), size_t xx = 10000, size_t yy = 10000)
      : size(size), xx(xx), yy(yy) {}

  void debug_draw(Engine &engine) {
    int32_t ii = xx / size.x;
    int32_t jj = yy / size.y;

    auto is_visible = [&](vi2d pos) -> bool {
      return engine.is_inside_camera_aabb(pos) ||
             engine.is_inside_camera_aabb(pos + vi2d{size.x, 0}) ||
             engine.is_inside_camera_aabb(pos + vi2d{0, size.y}) ||
             engine.is_inside_camera_aabb(pos + size);
    };

    vi2d start = engine.camera.position / size;
    for (int32_t i = start.x; i < ii; i++) {
      auto pos = vi2d(i * size.x, 0);

      if (!is_visible(pos)) {
        continue;
      }

      for (int32_t j = start.y; j < jj; j++) {
        pos.y = j * size.y;

        if (!is_visible(pos)) {
          break;
        }

        engine.draw_rect(pos, size, DARK_BLUE * 0.22);
      }
    }
  }

  vi2d size;
  size_t xx, yy;
  float resolution = 1.;
};

} // namespace grot
