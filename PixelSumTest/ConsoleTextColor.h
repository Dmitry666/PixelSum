#pragma once

#ifdef _WIN32
#include <Windows.h>
#else
typedef unsigned short WORD;
#endif

class ConsoleTextColor
{
public:
	static const WORD ForeGroundBlue = 0x0001;
	static const WORD ForeGroundGreen = 0x0002;
	static const WORD ForeGroundLightBlue = 0x0003;
	static const WORD ForeGroundRed = 0x0004;
	static const WORD ForeGroundPurpure = 0x0005;
	static const WORD ForeGroundYellow = 0x0006;
	static const WORD ForeGroundWhite = 0x0007;
	static const WORD ForeGroundIntensity = 0x0008;

	static const WORD BackGroundBlue = 0x0010;
	static const WORD BackGroundGreen = 0x0020;
	static const WORD BackGroundRed = 0x0040;
	static const WORD BackGroundIntensity = 0x0080;

public:
	ConsoleTextColor(WORD color);
	~ConsoleTextColor();
	static void setConsoleTextColor(WORD color);

private:
	ConsoleTextColor()
	{}

	WORD _previousColor;
};