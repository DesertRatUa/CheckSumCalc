#include "pch.h"
#include <thread>
#include "Config.h"

static const unsigned DEFAULT_BLOCK_SIZE = 1024 * 1024;

Config::Config() : m_blockSize(DEFAULT_BLOCK_SIZE), m_numCores(std::thread::hardware_concurrency())
{
}

Config::~Config()
{
}

std::string Config::GetInputFilePath() const
{
	return m_inputPath;
}

std::string Config::GetOutputFilePath() const
{
	return m_outpupPath;
}

unsigned Config::GetBlockSize() const
{
	return m_blockSize;
}

unsigned Config::GetThreadNum() const
{
	return m_numCores;
}

void Config::SetInputFilePath(const std::string& path)
{
	m_inputPath = path;
	LOG("Set input path: " + m_inputPath);
}

void Config::SetOutputFilePath(const std::string& path)
{
	m_outpupPath = path;
	LOG("Set output path: " + m_outpupPath);
}

void Config::SetBlockSize(unsigned size)
{
	m_blockSize = size;
	LOG("Set block size: " + std::to_string(m_blockSize));
}
