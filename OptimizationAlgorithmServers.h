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

extern bool sortCPUOPT(STRUCTEMP SVOPT_A, STRUCTEMP SVOPT_B);
extern bool sortTempOPT(STRUCTEMP SVOP_A, STRUCTEMP SVOP_B);

class OptimizationAlgorithmServers
{
public:
	~OptimizationAlgorithmServers(void);
	virtual int OptimizationServers(int clockSimulation) = 0;

protected:
	Server* (*ppoServers)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX];
	FLOATINGPOINT HeatRecirculation[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX]; 
	ServersPOOL* opoolServers;
};

class PoliceLowUtilization : public OptimizationAlgorithmServers
{
public:
	PoliceLowUtilization(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);
	int OptimizationServers(int clockSimulation);
};

class PoliceHighTemperature : public OptimizationAlgorithmServers
{
public:
	PoliceHighTemperature(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], const FLOATINGPOINT HeatRecirculationMatrix[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], ServersPOOL* opool);
	int OptimizationServers(int clockSimulation);
};

class PoliceIdle : public OptimizationAlgorithmServers
{
public:
	PoliceIdle(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], ServersPOOL* opool);
	int OptimizationServers(int clockSimulation);
};