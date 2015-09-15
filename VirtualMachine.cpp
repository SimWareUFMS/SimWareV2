#include "VirtualMachine.h"

UINT64 VirtualMachine::GetJobNumber()
{
	return jobNumber;
}

VirtualMachine::VirtualMachine(int runtime, int cputime, UINT64 jobnum)
{
	if (cputime < 0) // no information about cputime mexi aqui era < passei para <=
		cputime = runtime; // assuming 100% utilization
	runTimeSec = (FLOATINGPOINT)runtime;
	avgCPUTimeSec = (FLOATINGPOINT)cputime;
	cpuLoadRatio = avgCPUTimeSec/runTimeSec;
	memUseVM = 0;
	isFinished = true;
	if (runTimeSec > 0.0)
		isFinished = false;
	jobNumber = jobnum;
	isMove = false;
	TimeMoveFinish = 0.0;
}

VirtualMachine::VirtualMachine(int runtime, int cputime, UINT64 jobnum, long int memUse)
{
	if (cputime < 0) // no information about cputime 
		cputime = runtime; // assuming 100% utilization
	runTimeSec = (FLOATINGPOINT)runtime;
	avgCPUTimeSec = (FLOATINGPOINT)cputime;
	cpuLoadRatio = avgCPUTimeSec/runTimeSec;
	memUseVM = memUse;
	isFinished = true;
	if (runTimeSec > 0.0)
		isFinished = false;
	jobNumber = jobnum;
	isMove = false;
	TimeMoveFinish = 0.0;
}

VirtualMachine::VirtualMachine(int runtime, int cputime, UINT64 jobnum, long int memUse, FLOATINGPOINT cpuload)
{
	runTimeSec = (FLOATINGPOINT)runtime;
	avgCPUTimeSec = (FLOATINGPOINT)cputime;
	cpuLoadRatio = cpuload;
	memUseVM = memUse;
	isFinished = false;
	jobNumber = jobnum;
	isMove = false;
	TimeMoveFinish = 0.0;
}


VirtualMachine::~VirtualMachine(void)
{
}

FLOATINGPOINT VirtualMachine::HowMuchCPULoadWillThisVMRequire()
{
	return cpuLoadRatio;
}

FLOATINGPOINT VirtualMachine::RunVMAndReturnActualTime(FLOATINGPOINT sec)
{
	if (isFinished)
		cout << "Error: Finished VM consumed cpu time!!!! runTimeSec =  " << runTimeSec << "  avgCPUTimeSec = " << avgCPUTimeSec << " sec " << sec << endl;

	if (runTimeSec < sec) {
		isFinished = true;
		runTimeSec = avgCPUTimeSec = 0.0;
		return runTimeSec;
	}
	runTimeSec -= sec;
	avgCPUTimeSec -= sec*cpuLoadRatio;
	return sec;
}

bool VirtualMachine::IsFinished()
{
	return isFinished;
}

FLOATINGPOINT VirtualMachine::GetCPULoadRatio()
{
	return cpuLoadRatio;
}

long int VirtualMachine::GetMemUseVM()
{
	return memUseVM;
}


void VirtualMachine::InsertTimeSecMove(FLOATINGPOINT TimeFinish)
{
  isMove = true;
  TimeMoveFinish = TimeFinish;
}

FLOATINGPOINT VirtualMachine::ReturnTimeSecMove()
{
  return TimeMoveFinish;
}

bool VirtualMachine::ReturnIsMove()
{
  return isMove;
} 

bool VirtualMachine::ListVM()
{
 cout << "jobNumber " << jobNumber << " runTimeSec " << runTimeSec << " avgCPUTimeSec " << avgCPUTimeSec << " cpuLoadRatio " << cpuLoadRatio << " memUseVM " << memUseVM << " isFinished " << isFinished << " isMove " << isMove << " TimeMoveFinish " << TimeMoveFinish;
 cout << endl; 
 return true;
}




