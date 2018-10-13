#pragma once

#include <algorithm>

namespace utils {

template<class T>
T clamp(T x, T lower, T upper)
{
	return std::min(upper, std::max(x, lower));
}

struct Rect
{
	int x0;
	int y0;

	int x1;
	int y1;

	Rect(int inX0, int inY0, int inX1, int inY1)
		: x0(inX0)
		, y0(inY0)
		, x1(inX1)
		, y1(inY1)
	{}

	Rect normalized() const
	{
		return Rect(
			std::min(x0, x1),
			std::min(y0, y1),

			std::max(x0, x1),
			std::max(y0, y1)
		);
	}

	Rect intersected(int inX0, int inY0, int inX1, int inY1) const
	{
		// TODO. Use 'std::clamp' instead of 'utils::clamp' in C++17
		return Rect(
			clamp(x0, inX0, inX1),
			clamp(y0, inY0, inY1),
			clamp(x1, inX0, inX1),
			clamp(y1, inY0, inY1)
		);
	}

	int getWidth() const
	{
		return std::abs(x1 - x0) + 1;
	}

	int getHeight() const
	{
		return std::abs(y1 - y0) + 1;
	}
};

} // End utils