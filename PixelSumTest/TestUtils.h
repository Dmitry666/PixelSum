#pragma once

#include <limits.h>

#include "ConsoleTextColor.h"

#define TEST_CASE(name) \
	struct TestCase_##name \

#define TEST_CHECK(func, name, body) \
	{ \
		auto startTime = std::chrono::high_resolution_clock::now(); \
		\
		bool result = func; \
		\
		auto finisTime = std::chrono::high_resolution_clock::now(); \
		auto timeMks = std::chrono::duration_cast<std::chrono::microseconds>(finisTime - startTime).count(); \
		\
		if(result) \
		{ \
			ConsoleTextColor consoleColor(ConsoleTextColor::ForeGroundGreen); \
			std::cout << name << " " << body << " - test passed (" << timeMks << "mks)" << std::endl; \
		} \
		else \
		{ \
			ConsoleTextColor consoleColor(ConsoleTextColor::ForeGroundRed); \
			std::cout << name << " " << body << " - test failed (" << timeMks << "mks)" << std::endl; \
		} \
	}

#define TEST_CHECK_EQUAL(func, value, name, body) \
	{ \
		auto startTime = std::chrono::high_resolution_clock::now(); \
		\
		bool result = std::abs(func - value) <= std::numeric_limits<decltype(value)>::epsilon(); \
		\
		auto finisTime = std::chrono::high_resolution_clock::now(); \
		auto timeMks = std::chrono::duration_cast<std::chrono::microseconds>(finisTime - startTime).count(); \
		\
		if(result) \
		{ \
			ConsoleTextColor consoleColor(ConsoleTextColor::ForeGroundGreen); \
			std::cout << name << " " << body << " = " << value << " - test passed (" << timeMks << "mks)" << std::endl; \
		} \
		else \
		{ \
			ConsoleTextColor consoleColor(ConsoleTextColor::ForeGroundRed); \
			std::cout << name << " " << body << " = " << value << " - test failed (" << timeMks << "mks)" << std::endl; \
		} \
	}