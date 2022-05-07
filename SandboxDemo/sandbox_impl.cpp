#include "Sandbox_impl.h"

#include <algorithm>
#include <random>
#include <thread>

#define For3(i, j, k, a, b, c)  \
  for (int i = 0; i < a; ++i)   \
    for (int j = 0; j < b; ++j) \
      for (int k = 0; k < c; ++k)
#define For2(i, j, a, b)      \
  for (int i = 0; i < a; ++i) \
    for (int j = 0; j < b; ++j)
#define For2x2(x, y, offx, offy)         \
  for (int y = offy; y < grid.h; y += 2) \
    for (int x = offx; x < grid.w; x += 2)
#define For3x3(x, y, offx, offy)         \
  for (int y = offy; y < grid.h; y += 3) \
    for (int x = offx; x < grid.w; x += 3)
#define Swap(a, b) \
  do {             \
    auto temp = a; \
    a = b;         \
    b = temp;      \
  } while (0)
#define Swap3(a, b, c) \
  do {                 \
    auto temp = a;     \
    a = b;             \
    b = c;             \
    c = temp;          \
  } while (0)

void SandboxImpl::update_cell() {
  std::mt19937 eng{std::random_device{}()};
  std::uniform_int_distribution<> rng(0, 1);
  std::uniform_int_distribution<> rng100(0, 100);
  std::uniform_int_distribution<> rng10000(0, 10000);

  static constexpr gmath::Vec2<int> off8[8] = {
      {0, 0}, {-1, -1}, {-1, 0}, {0, -1}, {-1, -1}, {0, 0}, {0, -1}, {-1, 0}};
  /*static constexpr*/ gmath::Vec2<int> off9[9] = {{0, 0},   {1, 0},  {0, -1},
                                                   {-1, 0},  {0, 1},  {1, -1},
                                                   {-1, -1}, {-1, 1}, {1, 1}};

  static uint32_t time = 0;

  struct _Rand_urng_from_func {  // wrap rand() as a URNG
    using result_type = unsigned int;
    std::mt19937 *e;
    std::uniform_int_distribution<> *r;

    static result_type(min)() {  // return minimum possible generated value
      return 0;
    }

    static result_type(max)() {  // return maximum possible generated value
      return 100;                // RAND_MAX;
    }

    result_type operator()() {  // invoke rand()
      // return static_cast<result_type>(_CSTD rand());
      return static_cast<result_type>((*r)(*e));
    }
  };

  std::shuffle(off9, off9 + 9, _Rand_urng_from_func{&eng, &rng100});

  using SS = SandboxSetting;
  using cell_t = uint8_t;
  constexpr uint8_t mask = SS::ECellcategory_mask;
  constexpr auto CellType = [](cell_t cell) constexpr {
    return cell & SS::ECellcategory_mask;
  };
  constexpr auto is_solid = [](cell_t cell) constexpr {
    const cell_t cell_type = CellType(cell);
    return cell_type >= SS::ESolidBegin && cell_type < SS::ESolidEnd;
  };
  constexpr auto is_fluid = [](cell_t cell) constexpr {
    const cell_t cell_type = CellType(cell);
    return cell_type >= SS::EFluidBegin && cell_type < SS::EFluidEnd;
  };
  constexpr auto is_wall = [](cell_t cell) constexpr {
    return CellType(cell) == SS::ECellwall;
  };
  constexpr auto is_none = [](cell_t cell) constexpr {
    return CellType(cell) == SS::ECellnone;
  };
  constexpr auto is_not_solid = [](cell_t cell) constexpr {
    return !is_solid(cell) && !is_wall(cell);
  };
  constexpr auto is_void = [](cell_t cell) constexpr {
    return !CellType(cell);
  };
  constexpr auto is_boundary3x3 = [](const decltype(grid) &grid, int i, int j) {
    return (i >= grid.w - 1) || (i < 1) || (j >= grid.h - 1) || (j < 1);
  };
  constexpr auto is_boundary2x2 = [](const decltype(grid) &grid, int i, int j) {
    return (i >= grid.w - 1) || (i < 0) || (j >= grid.h - 1) || (j < 0);
  };

  constexpr auto swap2cell = [](cell_t &cell0, cell_t &cell1) {
    cell_t temp = cell0;
    cell0 = cell1;
    cell1 = temp;
  };
  constexpr auto swap3cell = [](cell_t &cell0, cell_t &cell1, cell_t &cell2) {
    cell_t temp = cell0;
    cell0 = cell1;
    cell1 = cell2;
    cell2 = temp;
  };
  constexpr auto swap4cell = [](cell_t &cell0, cell_t &cell1, cell_t &cell2,
                                cell_t &cell3) {
    cell_t temp = cell0;
    cell0 = cell1;
    cell1 = cell2;
    cell2 = cell3;
    cell3 = temp;
  };
  struct Neighborhood2x2 {
    cell_t c_c, c_r, c_d, c_rd;
  };

  struct Neighborhood3x3 {
    cell_t c_c, c_l, c_r, c_d, c_ld, c_rd, c_u, c_lu, c_ru;
  };

  constexpr auto get_neighber2x2 = [](const decltype(grid) &grid, int i,
                                      int j) {
    return Neighborhood2x2{grid.cell.Get(i, j), grid.cell.Get(i + 1, j),
                           grid.cell.Get(i, j + 1),
                           grid.cell.Get(i + 1, j + 1)};
  };

  constexpr auto set_neighber2x2 = [](decltype(grid) &grid, int i, int j,
                                      const Neighborhood2x2 &n) {
    grid.cell.Set(i, j, n.c_c);
    grid.cell.Set(i + 1, j, n.c_r);
    grid.cell.Set(i, j + 1, n.c_d);
    grid.cell.Set(i + 1, j + 1, n.c_rd);
  };

  constexpr auto get_neighber3x3 = [](const decltype(grid) &grid, int time,
                                      int i, int j) {
    return Neighborhood3x3{
        grid.cell.Get(i, j),         grid.cell.Get(i - 1, j),
        grid.cell.Get(i + 1, j),     grid.cell.Get(i, j + 1),
        grid.cell.Get(i - 1, j + 1), grid.cell.Get(i + 1, j + 1),
        grid.cell.Get(i, j - 1),     grid.cell.Get(i - 1, j - 1),
        grid.cell.Get(i + 1, j - 1)};
  };

  const auto set_neighber3x3 = [](decltype(grid) &grid, int time, int i, int j,
                                  const Neighborhood3x3 &n) {
    grid.cell.Set(i, j, n.c_c);
    grid.cell.Set(i - 1, j, n.c_l);
    grid.cell.Set(i + 1, j, n.c_r);
    grid.cell.Set(i, j + 1, n.c_d);
    grid.cell.Set(i - 1, j + 1, n.c_ld);
    grid.cell.Set(i + 1, j + 1, n.c_rd);
    grid.cell.Set(i, j - 1, n.c_u);
    grid.cell.Set(i - 1, j - 1, n.c_lu);
    grid.cell.Set(i + 1, j - 1, n.c_ru);
  };

  for (size_t t = 0; t < 9; t++) {
    const int offx = off9[t].x;
    const int offy = off9[t].y;
    For3x3(i, j, offx, offy) {
      if (is_boundary3x3(grid, i, j)) {
        continue;
      }
      auto n = get_neighber3x3(grid, time, i, j);

      if ((time /*+ t*/) & 1) {
        Swap(n.c_lu, n.c_ru);
        Swap(n.c_l, n.c_r);
        Swap(n.c_ld, n.c_rd);
      }

      //固体模拟
      if (is_solid(n.c_c)) {
        if (is_not_solid(n.c_d)) {
          swap2cell(n.c_c, n.c_d);
        } else if ((n.c_d & 0b10111111) != SS::ECellstone && is_solid(n.c_d)) {
          if (is_not_solid(n.c_ld)) {
            swap3cell(n.c_c, n.c_ld, n.c_d);
          } else if (is_not_solid(n.c_rd)) {
            swap3cell(n.c_c, n.c_rd, n.c_d);
          }
        }
      } else if (is_fluid(n.c_c)) {  //液体模拟
        if (is_void(n.c_d)) {
          swap2cell(n.c_c, n.c_d);
        } else {
          if (is_void(n.c_r) || is_fluid(n.c_r)) {
            swap2cell(n.c_c, n.c_r);
          } else if (is_void(n.c_l) || is_fluid(n.c_l)) {
            swap2cell(n.c_c, n.c_l);
          }
        }
      }

      if ((time /*+ t*/) & 1) {
        Swap(n.c_lu, n.c_ru);
        Swap(n.c_l, n.c_r);
        Swap(n.c_ld, n.c_rd);
      }
      set_neighber3x3(grid, time, i, j, n);
    }
  }

  // for (size_t t = 0; t < 2; t++) {
  //   const int offx = off8[t].x;
  //   const int offy = off8[t].y;
  //   For2x2(i, j, offx, offy) {
  //     if (is_boundary2x2(grid, i, j)) {
  //       continue;
  //     }
  //     auto n = get_neighber2x2(grid, i, j);
  //     if ((time /*+ t*/) & 4) {
  //       Swap(n.c_c, n.c_r);
  //       Swap(n.c_d, n.c_rd);
  //     }
  //     if (is_void(n.c_r) || is_fluid(n.c_r)) {
  //       swap2cell(n.c_c, n.c_r);
  //     }
  //     //固体模拟
  //     if (is_solid(n.c_c)) {
  //       if (is_not_solid(n.c_d)) {
  //         swap2cell(n.c_c, n.c_d);
  //       } else if ((n.c_d & 0b10111111) != SS::ECellstone) {
  //         if (is_not_solid(n.c_rd)) {
  //           // swap3cell(n.c_rd, n.c_c, n.c_d);
  //           swap3cell(n.c_c, n.c_rd, n.c_d);
  //         }
  //       }
  //     } else if (is_fluid(n.c_c)) {  //液体模拟
  //       if (is_void(n.c_d) || is_fluid(n.c_r)) {
  //         swap2cell(n.c_c, n.c_d);
  //       } else {
  //         if (is_void(n.c_r) || is_fluid(n.c_r)) {
  //           swap2cell(n.c_c, n.c_r);
  //         }
  //       }
  //     }

  //    if ((time /*+ t*/) & 4) {
  //      Swap(n.c_c, n.c_r);
  //      Swap(n.c_d, n.c_rd);
  //    }

  //    set_neighber2x2(grid, i, j, n);
  //  }
  //}
  ++time;
}

void SandboxImpl::update_visualization() {
  static constexpr Color4f difference_color[4] = {
      {.0f, .0f, .0f, .0f},
      {-.05f, -.05f, -.05f, .0f},
      {-.02f, -.02f, -.02f, .0f},
      {.05f, .05f, .05f, .0f},
  };

  for (int j = 0; j < grid.h; j++) {
    for (int i = 0; i < grid.w; i++) {
      using bs = SandboxSetting;
      using bsc = SandboxSetting::ComponentColor;
      using gmath::utility::BlendColor;
      Color4f color = {};
      if (const uint8_t s = grid.cell.Get(i, j);
          (s & SandboxSetting::ECellcategory_mask) > bs::ECellnone) {
        color = bsc::cell_color[std::min(s & SandboxSetting::ECellcategory_mask,
                                         SandboxSetting::ECellnum - 1)];
        color +=
            difference_color[(s & SandboxSetting::ECelldifference_mask) >> 6];
      }
      grid.visualization.Set(i, j, Color4f2ColorUInt32(color));
    }
  }
}

SandboxImpl::SandboxImpl() : grid{} {}

void SandboxImpl::init() { grid.init(); }

void SandboxImpl::update() {
  update_cell();
  update_visualization();
}

void SandboxImpl::set(int x, int y) {
  //...
  float r = fminf(fmaxf(SandboxSetting::GlobalSetting::brush_size() * .5f, 1.f),
                  50.f);  //半径
  for (float j = -r; j <= r; j++) {
    for (float i = -r; i <= r; i++) {
      const float v = r * r - (i * i + j * j);
      if (v > 0) {
        using bg = SandboxSetting::GlobalSetting;
        using b_t = SandboxSetting::GlobalSetting::E_BrushType;
        size_t layer = bg::use_eraser() ? b_t::EB_eraser : bg::current_brush();
        grid.Set((int)std::round(x + i), (int)std::round(y + j), layer, v);
      }
    }
  }
}

SandboxGrid &SandboxImpl::get_grid() { return grid; }
