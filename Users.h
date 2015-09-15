#pragma once

#include <vector>

#include "Constants.h"
#include "JobQueue.h"

using namespace std;

class SWFLine {
public:
	UINT64 jobNumber;
	int submitTimeSec;
	int waitTimeSec;
	int runTimeSec;
	int numCPUs;
	int avgCPUTimeSec;
	long int usedMemKB;
	int numCPUsRequested;
	int runTimeSecRequested;
	int usedMemKBRequested;
	int status;
	int userID;
	int groupID;
	int exefileID;
	int queueNum;
	int partitionNum;
	UINT64 dependencyJobNum;
	int thinkTimeAfterDependencySec;
};

class Users
{
public:
	Users(JobQueue* jq, vector<SWFLine*>* swf);
	~Users(void);
	void EveryASecond();
	bool IsFinished();
	int LastJob();
	
private:
	JobQueue* pJobQueue;
	vector<SWFLine*>* pvSWFToGo;
	int clock;

};

