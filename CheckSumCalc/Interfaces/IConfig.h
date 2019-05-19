#pragma once

class IConfig
{
public:
	virtual std::string GetInputFilePath() const = 0;
	virtual std::string GetOutputFilePath() const = 0;
	virtual unsigned GetBlockSize() const = 0;
	virtual unsigned GetThreadNum() const = 0;

	virtual void SetInputFilePath(const std::string& path)  = 0;
	virtual void SetOutputFilePath(const std::string& path) = 0;
	virtual void SetBlockSize(unsigned size) = 0;
};