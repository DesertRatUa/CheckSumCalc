#pragma once

#include <string>

class ILogger
{
public:
	virtual void Log(const std::string& text) const = 0;
	virtual void LogError(const std::string& text) const = 0;
};