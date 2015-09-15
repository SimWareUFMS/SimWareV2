#pragma once

#include <iostream>
#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#include <boost/algorithm/string.hpp>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>
#include <queue>

#include "Constants.h"

using namespace std;

extern int PERIODIC_LOG_INTERVAL;

class SingleJob {
public:
	UINT64 jobNumber;
	int numCPUs;
	int runTimeSec;
	int runTimeSecFromSWF;
	int avgCPUTimeSec;
	long int usedMemKB;
	int finishedVMs;
	int queuedClock;
	int finishedClock;
	queue<float> physicalNodeNumber;
	SingleJob(UINT64 jobNum, int numCPU, int runTime, int avgCPUTime, int qClock) {
		jobNumber = jobNum;
		numCPUs = numCPU;
		runTimeSec = runTime;
		runTimeSecFromSWF = runTime;
		avgCPUTimeSec = avgCPUTime;
		finishedVMs = 0;
		queuedClock = qClock;
		finishedClock = -1;
	}
	SingleJob(UINT64 jobNum, int numCPU, int runTime, int avgCPUTime, int qClock, long int usedMem) {
		jobNumber = jobNum;
		numCPUs = numCPU;
		runTimeSec = runTime;
		runTimeSecFromSWF = runTime;
		avgCPUTimeSec = avgCPUTime;
		finishedVMs = 0;
		queuedClock = qClock;
		finishedClock = -1;
		usedMemKB = usedMem;
	}
};

class JobQueue
{
public:
	JobQueue(void);
	~JobQueue(void);
	bool IsFinished();
	SingleJob* TakeFirst();
	void InsertASingleJob(UINT64 jobNum, int numCPU, int runTime, int avgCPUTime, int qClock);
	void InsertASingleJob(UINT64 jobNum, int numCPU, int runTime, int avgCPUTime, int qClock, long int usedMem);
	void OneVMFinishedOnASingleJob(UINT64 jobNum, int clock, float nodeNum);
	bool IsTheSingleJobFinished(UINT64 jobNum);
	void PrintResults(int endedClock);

private:
	map<UINT64, SingleJob*> mapSingleJob;
	map<UINT64, SingleJob*> mapRunningSingleJob;
	typedef pair<UINT64, SingleJob*> mapPair;
	typedef map<UINT64, SingleJob*>::iterator mapIterator;
	vector<UINT64> vFinishedJobID;
	UINT64 averageLatencyNumberOfSamples;
	FLOATINGPOINT averageLatencySum;
	UINT64 averageLatencyPerIntervalNumberOfSamples[MAX_NUMBER_OF_ELEMENTS_LOG_INTERVAL];
	FLOATINGPOINT averageLatencyPerIntervalSum[MAX_NUMBER_OF_ELEMENTS_LOG_INTERVAL];
	FLOATINGPOINT averageLatencyComparedToSWFInPercentage[MAX_NUMBER_OF_ELEMENTS_LOG_INTERVAL];
};

