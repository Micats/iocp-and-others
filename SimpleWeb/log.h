#pragma once
#include <stdarg.h>
#include<Windows.h>
#include <stdio.h>
void OutputDebugPrintf(const char * strOutputString, ...)
{
	char strBuffer[4096] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	_vsnprintf(strBuffer, sizeof(strBuffer)-1, strOutputString, vlArgs);
	//vsprintf(strBuffer,strOutputString,vlArgs);
	va_end(vlArgs);
	OutputDebugStringA(strBuffer);
}