#pragma once

#include "tools.hpp"

namespace lbm {
// lbm d2q9 模型
struct LBMD2Q9 {
  // 格子玻尔兹曼方法需要的变量 f(x,v,i)
  float f[9];
  float psi;  // sc模型势函数
  // 有时候可能会需要结合有限差分，保存了格子中粒子的总密度和速度
  float rho;
  Vec2 u;
  LBMD2Q9() : f{}, psi{}, rho{}, u{} {}
  LBMD2Q9(float f0)
      : f{f0, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}, psi{}, rho{f0}, u{} {}

  // 离散速度
  //    6    2     5
  //      ↖ ↑ ↗
  //    3 ←  0  →  1
  //      ↙ ↓ ↘
  //    7    4     8
  constexpr static float c_s = 0.577350269f;
  constexpr static float rc_s = 1.73205081f;
  constexpr static float rc_s2 = 3.f;
  constexpr static float rc_s4 = 9.f;
  constexpr static gmath::Vec2<int> ci[9] = {{0, 0},   {1, 0},  {0, -1},
                                             {-1, 0},  {0, 1},  {1, -1},
                                             {-1, -1}, {-1, 1}, {1, 1}};
  constexpr static gmath::Vec2<float> c[9] = {{0, 0},   {1, 0},  {0, -1},
                                              {-1, 0},  {0, 1},  {1, -1},
                                              {-1, -1}, {-1, 1}, {1, 1}};
  constexpr static float w[9] = {4.f / 9.f,  1.f / 9.f,  1.f / 9.f,
                                 1.f / 9.f,  1.f / 9.f,  1.f / 36.f,
                                 1.f / 36.f, 1.f / 36.f, 1.f / 36.f};
  constexpr static size_t oppo[9] = {0, 3, 4, 1, 2, 7, 8, 5, 6};
};

// lbm d2q4 模型
struct LBMD2Q4 {
  float f[4];
  LBMD2Q4() : f{} {};
  LBMD2Q4(float _f) : f{0.25f * _f, 0.25f * _f, 0.25f * _f, 0.25f * _f} {}
  // 离散速度
  //      1
  //      ↑
  // 2 ←     →  0
  //      ↓
  //      3
  constexpr static float c_s = 0.707106781f;
  constexpr static float rc_s = 1.41421356f;
  constexpr static float rc_s2 = 2.f;
  constexpr static float rc_s4 = 4.f;
  constexpr static gmath::Vec2<int> ci[4] = {
      {1, 0},
      {0, -1},
      {-1, 0},
      {0, 1},
  };
  constexpr static gmath::Vec2<float> c[4] = {
      {1, 0},
      {0, -1},
      {-1, 0},
      {0, 1},
  };
  constexpr static float w = 0.25f;
  constexpr static size_t oppo[4] = {2, 3, 0, 1};
};

// lbm d2q5 模型, 用来保存随流体运动并扩散的物理量
struct LBMD2Q5 {
  float f[5];
  float f_sum;
  LBMD2Q5() : f{}, f_sum{} {};
  LBMD2Q5(float f0) : f{f0, 0.f, 0.f, 0.f, 0.f}, f_sum{f0} {}
  // 离散速度
  //      2
  //      ↑
  // 3 ←  0  →  1
  //      ↓
  //      4
  constexpr static float c_s = 0.577350269f;
  constexpr static float rc_s = 1.73205081f;
  constexpr static float rc_s2 = 3.f;
  constexpr static float rc_s4 = 9.f;
  constexpr static gmath::Vec2<int> ci[5] = {
      {0, 0}, {1, 0}, {0, -1}, {-1, 0}, {0, 1},
  };
  constexpr static gmath::Vec2<float> c[5] = {
      {0, 0}, {1, 0}, {0, -1}, {-1, 0}, {0, 1},
  };
  constexpr static float w[5] = {1.f / 3.f, 1.f / 6.f, 1.f / 6.f, 1.f / 6.f,
                                 1.f / 6.f};
  constexpr static size_t oppo[5] = {0, 3, 4, 1, 2};
};
}  // namespace lbm
