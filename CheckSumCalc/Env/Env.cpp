#include "pch.h"
#include "Env.h"

Env* Env::m_instance = NULL;

Env::Env()
{
	m_instance = this;
}

Env::~Env()
{
	m_instance = NULL;
}

void Env::SetupConfig(std::shared_ptr<IConfig*> config)
{
	m_config = config;
}

void Env::SetupLogger(std::shared_ptr<ILogger*> logger)
{
	m_logger = logger;
}

const IConfig* Env::GetConfig() const
{
	return *m_config;
}

IConfig* Env::SetConfig() const
{
	return *m_config;
}

const ILogger* Env::GetLogger() const
{
	return *m_logger;
}

const Env* Env::GetEnv()
{
	return m_instance;
}