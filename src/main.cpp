#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <chrono>
#include <memory>

uint32_t scroll_int_over_time(int32_t min, int32_t max, float period_s) {
  using namespace std::chrono;
  uint32_t ms =
      duration_cast<milliseconds>(system_clock::now().time_since_epoch())
          .count() %
      (uint32_t)(period_s * 1000.);

  float percent = (float)ms / (period_s * 1000.);
  int32_t value = (int32_t)floor((max - min) * percent) + min;
  return value;
}

uint32_t wrap_pattern(uint32_t pattern, float period_s) {
  uint32_t offset = scroll_int_over_time(0, sizeof(uint32_t) * 8, period_s);
  return (pattern >> offset) | (pattern << (8 * sizeof(uint32_t) - offset));
}

using namespace olc;

struct UnitKind {
  std::string name;
  Pixel color;

  float max_hp;
  int radius;

  float hover_radius() {
    float root = sqrt((float)radius);
    return (root + 1) * (root + 1);
  }
};

struct Unit {
  std::shared_ptr<UnitKind> kind;

  vf2d position;
  std::optional<vf2d> desired_position = std::nullopt;
  float current_hp;
};

struct Camera {
  float speed_px_per_s = 30;
  vf2d position;
  float look_ahead;
};

struct Example : public olc::PixelGameEngine {
  /* Units */
  std::vector<Unit> units;
  int selected_unit = -1;

  /* Graphics */
  std::vector<vi2d> offsets;
  Camera camera;

  Example() { sAppName = "Grotesque"; }

  bool OnUserCreate() override { return true; }

  bool OnUserUpdate(float fElapsedTime) override {
    Clear(BLACK);

    auto go_left = GetKey(Key::A).bHeld || GetKey(Key::LEFT).bHeld;
    auto go_right = GetKey(Key::D).bHeld || GetKey(Key::RIGHT).bHeld;
    auto go_up = GetKey(Key::W).bHeld || GetKey(Key::UP).bHeld;
    auto go_down = GetKey(Key::S).bHeld || GetKey(Key::DOWN).bHeld;

    auto move = camera.speed_px_per_s * fElapsedTime;
    if (go_left)
      camera.position.x -= move;
    if (go_right)
      camera.position.x += move;
    if (go_down)
      camera.position.y += move;
    if (go_up)
      camera.position.y -= move;

    push_offset(-camera.position);
    update(fElapsedTime);
    offsets.clear();
    return true;
  }

  vf2d get_mouse_pos_normalized() {
    auto pos = GetMousePos();
    return {(float)pos.x / (float)ScreenWidth(),
            (float)pos.y / (float)ScreenHeight()};
  }

  vi2d mouse_pos_world() { return GetMousePos() + camera.position; }

  void draw_unit_path_to(Unit &unit, vf2d target_pos, Pixel color = GREY,
                         uint32_t pattern = 0xFF00FF00) {
    auto pos = unit.position;

    pattern = wrap_pattern(pattern, 4.);
    draw_circle(target_pos, unit.kind->radius * 1.2, color, pattern);
    draw_line(target_pos, pos, color, pattern);
  }

  vi2d get_mouse_world_pos() { return GetMousePos() + camera.position; }

  void update(float dt) {
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

  void push_offset(vi2d vec) { offsets.push_back(vec); }

  void draw_circle(vi2d pos, int32_t r, Pixel c,
                   uint32_t pattern = 0xFFFFFFFF) {
    DrawCircle(project(pos), r, c, pattern);
  }

  void draw_string(vi2d pos, const char *str, Pixel c = WHITE) {
    DrawString(project(pos), std::string(str), c);
  }

  void draw_line(vi2d a, vi2d b, Pixel color = WHITE,
                 uint32_t pattern = 0xFFFFFFFF) {
    DrawLine(project(a), project(b), color, pattern);
  }

  bool draw(vi2d pos, Pixel color) { return Draw(project(pos), color); }

  vi2d project(vi2d vec) {
    for (auto v : offsets)
      vec += v;
    return vec;
  }
};

int main() {
  auto ranger = UnitKind();
  ranger.name = "Ranger";
  ranger.max_hp = 100;
  ranger.radius = 4;
  ranger.color = BLUE;
  auto ranger_kind = std::make_shared<UnitKind>(ranger);

  Example demo;
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
