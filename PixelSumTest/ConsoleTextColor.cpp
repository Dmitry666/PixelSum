#include "ConsoleTextColor.h"

#ifndef _MSC_VER
const WORD ConsoleTextColor::ForeGroundBlue;
const WORD ConsoleTextColor::ForeGroundGreen;
const WORD ConsoleTextColor::ForeGroundLightBlue;
const WORD ConsoleTextColor::ForeGroundRed;
const WORD ConsoleTextColor::ForeGroundPurpure;
const WORD ConsoleTextColor::ForeGroundYellow;
const WORD ConsoleTextColor::ForeGroundWhite;
const WORD ConsoleTextColor::ForeGroundIntensity;

const WORD ConsoleTextColor::BackGroundBlue;
const WORD ConsoleTextColor::BackGroundGreen;
const WORD ConsoleTextColor::BackGroundRed;
const WORD ConsoleTextColor::BackGroundIntensity;
#endif

ConsoleTextColor::ConsoleTextColor(WORD color)
{
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
	_previousColor = info.wAttributes;
	setConsoleTextColor(color);
#endif
}

ConsoleTextColor::~ConsoleTextColor()
{
#ifdef _WIN32
	setConsoleTextColor(_previousColor);
#endif
}

void ConsoleTextColor::setConsoleTextColor(WORD color)
{
#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
#endif
}