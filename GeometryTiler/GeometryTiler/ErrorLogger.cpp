#include "ErrorLogger.h"

// Nicely logs the provided error message and error code.
void ErrorLogger::LogError(std::wstring errorMessage, fgdbError errorCode)
{
    std::wcout << errorMessage << std::endl;
    
    std::wstring errorText;
    FileGDBAPI::ErrorInfo::GetErrorDescription(errorCode, errorText);
    std::wcout << "Error: " << errorText << "[" << errorCode << "]" << std::endl;
}