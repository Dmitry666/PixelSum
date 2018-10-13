#include "PixelSumIntegral.h"

#include <string.h>		// memcpy
#include <stdlib.h>		// malloc, free, rand
#include <assert.h>
#include <algorithm>	// min, max, clamp

#include "Utils.h"

#include "SSE.h"

namespace integral {

void fillSummedArea(const unsigned char* buffer, unsigned int* summedArea, unsigned int* summedNonZeroArea, int xWidth, int yHeight)
{
	// First line
	{
		unsigned int sumLine = 0;
		unsigned int zeroSumLine = 0;

		for (int x = 0; x < xWidth; ++x)
		{
			unsigned char value = buffer[x];

			// SA(x, y) = B(x, y) + SA(x - 1, y)
			sumLine += value;
			zeroSumLine += (value > 0 ? 1 : 0);

			summedArea[x] = sumLine;
			summedNonZeroArea[x] = zeroSumLine;
		}
	}

	// Others
	for (int y = 1; y < yHeight; ++y)
	{
		const auto src = buffer + y * xWidth;

		const auto prevSum = summedArea + (y - 1) * xWidth;
		auto sum = summedArea + y * xWidth;

		const auto prevZeroSum = summedNonZeroArea + (y - 1) * xWidth;
		auto zeroSum = summedNonZeroArea + y * xWidth;

		unsigned int sumLine = 0;
		unsigned int zeroSumLine = 0;

		// Set line's values
		for (int x = 0; x < xWidth; ++x)
		{
			unsigned char value = src[x];

			// SA(x, y) = B(x, y) + SA(x - 1, y) + SA(x, y - 1)
			unsigned int currentSumValue = value + sumLine;
			unsigned int currentZeroSumValue = zeroSum[x] = (value > 0 ? 1 : 0) + zeroSumLine;

			sum[x] = currentSumValue + prevSum[x];
			zeroSum[x] = currentZeroSumValue + prevZeroSum[x];

			// Save 
			sumLine = currentSumValue;
			zeroSumLine = currentZeroSumValue;
		}
	}
}

PixelSum::PixelSum(const unsigned char* buffer, int xWidth, int yHeight)
	: _xWidth(xWidth)
	, _yHeight(yHeight)
{
	assert(buffer != nullptr);
	assert(xWidth > 0 && yHeight > 0);
	assert(xWidth * yHeight <= 4096 * 4096); // 4096 * 4096 * 256 == max(uint32)

	allocateMemory();

	// Copy
	memcpy(_buffer, buffer, _xWidth * _yHeight * sizeof(unsigned char));

	// TODO. Use SSE2 and more and to optimize fillSummedArea
	fillSummedArea(buffer, _summedArea, _summedNonZeroArea, _xWidth, yHeight);
}

PixelSum::~PixelSum()
{
	freeMemory();
}

PixelSum::PixelSum(const PixelSum& other)
	: _xWidth(other._xWidth)
	, _yHeight(other._yHeight)
{
	allocateMemory();

	// Copy data
	memcpy(_buffer, other._buffer, _xWidth * _yHeight * sizeof(unsigned char));
	memcpy(_summedArea, other._summedArea, _xWidth * _yHeight * sizeof(unsigned int));
	memcpy(_summedNonZeroArea, other._summedNonZeroArea, _xWidth * _yHeight * sizeof(unsigned int));
}

PixelSum::PixelSum(PixelSum&& other)
	: _xWidth(other._xWidth)
	, _yHeight(other._yHeight)
{
	// Move
	_buffer = other._buffer;
	other._buffer = nullptr;

	_summedArea = other._summedArea;
	other._summedArea = nullptr;

	_summedNonZeroArea = other._summedNonZeroArea;
	other._summedNonZeroArea = nullptr;
}

PixelSum& PixelSum::operator=(const PixelSum& other)
{
	assert(&other != this);

	// Free
	freeMemory();

	// copy
	_xWidth = other._xWidth;
	_yHeight = other._yHeight;

	allocateMemory();

	memcpy(_buffer, other._buffer, _xWidth * _yHeight * sizeof(unsigned char));
	memcpy(_summedArea, other._summedArea, _xWidth * _yHeight * sizeof(unsigned int));
	memcpy(_summedNonZeroArea, other._summedNonZeroArea, _xWidth * _yHeight * sizeof(unsigned int));

	return *this;
}

PixelSum& PixelSum::operator=(PixelSum&& other)
{
	assert(&other != this);

	// Free
	freeMemory();

	// Move
	_xWidth = other._xWidth;
	_yHeight = other._yHeight;

	_buffer = other._buffer;
	other._buffer = nullptr;

	_summedArea = other._summedArea;
	other._summedArea = nullptr;

	_summedNonZeroArea = other._summedNonZeroArea;
	other._summedNonZeroArea = nullptr;

	return *this;
}

unsigned int PixelSum::getPixelSum(int x0, int y0, int x1, int y1) const
{
	// Prepare
	auto rect = utils::Rect(x0, y0, x1, y1)
		.normalized()
		.intersected(0, 0, _xWidth - 1, _yHeight - 1);

	int minX = rect.x0;
	int minY = rect.y0;
	int maxX = rect.x1;
	int maxY = rect.y1;

	// Calculate
	unsigned int B = (minX > 0 && minY > 0) ? _summedArea[(minX - 1) + (minY - 1) * _xWidth] : 0;
	unsigned int C = minY > 0 ? _summedArea[maxX + (minY - 1) * _xWidth] : 0;

	unsigned int A = _summedArea[maxX + maxY * _xWidth];
	unsigned int D = minX > 0 ? _summedArea[(minX - 1) + maxY * _xWidth] : 0;

	// https://en.wikipedia.org/wiki/Summed-area_table
	return A + B - C - D;
}

double PixelSum::getPixelAverage(int x0, int y0, int x1, int y1) const
{
	// Calculate
	unsigned int sum = getPixelSum(x0, y0, x1, y1);

	// Result
	int width = std::abs(x1 - x0) + 1;
	int height = std::abs(y1 - y0) + 1;

	return double(sum) / double(width * height);
}

int PixelSum::getNonZeroCount(int x0, int y0, int x1, int y1) const
{
	// Prepare
	auto rect = utils::Rect(x0, y0, x1, y1)
		.normalized()
		.intersected(0, 0, _xWidth - 1, _yHeight - 1);

	int minX = rect.x0;
	int minY = rect.y0;
	int maxX = rect.x1;
	int maxY = rect.y1;

	// Calculate
	unsigned int B = (minX > 0 && minY > 0) ? _summedNonZeroArea[(minX - 1) + (minY - 1) * _xWidth] : 0;
	unsigned int C = minY > 0 ? _summedNonZeroArea[maxX + (minY - 1) * _xWidth] : 0;

	unsigned int A = _summedNonZeroArea[maxX + maxY * _xWidth];
	unsigned int D = minX > 0 ? _summedNonZeroArea[(minX - 1) + maxY * _xWidth] : 0;

	// https://en.wikipedia.org/wiki/Summed-area_table
	return A + B - C - D;
}

double PixelSum::getNonZeroAverage(int x0, int y0, int x1, int y1) const
{
	// Calculate
	unsigned int sum = getPixelSum(x0, y0, x1, y1);
	unsigned int count = getNonZeroCount(x0, y0, x1, y1);

	// Result
	return count > 0 ? double(sum) / double(count) : 0.0;
}

void PixelSum::allocateMemory()
{
	// TODO. Use PixelSum allocator and to cache mem blocks. to Optimization 30-40% at 4k
	_buffer = new unsigned char[_xWidth * _yHeight];
	_summedArea = new unsigned int[_xWidth * _yHeight];
	_summedNonZeroArea = new unsigned int[_xWidth * _yHeight];
}

void PixelSum::freeMemory()
{
	delete[] _summedNonZeroArea;
	delete[] _summedArea;
	delete[] _buffer;
}

} // End integral