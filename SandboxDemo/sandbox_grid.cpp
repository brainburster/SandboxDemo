#include "Sandbox_grid.h"

void SandboxGrid::init() {
  visualization.ReSize(w, h);
  cell.ReSize(w, h);
}

void SandboxGrid::Set(int x, int y, size_t layer, float r) {
  if (x < 0 || y < 0 || x >= w || y >= h) {
    return;
  }
#define SET_LAYER(layer, v) \
  do {                      \
    layer.Set(x, y, v);     \
  } while (0)

  using brs = SandboxSetting::GlobalSetting::E_BrushType;
  using sld = SandboxSetting::ECell;
  static uint8_t count = 0;

  if (layer == brs::EB_eraser) {
    SET_LAYER(cell, {});
  } else if (!cell.Get(x, y)) {
    const uint8_t diff = ((((uint8_t)(r * 5.f) + count++) & 3) << 6);
    // lbm::LBMD2Q9 l = {};
    switch (layer) {
      case brs::EB_wall: {
        SET_LAYER(cell, sld::ECellwall);
      } break;

      case brs::EB_sand: {
        SET_LAYER(cell, sld::ECellsand | diff);
      } break;
      case brs::EB_stone: {
        SET_LAYER(cell, sld::ECellstone | diff);
      } break;
      case brs::EB_salt: {
        SET_LAYER(cell, sld::ECellsalt | diff);
      } break;
      case brs::EB_suger: {
        SET_LAYER(cell, sld::ECellsugar | diff);
      } break;
      case brs::EB_water: {
        SET_LAYER(cell, sld::ECellWater | diff);
      } break;
      case brs::EB_oil: {
        SET_LAYER(cell, sld::ECelloil | diff);
      } break;
      default:
        break;
    }
  }
#undef SET_LAYER
}
