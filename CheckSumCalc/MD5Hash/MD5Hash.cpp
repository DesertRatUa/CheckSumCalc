#include "pch.h"
#include "MD5Hash.h"
#include <stdio.h>
#include <windows.h>
#include <Wincrypt.h>

#include <fstream>
#include <iterator>
#include <vector>
#include <future>

static const unsigned MD5LEN = 16;
static const unsigned BUFSIZE = 1024;

MD5Hash* MD5Hash::m_instance = NULL;

MD5Hash::MD5Hash() : m_threadPool(GET_CONFIG()->GetThreadNum()), m_run(false)
{
	m_instance = this;
}

MD5Hash::~MD5Hash()
{
	m_threadPool.stop();
	m_run = false;
	if (m_writeThread)
	{
		m_writeThread->join();
	}
	m_instance = NULL;
}

void MD5Hash::CalculateHashForFile()
{
	m_run = true;
	unsigned bufferSize = GET_CONFIG()->GetBlockSize();
	std::string filename = GET_CONFIG()->GetInputFilePath().c_str();
	std::ifstream file;
	try
	{
		file.open(filename.c_str(), std::ifstream::binary);

		if (!file)
		{
			throw std::runtime_error("Failed to open file: " + filename);
		}

		m_writeThread = new std::thread(WriteHasToFile);

		long long int fileSize = 0;
		long long int filePos = 0;
		long long int fileRest = 0;

		file.seekg(0, file.end);
		fileRest = fileSize = file.tellg();
		file.seekg(0, file.beg);

		while (m_run && file.good() && fileRest > bufferSize)
		{
			TByteVecShare buffer;
			while (!buffer.get()) //Out of RAM
			{
				try
				{
					buffer.reset(new TByteVec(bufferSize + 1));
				}
				catch (...)
				{
					Sleep(100);
				}
			}
			file.read((char*)buffer->data(), bufferSize);
			AddJobToQueue(buffer);
			filePos = file.tellg();
			fileRest = fileSize - filePos;
		}

		if (m_run)
		{
			unsigned bufferRest = bufferSize - unsigned(fileRest);
			TByteVecShare buffer(new TByteVec(unsigned(fileRest)));
			file.read((char*)buffer->data(), fileRest);
			buffer->insert(buffer->end(), bufferRest, '0');
			AddJobToQueue(buffer);
		}
	}
	catch (std::exception &e)
	{
		LOGERROR("Exception in CalculateHashForFile: " + std::string(e.what()));
	}

	file.close();
	m_threadPool.stop();
	m_run = false;
	if (m_writeThread)
	{
		m_writeThread->join();
		m_writeThread = NULL;
	}
}
	
void MD5Hash::AddJobToQueue(TByteVecShare buff)
{
	TSharePrm newThreadPromise(new std::promise<void>);

	if (!m_currThreadPromise.get())
	{
		m_currThreadPromise.reset(new std::promise<void>);
		m_currThreadPromise->set_value();
	}

	m_threadPool.push(StoreHashForBuff, buff, m_currThreadPromise, newThreadPromise);
	m_currThreadPromise = newThreadPromise;
}

void MD5Hash::PushHashResult(TStringShare& str)
{
	std::lock_guard<std::mutex> locker(m_resultMutex);
	LOG(*str);
	m_result += *str;
}

void MD5Hash::WriteHasToFile()
{
	std::ofstream file;
	try 
	{
		if (!m_instance)
		{
			throw std::runtime_error("MD5Hash m_instance not initilized");
		}

		std::string filename = GET_CONFIG()->GetOutputFilePath().c_str();
		file.open(filename.c_str(), std::ios_base::trunc);
		if (!file)
		{
			throw std::runtime_error("Failed to open output file: " + filename);
		}

		while (m_instance->m_run)
		{
			Sleep(100);
			std::string& result = m_instance->m_result;
			std::string tmpStrg;
			{
				std::lock_guard<std::mutex> locker(m_instance->m_resultMutex);
				tmpStrg = result;
			}
			file.write(tmpStrg.c_str(), tmpStrg.size());
			result.clear();
		}
	}
	catch (std::exception &e)
	{
		m_instance->m_run = false;
		LOGERROR("Exception in WriteHasToFile thread: " + std::string(e.what()));
	}
	catch (...)
	{
		m_instance->m_run = false;
		LOGERROR("Exception in WriteHasToFile thread");
	}

	file.close();
}

void MD5Hash::StoreHashForBuff(int id, TByteVecShare buff, TSharePrm prev, TSharePrm own)
{
	DWORD dwStatus = 0;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	BYTE rgbHash[MD5LEN];
	CHAR rgbDigits[] = "0123456789abcdef";
	TStringShare result(new std::string);

	try
	{
		if (!m_instance)
		{
			throw std::runtime_error("MD5Hash m_instance not initilized");
		}

		if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
		{
			dwStatus = GetLastError();
			throw std::runtime_error("CryptAcquireContext failed: " + std::to_string(dwStatus));
		}

		if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
		{
			dwStatus = GetLastError();
			throw std::runtime_error("CryptAcquireContext failed: " + std::to_string(dwStatus));
		}

		if (!CryptHashData(hHash, buff->data(), buff->size(), 0))
		{
			dwStatus = GetLastError();
			throw std::runtime_error("CryptHashData failed: " + dwStatus);
		}

		DWORD cbHash = MD5LEN;
		if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
		{
			for (DWORD i = 0; i < cbHash; i++)
			{
				*result += rgbDigits[rgbHash[i] >> 4];
				*result += rgbDigits[rgbHash[i] & 0xf];
			}
		}
		else
		{
			dwStatus = GetLastError();
			throw std::runtime_error("CryptGetHashParam failed: " + std::to_string(dwStatus));
		}

		std::future<void> frs = prev->get_future();
		frs.wait(); //Check that prev block is complete work
		m_instance->PushHashResult(result);
	}
	catch (std::exception &e)
	{
		m_instance->m_run = false;
		LOGERROR("Exception in StoreHashForBuff thread: " + std::string(e.what()));
	}
	catch (...)
	{
		m_instance->m_run = false;
		LOGERROR("Exception in StoreHashForBuff thread");
	}

	own->set_value();
	if (hProv)
	{
		CryptReleaseContext(hProv, 0);
	}
	if (hHash)
	{
		CryptDestroyHash(hHash);
	}
}