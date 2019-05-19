#include "pch.h"
#include <iostream>

#include <Logger/Logger.h>
#include <Config/Config.h>
#include <Env/Env.h>
#include <MD5Hash/MD5Hash.h>

void SetupEnv(Env& env)
{
	env.SetupConfig(std::make_shared<IConfig*>(new Config()));
	env.SetupLogger(std::make_shared<ILogger*>(new Logger()));
}

void SetupConfig(int argc, char *argv[])
{
	IConfig* config = Env::GetEnv()->SetConfig();
	
	config->SetInputFilePath(argv[1]);
	config->SetOutputFilePath(argv[2]);
	if (argc >= 4)
	{
		int size = std::stoi(argv[3]);
		if (size > 0)
		{
			config->SetBlockSize(size);
		}
	}
}

int main(int argc, char *argv[])
{
	try
	{
		Env env;
		SetupEnv(env);

		if (argc < 3)
		{
			LOG("Min 2 args");
			return 1;
		}

		SetupConfig(argc, argv);

		MD5Hash hash;
		hash.CalculateHashForFile();
	}
	catch (std::exception &e)
	{
		LOGERROR("Exception: " + std::string(e.what()));
	}
	catch (...)
	{
		LOGERROR("Unrecognized Exception catch");
	}
	return 0;
}
