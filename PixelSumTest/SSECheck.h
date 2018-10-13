#pragma once

/*
 * Check SSE/AVX support.
 * This application can detect the instruction support of
 * SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2, SSE4a, SSE5, and AVX.
 *
 * https://gist.github.com/hi2p-perim/7855506
 */

void showCpuSupport();
bool isAVXSupport();