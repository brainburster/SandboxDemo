#pragma once

#include <tuple>

#include "Sandbox_setting.hpp"
#include "buffer_2d.hpp"
#include "lbm_model.hpp"

struct SandboxGrid {
  enum {
    w = SandboxSetting::GlobalSetting::grid_width,
    h = SandboxSetting::GlobalSetting::grid_height,
  };

  // struct Cell {
  //   union {
  //     union {
  //       struct {
  //         uint8_t sub_type : 4;
  //         uint8_t sim_type : 2;
  //         uint8_t _pad : 2;
  //       };
  //       uint8_t cell_type : 6;
  //       uint8_t differentia : 2;
  //     };
  //     uint8_t data0;
  //   };

  //  uint8_t data1;
  //  uint16_t data2;
  //};

  Buffer2D<uint8_t> cell = {};  //固体颗粒 (落沙模型)
  Buffer2D<uint32_t> visualization = {};
  // static SandboxGrid& get_inst() {
  //	static SandboxGrid inst{};
  //	return inst;
  // }

  void init();

  void Set(int x, int y, size_t layer, float r);
};
