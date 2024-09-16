#pragma once

#include "olcPixelGameEngine.h"

using namespace olc;

namespace grot {

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

}
