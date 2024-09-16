#include <chrono>
#include <memory>

#include "./engine.hpp"
#include "./unit.hpp"

using namespace grot;

struct Game : Engine {
  std::vector<Unit> units;
  int selected_unit = -1;

  void update(float dt) override {
    if (selected_unit >= units.size())
      selected_unit = -1;

    if (selected_unit != -1) {
      auto &unit = units[selected_unit];
      draw_unit_path_to(unit, get_mouse_world_pos());
    }

    auto nearest_idx = -1;
    auto nearest_distance = INFINITY;

    auto p0 = GetMousePos();
    for (size_t i = 0; i < units.size(); i++) {
      auto &unit = units[i];
      draw_circle(unit.position, unit.kind->radius, unit.kind->color);
      if (unit.desired_position && i != selected_unit) {
        draw_unit_path_to(unit, unit.desired_position.value(), DARK_GREY);
      }

      auto p1 = unit.position - camera.position;
      auto d = (p1 - p0).mag2();
      if (d < nearest_distance) {
        nearest_distance = d;
        nearest_idx = i;
      }
    }

    if (GetMouse(1).bPressed) {
      selected_unit = -1;
    }

    if (selected_unit == -1) {
      if (nearest_idx != -1) {
        auto &unit = units[nearest_idx];
        auto radius = unit.kind->hover_radius();
        if (radius * radius > nearest_distance) {
          draw_circle(unit.position, unit.kind->hover_radius(),
                      unit.kind->color * 0.33);
          draw_string(unit.position, unit.kind->name.c_str(), WHITE);
        }

        if (GetMouse(0).bPressed) {
          selected_unit = nearest_idx;
        }
      }
    } else {
      auto &unit = units[selected_unit];
      if (GetMouse(0).bPressed) {
        unit.desired_position = get_mouse_world_pos();
        selected_unit = -1;
      }
    }
  }
};

int main() {
  auto ranger = UnitKind();
  ranger.name = "Ranger";
  ranger.max_hp = 100;
  ranger.radius = 4;
  ranger.color = BLUE;
  auto ranger_kind = std::make_shared<UnitKind>(ranger);

  Game demo;
  demo.units.push_back([&]() {
    Unit unit;
    unit.kind = ranger_kind;
    unit.current_hp = ranger_kind->max_hp;
    return unit;
  }());

  if (demo.Construct(256, 240, 4, 4))
    demo.Start();
  return 0;
}
