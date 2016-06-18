#pragma once
#include <iostream>
#include <FileGDBAPI.h>

class ErrorLogger
{
public:
    static void LogError(std::wstring errorMessage, fgdbError errorCode);
};

