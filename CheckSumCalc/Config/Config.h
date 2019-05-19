#pragma once
#include "Interfaces/IConfig.h"

class Config : public IConfig
{
public:
	Config();
	virtual ~Config();

	virtual std::string GetInputFilePath() const;
	virtual std::string GetOutputFilePath() const;
	virtual unsigned GetBlockSize() const;
	virtual unsigned GetThreadNum() const;

	virtual void SetInputFilePath(const std::string& path);
	virtual void SetOutputFilePath(const std::string& path);
	virtual void SetBlockSize(unsigned size);

private:
	std::string m_inputPath;
	std::string m_outpupPath;
	unsigned m_blockSize;
	unsigned m_numCores;
};

