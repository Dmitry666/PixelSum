#include "PixelSumNaiveV2.h"

#include <string.h>		// memcpy
#include <stdlib.h>		// malloc, free, rand
#include <assert.h>
#include <algorithm>	// min, max, clamp

#include "Utils.h"

#include "SSE.h"

namespace naivev2 {

PixelSum::PixelSum(const unsigned char* buffer, int xWidth, int yHeight)
	: _xWidth(xWidth)
	, _yHeight(yHeight)
{
	assert(buffer != nullptr);
	assert(xWidth > 0 && yHeight > 0);
	assert(xWidth * yHeight <= 4096 * 4096); // 4096 * 4096 * 256 == max(uint32)

	// TODO. Use PixelSum allocator and to cache mem blocks
	_buffer = new unsigned char[_xWidth * _yHeight];
	memcpy(_buffer, buffer, xWidth * yHeight * sizeof(unsigned char));
}

PixelSum::~PixelSum()
{
	delete[] _buffer;
}

PixelSum::PixelSum(const PixelSum& other)
	: _xWidth(other._xWidth)
	, _yHeight(other._yHeight)
{
	// Copy
	_buffer = new unsigned char[_xWidth * _yHeight];
	memcpy(_buffer, other._buffer, _xWidth * _yHeight * sizeof(unsigned char));
}

PixelSum::PixelSum(PixelSum&& other)
	: _xWidth(other._xWidth)
	, _yHeight(other._yHeight)
{
	// Move
	_buffer = other._buffer;
	other._buffer = nullptr;
}

PixelSum& PixelSum::operator=(const PixelSum& other)
{
	assert(&other != this);

	// Free
	delete[] _buffer;

	// Copy
	_xWidth = other._xWidth;
	_yHeight = other._yHeight;

	_buffer = (unsigned char*)malloc(_xWidth * _yHeight * sizeof(unsigned char));
	memcpy(_buffer, other._buffer, _xWidth * _yHeight * sizeof(unsigned char));

	return *this;
}

PixelSum& PixelSum::operator=(PixelSum&& other)
{
	assert(&other != this);

	// Free
	delete[] _buffer;

	// Move
	_xWidth = other._xWidth;
	_yHeight = other._yHeight;

	_buffer = other._buffer;
	other._buffer = nullptr;

	return *this;
}

unsigned int PixelSum::getPixelSum(int x0, int y0, int x1, int y1) const
{
	// Prepare
	auto rect = utils::Rect(x0, y0, x1, y1)
		.normalized()
		.intersected(0, 0, _xWidth - 1, _yHeight - 1);

	int rectWidth = rect.getWidth();

	// Calculate
	int sum = 0;

	for (int y = rect.y0; y <= rect.y1; ++y)
	{
#ifdef __SSE2__
		int offset = rect.x0 + y * _xWidth;
		auto ptr = _buffer + offset;

		sum += sumSSE(ptr, rectWidth);
#else
		for (int x = rect.x0; x <= rect.x1; ++x)
		{
			sum += _buffer[x + y * _xWidth];
		}
#endif // __AVX2__
	}
	
	return sum;
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

	int rectWidth = rect.getWidth();

	// Calculate
	unsigned int count = 0;

	for (int y = rect.y0; y <= rect.y1; ++y)
	{
#ifdef __SSE2__
		int offset = rect.x0 + y * _xWidth;
		auto ptr = _buffer + offset;

		count += countNonZeroSSE(ptr, rectWidth);
#else
		for (int x = rect.x0; x <= rect.x1; ++x)
		{
			unsigned char value = _buffer[x + y * _xWidth];
			count += value != 0 ? 1 : 0;
		}
#endif // __AVX2__
	}

	return count;
}

double PixelSum::getNonZeroAverage(int x0, int y0, int x1, int y1) const
{
	// Prepare
	auto rect = utils::Rect(x0, y0, x1, y1)
		.normalized()
		.intersected(0, 0, _xWidth - 1, _yHeight - 1);

	int rectWidth = rect.getWidth();

	// Calculate
	unsigned int sum = 0;
	unsigned int count = 0;

	for (int y = rect.y0; y <= rect.y1; ++y)
	{
#ifdef __SSE2__
		int offset = rect.x0 + y * _xWidth;
		auto ptr = _buffer + offset;

		sumAndCountNonZeroSSE(ptr, rectWidth, sum, count);
#else
		for (int x = rect.x0; x <= rect.x1; ++x)
		{
			unsigned char value = _buffer[x + y * _xWidth];

			sum += value;
			count += value != 0 ? 1 : 0;
		}
#endif // __AVX2__
	}

	// Result
	return count != 0 ? double(sum) / double(count) : 0.0;
}

} // End naivev2