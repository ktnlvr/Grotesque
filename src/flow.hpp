#pragma once

#include <deque>
#include <set>

#include "./engine.hpp"

namespace grot {

struct Cell {
  int cost;
  char theta;
};

const vi2d MAP_SCALE_FACTOR = {16, 16};
vi2d neighbours[8] = {vi2d(1, 0),   vi2d(0, 1),  vi2d(-1, 0), vi2d(0, -1),
                      vi2d(-1, -1), vi2d(-1, 1), vi2d(1, -1), vi2d(1, 1)};

struct FlowMap {
  FlowMap(size_t xx = 16, size_t yy = 16)
      : xx(xx), yy(yy), cells(xx * yy), max_cost(INT32_MIN) {}

  void debug_draw(Engine &engine) {
    int32_t ii = xx;
    int32_t jj = yy;

    auto is_visible = [&](vi2d pos) -> bool {
      return engine.is_inside_camera_aabb(pos) ||
             engine.is_inside_camera_aabb(pos + vi2d{MAP_SCALE_FACTOR.x, 0}) ||
             engine.is_inside_camera_aabb(pos + vi2d{0, MAP_SCALE_FACTOR.y}) ||
             engine.is_inside_camera_aabb(pos + MAP_SCALE_FACTOR);
    };

    vi2d start = engine.camera.position / MAP_SCALE_FACTOR;
    start.x = std::max(0, start.x);
    start.y = std::max(0, start.y);
    for (int32_t i = start.x; i < ii; i++) {
      auto pos = vi2d(i * MAP_SCALE_FACTOR.x, 0);

      if (!is_visible(pos)) {
        continue;
      }

      for (int32_t j = start.y; j < jj; j++) {
        pos.y = j * MAP_SCALE_FACTOR.y;

        if (!is_visible(pos)) {
          break;
        }

        auto &cell = cells[i * xx + j];

        float p = (float)cell.cost / (float)max_cost;
        auto color = DARK_RED * sqrt(p) + GREEN * (1 - sqrt(p));

        engine.draw_rect(pos, MAP_SCALE_FACTOR, color);
        engine.draw_line(pos + MAP_SCALE_FACTOR / 2,
                         pos + MAP_SCALE_FACTOR / 2 +
                             vf2d(MAP_SCALE_FACTOR.mag() / 2,
                                  (2 * 3.14) * (float)cell.theta / 255.)
                                 .cart(),
                         color);
      }
    }
  }

  std::optional<std::reference_wrapper<Cell>> get_cell(vi2d cell) {
    if (cell.x < 0 || cell.x >= xx || cell.y >= yy || cell.y < 0)
      return std::nullopt;
    return cells[cell.x * xx + cell.y];
  }

  void calculate_flow(vi2d target) {
    for (auto &cell : cells) {
      cell.cost = INT32_MAX;
    }

    int32_t max = INT32_MIN;
    std::set<vi2d> visited;
    std::deque<std::pair<vi2d, int32_t>> queue = {{target, 0}};
    while (queue.size() > 0) {
      auto [next, cost] = std::move(queue.front());
      queue.pop_front();
      if (visited.contains(next))
        continue;

      auto maybe_cell = get_cell(next);
      if (!maybe_cell)
        continue;

      auto &cell = maybe_cell.value().get();
      cell.cost = std::min(cost, cell.cost);
      max = std::max(max, cell.cost);
      for (auto n : neighbours) {
        queue.push_back(std::make_pair(next + n, cost + 1));
      }

      visited.insert(next);
    }

    max_cost = max;

    for (int32_t i = 0; i < xx; i++) {
      for (int32_t j = 0; j < yy; j++) {
        vf2d convolved = {};

        auto &self = cells[i * xx + j];
        for (auto n : neighbours) {
          auto p = vi2d(i, j) + n;
          auto maybe_cell = get_cell(p);
          if (!maybe_cell)
            continue;
          auto &cell = maybe_cell.value().get();
          convolved += (vf2d)n.norm() * (self.cost - cell.cost);
        }

        auto theta = round(255 * convolved.polar().y / (2 * 3.14159265359));
        self.theta = theta;
      }
    }
  }

  size_t xx, yy;
  int32_t max_cost = INT32_MIN;
  std::vector<Cell> cells;
};

} // namespace grot
