#pragma once
#include <vector>
#include <windows.h>
#include <mutex>
#include <queue>
#include <ctpl.h>
#include <future>

class MD5Hash
{
public:
	typedef std::vector<BYTE> TByteVec;
	typedef std::shared_ptr<TByteVec> TByteVecShare;
	typedef std::shared_ptr<std::string> TStringShare;
	typedef std::shared_ptr<std::promise<void>> TSharePrm;

public:
	MD5Hash();
	~MD5Hash();

	void CalculateHashForFile();

private:
	void AddJobToQueue(TByteVecShare buff);

	static void StoreHashForBuff(int id, TByteVecShare buff, TSharePrm prev, TSharePrm own);
	static void WriteHasToFile();

	void PushHashResult(TStringShare& str);

	bool m_run;
	std::mutex m_resultMutex;
	std::thread* m_writeThread;
	ctpl::thread_pool m_threadPool;
	std::string m_result;
	TSharePrm m_currThreadPromise;
	
	static MD5Hash* m_instance;
};

