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

MD5Hash::MD5Hash() : m_threadPool(4)
{
	m_instance = this;
}

MD5Hash::~MD5Hash()
{
}

void MD5Hash::CalculateHashForFile()
{
	unsigned bufferSize = GET_CONFIG()->GetBlockSize();
	std::string filename = GET_CONFIG()->GetInputFilePath().c_str();
	
	std::ifstream file(filename.c_str(), std::ifstream::binary);

	long long int fileSize = 0;
	long long int filePos = 0;
	if (file)
	{
		file.seekg(0, file.end);
		fileSize = file.tellg();
		file.seekg(0, file.beg);
	}
	

	TShareVecBuff buffer(new TVecBuff(bufferSize));
	while (file.good() && (fileSize - filePos) > bufferSize)
	{
		file.read((char*)buffer->data(), bufferSize);
		AddJobToQueue(buffer);
		buffer.reset(new TVecBuff(bufferSize));
		filePos = file.tellg();
	}
		
	buffer.reset(new TVecBuff());
	unsigned char readBuff;
	while (file.good())
	{
		readBuff = file.get();
		buffer->push_back(readBuff);
		if (buffer->size() >= bufferSize)
		{
			AddJobToQueue(buffer);
			buffer.reset(new TVecBuff());
		}
	}
	
	unsigned diff = bufferSize - buffer->size();
	for (unsigned i = 0; i < diff; ++i)
	{
		buffer->push_back('0');
	}
	AddJobToQueue(buffer);

	m_threadPool.stop();
}
	
void MD5Hash::AddJobToQueue(TShareVecBuff buff)
{
	if (!m_currThreadMutex.get())
	{
		m_currThreadMutex.reset(new std::promise<void>);
		m_currThreadMutex->set_value();
	}

	TShareMutex newThreadMut(new std::promise<void>);

	m_threadPool.push(StoreHashForBuff, buff, m_currThreadMutex, newThreadMut);
	m_currThreadMutex = newThreadMut;
}

void MD5Hash::StoreHashForBuff(int id, TShareVecBuff buff, TShareMutex prev, TShareMutex own)
{
	DWORD dwStatus = 0;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	BYTE rgbHash[MD5LEN];
	DWORD cbHash = 0;
	CHAR rgbDigits[] = "0123456789abcdef";

	if (!CryptAcquireContext(&hProv,
		NULL,
		NULL,
		PROV_RSA_FULL,
		CRYPT_VERIFYCONTEXT))
	{
		dwStatus = GetLastError();
		LOGERROR("CryptAcquireContext failed: " + std::to_string(dwStatus));
	}

	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
	{
		dwStatus = GetLastError();
		LOGERROR("CryptAcquireContext failed: " + std::to_string(dwStatus));
		CryptReleaseContext(hProv, 0);
	}

	if (!CryptHashData(hHash, buff->data(), buff->size(), 0))
	{
		dwStatus = GetLastError();
		printf("CryptHashData failed: %d\n", dwStatus);
		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
	}

	TShareString result(new std::string);
	cbHash = MD5LEN;
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
		LOGERROR("CryptGetHashParam failed: " + std::to_string(dwStatus));
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);

	if (m_instance)
	{
		std::future<void> frs = prev->get_future();
		frs.wait();
		m_instance->PushHashResult(result);
		own->set_value();
	}
}

void  MD5Hash::PushHashResult(TShareString& str)
{
	std::lock_guard<std::mutex> locker(m_resultMutex);
	LOG(*str);
	m_results.push(str);
}

MD5Hash::TShareString MD5Hash::PopFrontHashResult()
{
	std::lock_guard<std::mutex> locker(m_resultMutex);
	TShareString str = m_results.front();
	m_results.pop();
	return str;
}