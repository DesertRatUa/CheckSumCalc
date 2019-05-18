#pragma once

#include "Interfaces/IConfig.h"
#include "Interfaces/ILogger.h"

class Env
{
public:
	Env();
	virtual ~Env();

	static const Env* GetEnv();

	void SetupConfig(std::shared_ptr<IConfig*> config);
	void SetupLogger(std::shared_ptr<ILogger*> logger);

	const IConfig* GetConfig() const;
	const ILogger* GetLogger() const;

	virtual IConfig* SetConfig() const;

private:
	std::shared_ptr<IConfig*> m_config;
	std::shared_ptr<ILogger*> m_logger;

	static Env* m_instance;
};

#define LOG(arg) Env::GetEnv()->GetLogger()->Log(arg)
#define LOGERROR(arg) Env::GetEnv()->GetLogger()->LogError(arg)
#define GET_CONFIG() Env::GetEnv()->GetConfig()