#pragma once

#include <iostream>
#include "Constants.h"

using namespace std;

class VirtualMachine
{
public:
	VirtualMachine(int runtime, int cputime, UINT64 jobnum);
	VirtualMachine(int runtime, int cputime, UINT64 jobnum, long int memUse);
	VirtualMachine(int runtime, int cputime, UINT64 jobnum, long int memUse, FLOATINGPOINT cpuload);
	~VirtualMachine(void);
	FLOATINGPOINT HowMuchCPULoadWillThisVMRequire();
	FLOATINGPOINT RunVMAndReturnActualTime(FLOATINGPOINT sec);
	bool IsFinished();
	UINT64 GetJobNumber();
	FLOATINGPOINT GetCPULoadRatio();
	long int GetMemUseVM();
	void InsertTimeSecMove(FLOATINGPOINT TimeFinish);
	FLOATINGPOINT ReturnTimeSecMove();
	bool ReturnIsMove();
	bool ListVM();
	inline FLOATINGPOINT ReturnRunTimeSec() {return runTimeSec; }
	inline FLOATINGPOINT ReturnAvgCPUTimeSec() { return avgCPUTimeSec; }
	inline long int ReturnMemUseVM() { return memUseVM; }
	inline UINT64 ReturnJobNumber() { return jobNumber; }
	inline FLOATINGPOINT ReturncpuLoadRatio() {return cpuLoadRatio;}
private:
	FLOATINGPOINT runTimeSec;
	FLOATINGPOINT avgCPUTimeSec;
	FLOATINGPOINT cpuLoadRatio;
	long int memUseVM;
	UINT64 jobNumber;
	bool isFinished;
	bool isMove;
	FLOATINGPOINT TimeMoveFinish;
};

