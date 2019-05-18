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
	typedef std::vector<BYTE> TVecBuff;
	typedef std::shared_ptr<TVecBuff> TShareVecBuff;

	typedef std::shared_ptr<std::string> TShareString;
	typedef std::queue<TShareString> TQueShareStrings;

	typedef std::shared_ptr<std::promise<void>> TShareMutex;

public:
	MD5Hash();
	~MD5Hash();

	void CalculateHashForFile();

private:
	void AddJobToQueue(TShareVecBuff buff);

	static void StoreHashForBuff(int id, TShareVecBuff buff, TShareMutex prev, TShareMutex own);
	void PushHashResult(TShareString& str);
	TShareString PopFrontHashResult();

	TQueShareStrings m_results;
	std::mutex m_resultMutex;
	std::mutex m_jobsMutex;
	ctpl::thread_pool m_threadPool;
	TShareMutex m_currThreadMutex;

	static MD5Hash* m_instance;
};

