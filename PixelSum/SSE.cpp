#include "SSE.h"

#include <algorithm>	// min
#include <immintrin.h>	// SSE instructions


inline int reduce_u32(__m128i a)
{
	__m128i result;
	result = _mm_hadd_epi32(a, a);
	result = _mm_hadd_epi32(result, result);

	return result.m128i_u32[0];
}

_inline __m128i add16_u8_u8(__m128i au8, __m128i bu8)
{
	const __m128i zero = _mm_setzero_si128();

	// Make 2 x 8 x 16bits
	__m128i aLower = _mm_unpacklo_epi8(au8, zero);   // First 8 ushort values
	__m128i bLower = _mm_unpacklo_epi8(bu8, zero);   // First 8 ushort values

	// sum16 = a + b
	return _mm_adds_epu16(aLower, bLower);
}


_inline __m128i add_u16_u8(__m128i u16, __m128i u8)
{
	const __m128i zero = _mm_setzero_si128();

	// Make 2 x 8 x 16bits
	__m128i lower = _mm_unpacklo_epi8(u8, zero);   // First 8 ushort values
	__m128i higher = _mm_unpackhi_epi8(u8, zero);  // Last 8 ushort values

	// sum16 += lower + higher
	__m128i blockSum = _mm_adds_epu16(lower, higher);
	
	return _mm_adds_epu16(u16, blockSum);
}

_inline __m128i add_u32_u16(__m128i u32, __m128i u16)
{
	const __m128i zero = _mm_setzero_si128();

	// Make 2 x 4 x 32bits
	__m128i lower = _mm_unpacklo_epi16(u16, zero);   // First 4 uint values
	__m128i higher = _mm_unpackhi_epi16(u16, zero);  // Last 4 uint values

	// sum32 += lower + higher
	__m128i partSum = _mm_add_epi32(lower, higher);
	return _mm_add_epi32(u32, partSum);
}

_inline __m128i non_zero(__m128i values)
{
	const __m128i zero = _mm_setzero_si128();
	const __m128i bitmask = _mm_set1_epi8(1);

	// Equal zero
	__m128i eqZero = _mm_cmpeq_epi8(values, zero);

	// !([00/FF] -> [00/01]) 
	return _mm_andnot_si128(eqZero, bitmask);
}

//
int sumSSE(const unsigned char* data, int len)
{
	__m128i zero = _mm_setzero_si128();


	int nlanes = 16;

	// 4 x 32bits
	__m128i xSum32 = _mm_setzero_si128();

	int x = 0;

	int roundedLen = len & -nlanes;
	while (x < roundedLen)
	{
		// 8 x 16bits
		__m128i xSum16 = _mm_setzero_si128();

		// 256 / 2
		int tmpLen = std::min(x + 128 * nlanes, roundedLen);
		for (; x < tmpLen; x += nlanes)
		{
			// 16 x 8bits
			__m128i src = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[x]));
			xSum16 = add_u16_u8(xSum16, src);
		}

		xSum32 = add_u32_u16(xSum32, xSum16);
	}

	// TODO. Add  nlanes / 2
	// ....


	int sum = reduce_u32(xSum32);

	// Add single values
	while (x < len)
	{
		sum += data[x++];
	}

	return sum;
}

int countNonZeroSSE(const unsigned char* data, int len)
{
	int nlanes = 16;

	// 4 x 32bits
	__m128i xCount32 = _mm_setzero_si128();

	int x = 0;

	int roundedLen = len & -nlanes;
	while (x < roundedLen)
	{
		// 8 x 16bits
		__m128i xCount16 = _mm_setzero_si128();

		// 256 / 2
		int tmpLen = std::min(x + 128 * nlanes, roundedLen);
		for (; x < tmpLen; x += nlanes)
		{
			// 16 x 8bits
			__m128i src = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[x]));

			__m128i nonZero = non_zero(src);
			xCount16 = add_u16_u8(xCount16, nonZero);
		}

		xCount32 = add_u32_u16(xCount32, xCount16);
	}

	// TODO. Add  nlanes / 2
	// ....

	int sum = reduce_u32(xCount32);

	// Add single values
	while (x < len)
	{
		sum += data[x++] != 0 ? 1 : 0;
	}

	return sum;
}

void sumAndCountNonZeroSSE(const unsigned char* data, int len, unsigned int& sum, unsigned int& countNonZero)
{
	int nlanes = 16;

	// 4 x 32bits
	__m128i xSum32 = _mm_setzero_si128();
	__m128i xCount32 = _mm_setzero_si128();

	int x = 0;

	int roundedLen = len & -nlanes;
	while (x < roundedLen)
	{
		// 8 x 16bits
		__m128i xSum16 = _mm_setzero_si128();
		__m128i xCount16 = _mm_setzero_si128();

		// 256 / 2
		int tmpLen = std::min(x + 128 * nlanes, roundedLen);
		for (; x < tmpLen; x += nlanes)
		{
			// 16 x 8bits
			__m128i src = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[x]));

			// Sum
			xSum16 = add_u16_u8(xSum16, src);

			// Count
			__m128i nonZero = non_zero(src);
			xCount16 = add_u16_u8(xCount16, nonZero);
		}

		// Sum
		xSum32 = add_u32_u16(xSum32, xSum16);

		// Count
		xCount32 = add_u32_u16(xCount32, xCount16);
	}

	// TODO. Add  nlanes / 2
	// ....

	sum = reduce_u32(xSum32);
	countNonZero = reduce_u32(xCount32);

	// Add single values
	while (x < len)
	{
		sum += data[x];
		countNonZero += data[x] != 0 ? 1 : 0;

		++x;
	}
}

//
void sumArraySSE(const unsigned int* src0, const unsigned int* src1, unsigned int* dst, int len)
{
	int nlanes = 4;
	int x = 0;

	int roundedLen = len & -nlanes;
	while (x < roundedLen)
	{
		// 4 x 32bits
		__m128i a = _mm_load_si128(reinterpret_cast<const __m128i*>(&src0[x]));
		__m128i b = _mm_load_si128(reinterpret_cast<const __m128i*>(&src1[x]));

		__m128i& result = *reinterpret_cast<__m128i*>(&dst[x]);
		result = _mm_add_epi32(a, b);

		x += nlanes;
	}

	// TODO. Add  nlanes / 2
	// ....

	// Add single values
	while (x < len)
	{
		dst[x] = src0[x] + src1[x];
	}
}

// OPtimizaed sum2
void sumArray2SSE(
	const unsigned int* srcA0,
	const unsigned int* srcA1,
	unsigned int* dstA,

	const unsigned int* srcB0,
	const unsigned int* srcB1,
	unsigned int* dstB,

	int len
)
{
	int nlanes = 4;
	int x = 0;

	int roundedLen = len & -nlanes;
	while (x < roundedLen)
	{
		// For A
		{
			// 4 x 32bits
			__m128i a = _mm_load_si128(reinterpret_cast<const __m128i*>(&srcA0[x]));
			__m128i b = _mm_load_si128(reinterpret_cast<const __m128i*>(&srcA1[x]));

			__m128i& result = *reinterpret_cast<__m128i*>(&dstA[x]);
			result = _mm_add_epi32(a, b);
		}

		// For B
		{
			// 4 x 32bits
			__m128i a = _mm_load_si128(reinterpret_cast<const __m128i*>(&srcB0[x]));
			__m128i b = _mm_load_si128(reinterpret_cast<const __m128i*>(&srcB1[x]));

			__m128i& result = *reinterpret_cast<__m128i*>(&dstB[x]);
			result = _mm_add_epi32(a, b);
		}

		x += nlanes;
	}

	// TODO. Add  nlanes / 2
	// ....

	// Add single values
	while (x < len)
	{
		dstA[x] = srcA0[x] + srcA1[x];
	}
}

__m128i fillSummedAreaFirstElementSSE(__m128i valueShr0, __m128i sumLine, unsigned int* out8uint)
{
	__m128i zero = _mm_setzero_si128();

	__m128i valueShr1 = _mm_slli_si128(valueShr0, 1);
	__m128i valueShr2 = _mm_slli_si128(valueShr0, 2);
	__m128i valueShr3 = _mm_slli_si128(valueShr0, 3);

	// 8 x 16bits
	__m128i valueShr01 = add16_u8_u8(valueShr0, valueShr1);
	__m128i valueShr23 = add16_u8_u8(valueShr2, valueShr3);

	__m128i value = _mm_adds_epu16(valueShr01, valueShr23);

	// 4 x 32 bits
	__m128i valuel = _mm_unpacklo_epi16(value, zero);
	__m128i valueh = _mm_unpackhi_epi16(value, zero);
	__m128i value4h = _mm_adds_epu16(valuel, valueh);

	// 4 x 32 bits
	__m128i suml = _mm_add_epi32(sumLine, valuel);
	__m128i sumh = _mm_add_epi32(sumLine, value4h);

	*reinterpret_cast<__m128i*>(out8uint) = suml;
	*reinterpret_cast<__m128i*>(out8uint + 4) = sumh;

	return value4h;
}

__m128i fillSummedAreaElementSSE(__m128i valueShr0, __m128i sumLine, __m128i suml, __m128i sumh, unsigned int* out8uint)
{
	__m128i zero = _mm_setzero_si128();

	__m128i valueShr1 = _mm_slli_si128(valueShr0, 1);
	__m128i valueShr2 = _mm_slli_si128(valueShr0, 2);
	__m128i valueShr3 = _mm_slli_si128(valueShr0, 3);

	suml = _mm_add_epi32(suml, sumLine);
	sumh = _mm_add_epi32(sumh, sumLine);

	// 8 x 16bits
	__m128i valueShr01 = add16_u8_u8(valueShr0, valueShr1);
	__m128i valueShr23 = add16_u8_u8(valueShr2, valueShr3);

	__m128i value = _mm_adds_epu16(valueShr01, valueShr23);

	// 4 x 32 bits
	__m128i valuel = _mm_unpacklo_epi16(value, zero);
	__m128i valueh = _mm_unpackhi_epi16(value, zero);
	__m128i value4h = _mm_adds_epu16(valuel, valueh);

	// 4 x 32 bits
	suml = _mm_add_epi32(suml, valuel);
	sumh = _mm_add_epi32(sumh, value4h);

	*reinterpret_cast<__m128i*>(out8uint) = suml;
	*reinterpret_cast<__m128i*>(out8uint + 4) = sumh;

	return value4h;
}

void fillSummedAreaFirstLineSSE(const unsigned char* src, unsigned int* sum, unsigned int* zeroSum, int xWidth)
{
	int nlanes = 8;
	__m128i zero = _mm_setzero_si128();

	// 4 x 32bits
	__m128i sumLine = _mm_setzero_si128();
	__m128i zeroSumLine = _mm_setzero_si128();

	int x = 0;
	for (; x < xWidth; x += nlanes)
	{
		// 8 x 8bits
		__m128i valueShr0 = _mm_load_si128(reinterpret_cast<const __m128i*>(src + x));
		/*
		__m128i valueShr1 = _mm_slli_si128(valueShr0, 1);
		__m128i valueShr2 = _mm_slli_si128(valueShr0, 2);
		__m128i valueShr3 = _mm_slli_si128(valueShr0, 3);

		// 8 x 16bits
		__m128i valueShr01 = add16_u8_u8(valueShr0, valueShr1);
		__m128i valueShr23 = add16_u8_u8(valueShr2, valueShr3);

		__m128i value = _mm_adds_epu16(valueShr01, valueShr23);

		// 4 x 32 bits
		__m128i valuel = _mm_unpacklo_epi16(value, zero);
		__m128i valueh = _mm_unpackhi_epi16(value, zero);
		__m128i value4h = _mm_adds_epu16(valuel, valueh);

		// 4 x 32 bits
		__m128i suml = _mm_add_epi32(sumLine, valuel);
		__m128i sumh = _mm_add_epi32(sumLine, value4h);

		*reinterpret_cast<__m128i*>(sum + x) = suml;
		*reinterpret_cast<__m128i*>(sum + x + 4) = sumh;
		
		unsigned int pt = _mm_extract_epi32(value4h, 3);

		__m128i vvvv;
		vvvv.m128i_u32[0] = vvvv.m128i_u32[1] = vvvv.m128i_u32[2] = vvvv.m128i_u32[3] = pt;
		sumLine = _mm_add_epi32(sumLine, vvvv);
		*/

		__m128i value4h = fillSummedAreaFirstElementSSE(valueShr0, sumLine, sum + x);

		// 16 x 8bits
		__m128i nonZeroValueShr0 = non_zero(valueShr0);
		__m128i nonZeroValue4h = fillSummedAreaFirstElementSSE(nonZeroValueShr0, zeroSumLine, zeroSum + x);

		// Add
		unsigned int sunLineLast = _mm_extract_epi32(value4h, 3);

		__m128i vvvv;
		vvvv.m128i_u32[0] = vvvv.m128i_u32[1] = vvvv.m128i_u32[2] = vvvv.m128i_u32[3] = sunLineLast;
		sumLine = _mm_add_epi32(sumLine, vvvv);

		unsigned int nonZeroSumLineLast = _mm_extract_epi32(nonZeroValue4h, 3);

		vvvv.m128i_u32[0] = vvvv.m128i_u32[1] = vvvv.m128i_u32[2] = vvvv.m128i_u32[3] = nonZeroSumLineLast;
		sumLine = _mm_add_epi32(sumLine, vvvv);
	}

	//
	unsigned int sumLineLast = _mm_extract_epi32(sumLine, 3);
	unsigned int nonZeroSumLineLast = _mm_extract_epi32(zeroSumLine, 3);
	for (;x < xWidth;++x)
	{
		unsigned char value = src[x];
		sumLineLast += value;
		nonZeroSumLineLast += (value > 0 ? 1 : 0);

		sum[x] = sumLineLast;
		zeroSum[x] = nonZeroSumLineLast;
	}
}

void fillSummedAreaLineSSE(
	const unsigned char* src,
	unsigned int* prevSum,
	unsigned int* sum,
	unsigned int* prevZeroSum,
	unsigned int* zeroSum,
	int xWidth
)
{
	int nlanes = 8;
	__m128i zero = _mm_setzero_si128();

	// 4 x 32bits
	__m128i sumLine = _mm_setzero_si128();
	__m128i zeroSumLine = _mm_setzero_si128();
	
	int x = 0;
	for (; x < xWidth; x += nlanes)
	{
		__m128i suml = _mm_load_si128(reinterpret_cast<const __m128i*>(prevSum + x));
		__m128i sumh = _mm_load_si128(reinterpret_cast<const __m128i*>(prevSum + x + 4));

		
		// 8 x 8bits
		__m128i valueShr0 = _mm_load_si128(reinterpret_cast<const __m128i*>(src + x));
		/*
		__m128i valueShr1 = _mm_slli_si128(valueShr0, 1);
		__m128i valueShr2 = _mm_slli_si128(valueShr0, 2);
		__m128i valueShr3 = _mm_slli_si128(valueShr0, 3);

		suml = _mm_add_epi32(suml, sumLine);
		sumh = _mm_add_epi32(sumh, sumLine);

		// 8 x 16bits
		__m128i valueShr01 = add16_u8_u8(valueShr0, valueShr1);
		__m128i valueShr23 = add16_u8_u8(valueShr2, valueShr3);

		__m128i value = _mm_adds_epu16(valueShr01, valueShr23);

		// 4 x 32 bits
		__m128i valuel = _mm_unpacklo_epi16(value, zero);
		__m128i valueh = _mm_unpackhi_epi16(value, zero);
		__m128i value4h = _mm_adds_epu16(valuel, valueh);

		// 4 x 32 bits
		suml = _mm_add_epi32(suml, valuel);
		sumh = _mm_add_epi32(sumh, value4h);

		*reinterpret_cast<__m128i*>(sum + x) = suml;
		*reinterpret_cast<__m128i*>(sum + x + 4) = sumh;

		unsigned int pt = _mm_extract_epi32(value4h, 3);

		__m128i vvvv;
		vvvv.m128i_u32[0] = vvvv.m128i_u32[1] = vvvv.m128i_u32[2] = vvvv.m128i_u32[3] = pt;
		sumLine = _mm_add_epi32(sumLine, vvvv);
		*/

		__m128i value4h = fillSummedAreaElementSSE(valueShr0, sumLine, suml, sumh, sum + x);

		// 16 x 8bits
		__m128i nonZeroValueShr0 = non_zero(valueShr0);
		__m128i nonZeroValue4h = fillSummedAreaElementSSE(nonZeroValueShr0, zeroSumLine, suml, sumh, zeroSum + x);

		// Add
		unsigned int sunLineLast = _mm_extract_epi32(value4h, 3);

		__m128i vvvv;
		vvvv.m128i_u32[0] = vvvv.m128i_u32[1] = vvvv.m128i_u32[2] = vvvv.m128i_u32[3] = sunLineLast;
		sumLine = _mm_add_epi32(sumLine, vvvv);

		unsigned int nonZeroSumLineLast = _mm_extract_epi32(nonZeroValue4h, 3);

		vvvv.m128i_u32[0] = vvvv.m128i_u32[1] = vvvv.m128i_u32[2] = vvvv.m128i_u32[3] = nonZeroSumLineLast;
		sumLine = _mm_add_epi32(sumLine, vvvv);
	}

	//
	unsigned int sumLineLast = _mm_extract_epi32(sumLine, 3);
	unsigned int nonZeroSumLineLast = _mm_extract_epi32(zeroSumLine, 3);
	for (; x < xWidth; ++x)
	{
		unsigned char value = src[x];

		// SA(x, y) = B(x, y) + SA(x - 1, y) + SA(x, y - 1)
		unsigned int currentSumValue = value + sumLineLast;
		unsigned int currentZeroSumValue = zeroSum[x] = (value > 0 ? 1 : 0) + nonZeroSumLineLast;

		sum[x] = currentSumValue + prevSum[x];
		zeroSum[x] = currentZeroSumValue + prevZeroSum[x];

		// Save 
		sumLineLast = currentSumValue;
		nonZeroSumLineLast = currentZeroSumValue;
	}
}

void fillSummedAreaSSE(const unsigned char* buffer, unsigned int* summedArea, unsigned int* summedNonZeroArea, int xWidth, int yHeight)
{
	// First line
	{
		auto src = buffer;
		auto sum = summedArea;
		auto zeroSum = summedNonZeroArea;

		fillSummedAreaFirstLineSSE(buffer, summedArea, summedNonZeroArea, xWidth);
	}

	// Others
	for (int y = 1; y < yHeight; ++y)
	{
		const auto src = buffer + y * xWidth;

		const auto prevSum = summedArea + (y - 1) * xWidth;
		auto sum = summedArea + y * xWidth;

		const auto prevZeroSum = summedNonZeroArea + (y - 1) * xWidth;
		auto zeroSum = summedNonZeroArea + y * xWidth;

		fillSummedAreaLineSSE(
			src,
			prevSum, sum,
			prevZeroSum, zeroSum,
			xWidth
		);
	}
}