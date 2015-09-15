#pragma once

#include <iostream>
#include <vector>
#include "Constants.h"
#include "Server.h"

using namespace std;

extern bool sortPOOLServer(POOL S_A, POOL S_B);
extern int SIZEWINDOWNPREDICTIONPOOLSERVER;
extern int NUMBER_OF_CHASSIS;
extern int NUMBER_OF_SERVERS_IN_ONE_CHASSIS;

extern vector<double> runRBF(vector<double> vetorPredicao);
extern vector<double> runPolynom(vector<double> vetorPredicao);

extern bool sortTemperature(POOL SV_A, POOL SV_B);

class ServersPOOL
{
public:
	ServersPOOL(void);
    ~ServersPOOL(void);
	void InsertVectorServersPOOL(int Chassi, int Server, double Temperature);
	void InsertVectorServersPowerOFF(int Chassi, int Server, double Temperature);
	void InsertVectorServersPowerON(int Chassi, int Server, double Temperature);
	void ListVectorServersPOOL(void);
	void ListVectorPowerOFF(void);
	void SortVectorServersPowerOFF(void);
	void SortVectorServersPOOL(void);
	void ServerPowerON(Server* (*sv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], int wakeUP);
	POOL RemoveServerPOOL(Server* (*sv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);
	inline int ReturnMinimumServersPool(void) { return minimumServersPool; }
	void DefineNewSizePool(void);
	void EveryASecond(Server* (*psv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);
	void InsertWindowsSmallVMSArrived(int VMsSmall);
	void InsertWindowsMediumVMSArrived(int VMsMedium);
	void InsertWindowsBigVMSArrived(int VMsBig);
	void UpdateClockSimulation(int clocksimul);
	void PowerOFFDC(Server* (*psv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);
	inline int ReturnSizePool(void) { return serversPOOL.size(); }
	void Print(void);
private:
	int clockSimulation;
    vector<POOL> serversPowerOFF;
	vector<POOL> serversPowerON;
	vector<POOL> serversPOOL;
	vector<double> windowsSmallVMSArrived;
	vector<double> windowsMediumVMSArrived;
	vector<double> windowsBigVMSArrived;
	int minimumServersPool;
};
