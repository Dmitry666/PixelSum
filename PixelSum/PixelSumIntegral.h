#pragma once

#include "Common.h"

namespace integral {

 /**
 * Integral image implementation for providing fast region queries from an 8-bit pixel buffer.
 * Note: all coordinates are *inclusive* and clamped internally to the borders
 * of the buffer by the implementation.
 *
 * For example: getPixelSum(4,8,7,10) gets the sum of a 4x3 region where top left
 * corner is located at (4,8) and bottom right at (7,10). In other words
 * all coordinates are _inclusive_.
 * If the resulting region after clamping is empty, the return value for all
 * functions should be 0.
 *
 * The width and height of the buffer dimensions < 4096 x 4096.
 *
 * O(1) solution is made. SAT
 * https://en.wikipedia.org/wiki/Summed-area_table
 *
 * Long preparation and takes more memory.
 * Memory: xWidth * yHeight * (sizeof(uint8) + sizeof(uint32) * 2)
 */
class PIXEL_SUM_API PixelSum
{
public:
	// Contrustors/Destructor
	PixelSum(const unsigned char* buffer, int xWidth, int yHeight);
	~PixelSum();
	PixelSum(const PixelSum& other);
	PixelSum(PixelSum&& other);

	// Operators
	PixelSum& operator=(const PixelSum& other);
	PixelSum& operator=(PixelSum&& other);

	// Methods
	unsigned int getPixelSum(int x0, int y0, int x1, int y1) const;
	double getPixelAverage(int x0, int y0, int x1, int y1) const;

	int getNonZeroCount(int x0, int y0, int x1, int y1) const;
	double getNonZeroAverage(int x0, int y0, int x1, int y1) const;

private:
	void allocateMemory();
	void freeMemory();

private:
	unsigned char* _buffer;
	unsigned int* _summedArea;
	unsigned int* _summedNonZeroArea;

	int _xWidth;
	int _yHeight;
};

} // End integral