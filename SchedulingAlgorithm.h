#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <queue>

#include "Constants.h"
#include "Server.h"
#include "VirtualMachine.h"
#include "PoolServers.h"

using namespace std;

extern int NUMBER_OF_SERVERS_IN_ONE_CHASSIS;
extern int NUMBER_OF_CHASSIS;
extern int NUMBER_OF_CORES_IN_ONE_SERVER;
extern long CLENGTH;


extern void quickSort(SORTSERVER *server, int begin, int end);
extern vector<double> runRBF(vector<double> vetorPredicao);
extern vector<double> runPolynom(vector<double> vetorPredicao);

class SchedulingAlgorithm
{
public:
	~SchedulingAlgorithm(void);
	virtual void AssignVMs() = 0;
	virtual int returnTotalScheduling(void) = 0;

protected:
	Server* (*ppServers)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX];
	queue<VirtualMachine*>* pqVMsToGo;
	const FLOATINGPOINT (*pHeatRecirculationMatrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX];
	ServersPOOL* ppollServers;
	int totalScheduling;
};

class LowTemperatureFirstSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	LowTemperatureFirstSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX]);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
};

class UniformTaskSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	UniformTaskSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX]);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
};

class BestPerformanceSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	BestPerformanceSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX]);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
};

class RandomSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	RandomSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX]);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
};

class MinHRSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	MinHRSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX]);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }

private:
	FLOATINGPOINT HRF[SIZE_OF_HR_MATRIX];
	int HRFSortedIndex[SIZE_OF_HR_MATRIX];
};

class XintSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	XintSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX]);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }

private:
	FLOATINGPOINT GetHighestTemperatureIncrease();
};

class CenterRackFirstSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	CenterRackFirstSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX]);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }

private:
	int HRFSortedIndex[SIZE_OF_HR_MATRIX];

};

class TwoDimensionWithPoolSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	TwoDimensionWithPoolSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], ServersPOOL* ppool);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }

private:
	FLOATINGPOINT HRF[SIZE_OF_HR_MATRIX];
};

class TwoDimensionWithPoolAndPredictionSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	TwoDimensionWithPoolAndPredictionSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], ServersPOOL* ppool);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
private:
	FLOATINGPOINT HRF[SIZE_OF_HR_MATRIX];
};


class TwoDimensionWithPredictionSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	TwoDimensionWithPredictionSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX]);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
private:
	FLOATINGPOINT HRF[SIZE_OF_HR_MATRIX];
};