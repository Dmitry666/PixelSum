#include "PixelSumNaive.h"
#include "PixelSumNaiveV2.h"
#include "PixelSumIntegral.h"

#include <vector>
#include <chrono>
#include <ratio>

#include <ctime>		// std::time
#include <cstdlib>		// std::rand
#include <algorithm>	// std::generate

#include <iostream>		// std::cout

#include "TestUtils.h"
#include "SSECheck.h"


template<class TFunction>
std::vector<unsigned char> makeData(int xWidth, int yHeight, TFunction func)
{
	std::vector<unsigned char> values(xWidth * yHeight);
	std::generate(values.begin(), values.end(), func);
	return values;
}

std::vector<unsigned char> makeData(int xWidth, int yHeight)
{
	std::vector<unsigned char> values(xWidth * yHeight);

	int index = 0;
	std::generate(values.begin(), values.end(), [&index]() {
		return index++ % 256;
	});

	return values;
}

std::vector<unsigned char> makeDataOne(int xWidth, int yHeight)
{
	std::vector<unsigned char> values(xWidth * yHeight);
	std::fill(values.begin(), values.end(), 1);
	return values;
}

std::vector<unsigned char> makeDataMax(int xWidth, int yHeight)
{
	std::vector<unsigned char> values(xWidth * yHeight);
	std::fill(values.begin(), values.end(), 255);
	return values;
}

std::vector<unsigned char> makeRandomData(int xWidth, int yHeight)
{
	std::vector<unsigned char> values(xWidth * yHeight);

	std::generate(values.begin(), values.end(), []() {
		return std::rand() % 256;
	});

	return values;
}

std::vector<unsigned char> makeDataWithoutZero(int xWidth, int yHeight)
{
	std::vector<unsigned char> values(xWidth * yHeight);

	int index = 0;
	std::generate(values.begin(), values.end(), [&index]() {
		return (index++ % 255) + 1;
	});

	return values;
}

std::vector<unsigned char> makeDataZero(int xWidth, int yHeight)
{
	std::vector<unsigned char> values(xWidth * yHeight);
	std::fill(values.begin(), values.end(), 0);
	return values;
}


/*
TEST_CASE(test0)
{

}
*/

template<class TPixelSum>
void test(const naive::PixelSum& pixelSum0, const TPixelSum& pixelSum, const char* name, int x0, int y0, int x1, int y1)
{
	// Get naive values
	unsigned int v0Sum = pixelSum0.getPixelSum(x0, y0, x1, y1);
	double v0Average = pixelSum0.getPixelAverage(x0, y0, x1, y1);
	unsigned int v0NonZeroCount = pixelSum0.getNonZeroCount(x0, y0, x1, y1);
	double v0NonZeroAverage = pixelSum0.getNonZeroAverage(x0, y0, x1, y1);
	
	// Tests
	TEST_CHECK(pixelSum.getPixelSum(x0, y0, x1, y1) == v0Sum, name, "Sum");
	TEST_CHECK_EQUAL(pixelSum.getPixelAverage(x0, y0, x1, y1), v0Average, name, "Average");
	TEST_CHECK(pixelSum.getNonZeroCount(x0, y0, x1, y1) == v0NonZeroCount, name, "NonZeroCount");
	TEST_CHECK_EQUAL(pixelSum.getNonZeroAverage(x0, y0, x1, y1), v0NonZeroAverage, name, "NonZeroAverage");
}

template<class TPixelSum>
void testCaseBase(const char* name, const std::vector<unsigned char>& values, int xWidth, int yWidth)
{
	// Naive implementation
	naive::PixelSum pixelSum0(values.data(), xWidth, yWidth);

	// Tested implementation
	auto startMakeTime = std::chrono::high_resolution_clock::now();
	TPixelSum pixelSum(values.data(), xWidth, yWidth);
	auto finisMakeTime = std::chrono::high_resolution_clock::now();

	// Preapre time
	auto makeTimeMks = std::chrono::duration_cast<std::chrono::microseconds>(finisMakeTime - startMakeTime).count();

	std::cout << name << " (" << xWidth << "x" << yWidth << ") Preapre time: " << makeTimeMks << "mks" << std::endl;

	// Tests
	test(pixelSum0, pixelSum, "(0, 0, 100%, 100%)    ", 0, 0, xWidth - 1, yWidth - 1);
	test(pixelSum0, pixelSum, "(50%, 50%, 50%, 50%)  ", xWidth / 2, yWidth / 2, xWidth / 2, yWidth / 2);
	test(pixelSum0, pixelSum, "(25%, 25%, 75%, 75%)  ", xWidth / 4, yWidth / 4, xWidth * 3 / 4, yWidth * 3 / 4);
	test(pixelSum0, pixelSum, "(75%, 75%, 25%, 25%)  ", xWidth * 3 / 4, yWidth * 3 / 4, xWidth / 4, yWidth / 4);
	test(pixelSum0, pixelSum, "(50%, 50%, 110%, 110%)", xWidth / 2, yWidth / 2, xWidth + 10, yWidth + 10);
	test(pixelSum0, pixelSum, "(-10, -10, 50%, 50%)  ", -10, -10, xWidth / 2, yWidth / 2);
	test(pixelSum0, pixelSum, "(-10, -10, 110%, 110%)", -10, -10, xWidth + 10, yWidth + 10);

	std::cout << std::endl;
}

void testCaseNaive(int xWidth = 4096, int yWidth = 4096)
{
	std::vector<unsigned char> values = makeData(xWidth, yWidth);
	return testCaseBase<naivev2::PixelSum>("Optimized naive", values, xWidth, yWidth);
}

void testCaseMain(int xWidth = 4096, int yWidth = 4096)
{
	std::vector<unsigned char> values = makeDataOne(xWidth, yWidth);
	return testCaseBase<integral::PixelSum>("SAT", values, xWidth, yWidth);
}

void testCaseNoZero(int xWidth = 4096, int yWidth = 4096)
{
	std::vector<unsigned char> values = makeDataWithoutZero(xWidth, yWidth);
	return testCaseBase<integral::PixelSum>("SAT without zero", values, xWidth, yWidth);
}

void testCaseOnlyZero(int xWidth = 4096, int yWidth = 4096)
{
	std::vector<unsigned char> values = makeDataZero(xWidth, yWidth);
	return testCaseBase<integral::PixelSum>("SAT only zero", values, xWidth, yWidth);
}

void testCaseMax(int xWidth = 4096, int yWidth = 4096)
{
	std::vector<unsigned char> values = makeDataMax(xWidth, yWidth);
	return testCaseBase<integral::PixelSum>("SAT only maximum", values, xWidth, yWidth);
}


int main(int argc, char** argv)
{
#ifdef __AVX2__
	showCpuSupport();
	if (!isAVXSupport())
	{
		std::cout << "AVX not supported" << std::endl;
		return -1;
	}
#elif __SSE2__
	// x86 64 minimum SSE2
#endif // __SSE2__

	std::srand(unsigned(std::time(0)));

	// Tests
	testCaseNaive();
	testCaseMain();
	testCaseMain(359, 257);
	testCaseNoZero();
	testCaseOnlyZero();
	testCaseMax();

	return 0;
}