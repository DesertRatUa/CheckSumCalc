#include "pch.h"
#include <iostream>

#include "Logger.h"

Logger::Logger()
{
}

Logger::~Logger()
{
}

void Logger::Log(const std::string& text) const
{
	std::cout << text.c_str() << std::endl;
}

void Logger::LogError(const std::string& text) const
{
	std::cout << "ERROR:" << text.c_str() << std::endl;
}