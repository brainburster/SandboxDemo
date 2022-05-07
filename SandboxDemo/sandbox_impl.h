#pragma once
#include "Sandbox_grid.h"

class SandboxImpl
{
protected:
	SandboxGrid grid;
	void update_cell();
	void update_visualization();
public:
	SandboxImpl();
	void init();
	void update();
	void set(int x, int y);
	SandboxGrid& get_grid();
};
