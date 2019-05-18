#pragma once

class IConfig
{
public:
	virtual const std::string GetInputFilePath() const = 0;
	virtual const std::string GetOutputFilePath() const = 0;
	virtual const unsigned GetBlockSize() const = 0;

	virtual void SetInputFilePath(const std::string& path)  = 0;
	virtual void SetOutputFilePath(const std::string& path) = 0;
	virtual void SetBlockSize(unsigned size) = 0;
};