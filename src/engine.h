#pragma once

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include "./unit.h"

using namespace olc;

namespace grot {

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

struct Camera {
  float speed_px_per_s = 30;
  vf2d position;
  float look_ahead;
};

struct Engine : olc::PixelGameEngine {
  /* Graphics */
  std::vector<vi2d> offsets;
  Camera camera;

  Engine() { sAppName = "Grotesque"; }

  vi2d get_mouse_world_pos() { return GetMousePos() + camera.position; }

  vf2d get_mouse_pos_normalized() {
    auto pos = GetMousePos();
    return {(float)pos.x / (float)ScreenWidth(),
            (float)pos.y / (float)ScreenHeight()};
  }

  vi2d mouse_pos_world() { return GetMousePos() + camera.position; }

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

  virtual void update(float dt) {}

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

  void draw_unit_path_to(Unit &unit, vf2d target_pos, Pixel color = GREY,
                         uint32_t pattern = 0xFF00FF00) {
    auto pos = unit.position;

    pattern = wrap_pattern(pattern, 4.);
    draw_circle(target_pos, unit.kind->radius * 1.2, color, pattern);
    draw_line(target_pos, pos, color, pattern);
  }

  vi2d project(vi2d vec) {
    for (auto v : offsets)
      vec += v;
    return vec;
  }
};

} // namespace grot
