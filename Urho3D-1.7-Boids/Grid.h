#pragma once
#include "Boids.h"

// Class consideration for Spatial Partitioning

class Grid
{
public:
	Grid();

	~Grid();

	static const int NUM_CELLS = 20;
	static const int CELL_SIZE = 10;

private:
	Boids* cells_[NUM_CELLS][NUM_CELLS];

};

