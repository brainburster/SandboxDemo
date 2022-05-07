#pragma once

#include "tools.hpp"

struct SandboxSetting {
  //标准温度 0℃ 273.15K
  constexpr static float T0 = 273.15f;

  //全局设置
  struct GlobalSetting {
    enum {
      width = 800,
      height = 600,
      grid_size = 6,
      grid_width = width / grid_size,
      grid_height = height / grid_size,
    };

    enum E_BrushType {
      EB_wall = 0,
      EB_eraser = 1,
      EB_water = 2,
      EB_oil = 3,
      EB_air = 4,
      EB_sand = 5,
      EB_stone = 6,
      EB_salt = 7,
      EB_suger = 8,
      EB_warming = 9,
      EB_cooling = 10,
    };

    //重力加速度
    static Vec2& g() {
      static Vec2 g = {0.f, 0.01f};
      return g;
    }
    //背景温度
    static float& T() {
      static float T = T0 + 20.f;
      return T;
    }
    //当前笔刷
    static int& current_brush() {
      static int b = 0;
      return b;
    }
    //临时切换为橡皮
    static bool& use_eraser() {
      static bool e = 0;
      return e;
    }
    //笔刷大小
    static float& brush_size() {
      static float bsize = 10.f;
      return bsize;
    }

    static int& show_velocity() {
      static int sv = 0;
      return sv;
    }
  };

  ////流体
  // enum E_Fluid
  //{
  //	EFluid_water = 0,
  //	EFluid_oil = 1,
  //	EFluid_air = 2, //主要是氮气
  //	EFluid_num = 3,
  // };

  ////溶质
  // enum E_Solute
  //{
  //	ESolute_H_p = 0,			//氢离子
  //	ESolute_OH_p = 1,			//氢氧根离子
  //	ESolute_Na_p = 2,			//钠离子
  //	ESolute_Cl_n = 3,			//氯离子
  //	ESolute_sugar = 4,			//糖
  //	ESolute_num = 5,
  // };

  //固体（粉末）
  enum ECell : uint8_t {
    ECellnone = 0,
    ECellwall = 1,
    ESolidBegin = 2,
    ECellice = 2,
    ECellsnow = 3,
    ECellsalt = 4,
    ECellsugar = 5,
    ECellsand = 6,
    ECellstone = 7,
    ESolidEnd = 8,
    EFluidBegin = 8,
    ECellWater = 8,
    ECelloil = 9,
    EFluidEnd = 10,
    ECellnum = 10,
    ECellcategory_mask = 0b00111111,
    ECelldifference_mask = 0b11000000,
  };

  // enum ECell : uint8_t {
  //   ECellnone = 0,
  //   ECellwall = 1,     //墙
  //   ECellFall = 2,     //下落
  //   ECellCollaps = 3,  //坍塌
  //   ECellLiquid = 4,   //液体
  //   ECellGas = 5,      //气体
  //   ECellSolute = 6,   //溶质
  // };

  /* struct Cell {
     uint8_t cell_category;
     uint8_t differentia;
     uint16_t data_uint16;
     float temperature;
     float data_float;
     uint32_t data_uint32;
   };*/

  struct ComponentColor {
    constexpr static Color4f bg = {.8f, .8f, .8f, 1.f};
    constexpr static Color4f air = {.96f, .91f, .78f, 0.1f};
    constexpr static Color4f water = {0.f, .67f, 1.f, .7f};
    constexpr static Color4f oil = {1.f, .82f, .28f, .8f};
    constexpr static Color4f Na = {.91f, .91f, .85f, 0.4f};
    constexpr static Color4f Cl = {.82f, .91f, .48f, 0.4f};
    constexpr static Color4f sugar = {1.f, .82f, .77f, 0.4f};
    constexpr static Color4f cell_color[ECellnum] = {
        {0.f},                    // none
        {.2f, .2f, .2f, 1.f},     // wall
        {.8f, .8f, .97f, 0.6f},   // ice
        {.97f, .97f, .97f, 1.f},  // snow
        {.9f, .9f, .9f, 1.f},     // salt
        {1.f, .82f, .77f, 0.4f},  // suagr
        {.84f, .77f, .7f, 1.f},   // sand
        {.36f, .36f, .36f, 1.f},  // stone
        {0.1f, .67f, 1.f, .7f},   // water
        {1.f, .82f, .28f, .8f},   // oil
    };
  };
};
