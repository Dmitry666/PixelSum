#pragma once

// Optimized methods

// Sum all elements of an array
int sumSSE(const unsigned char* data, int len);

// Count all non zero elements of an array
int countNonZeroSSE(const unsigned char* data, int len);

// Combined method Sum all elements and count non zero
void sumAndCountNonZeroSSE(const unsigned char* data, int len, unsigned int& sum, unsigned int& countNonZero);

// Fill summed area
void fillSummedAreaSSE(const unsigned char* buffer, unsigned int* summedArea, unsigned int* summedNonZeroArea, int xWidth, int yHeight);