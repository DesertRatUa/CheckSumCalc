#pragma once
#include "Interfaces/ILogger.h"

class Logger : public ILogger
{
public:
	Logger();
	virtual ~Logger();

	virtual void Log(const std::string& text) const;
	virtual void LogError(const std::string& text) const;
};
