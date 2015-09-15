#pragma once

#include <list>
#include <vector>

#include "Constants.h"
#include "Switches.h"
#include "Rack.h"
#include "Server.h"

using namespace std;

extern int NUMBER_OF_CHASSIS;
extern int NUMBER_OF_SERVERS_IN_ONE_CHASSIS;

extern int SWITCH_CORE;
extern int SWITCH_AGGREGATION; 	
extern int SWITCH_ACCESS;

extern int NUMBER_CHASSI_RACK;

extern int LEVEL_OF_REDUNDANCY_OF_LINKS_N0N1;	
extern int LEVEL_OF_REDUNDANCY_OF_LINKS_N1N2;		
extern int LEVEL_OF_REDUNDANCY_OF_LINKS_N2N3;

extern int NUMBER_OF_PORTS_SWITCH_ACCESS;
extern int NUMBER_OF_PORTS_SWITCH_AGGREGATION;
extern int NUMBER_OF_PORTS_SWITCH_CORE;

extern int SWITCH_ACCESS_SPEED; 
extern double SWITCH_ACCESS_DELAY; 

extern int SWITCH_AGGREGATION_SPEED; 
extern double SWITCH_AGGREGATION_DELAY; 

extern int SWITCH_CORE_SPEED;
extern double SWITCH_CORE_DELAY;

extern double ECORE_CHASSIS; 
extern double ECORE_LINECARD; 
extern double ECORE_PORT;

extern double EAGGR_CHASSIS; 
extern double EAGGR_LINECARD; 
extern double EAGGR_PORT;

extern double EACCE_CHASSIS; 
extern double EACCE_LINECARD; 
extern double EACCE_PORT;

extern int PERIODIC_LOG_INTERVAL;

class Topology 
{
public:
	Topology(void);
	~Topology(void);
	void CreateTopology(Server* (*tservers)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);
	void EveryASecond(int clock_);
	inline double ReturnTotalEnergyAccessSwitch(void) { return vTotalEnergyAccessSwitch; }
	inline double ReturnTotalEnergyAggregationSwitch(void) { return vTotalEnergyAggregationSwitch; }
	inline double ReturnTotalEnergyCoreSwitch(void) { return vTotalEnergyCoreSwitch; }
	inline int ReturnSizeVectorTotalEnergyAccessSwitch(void) { return vTotalEnergyAccessSwitchSparseLog.size(); }
	inline int ReturnSizeVectorTotalEnergyAggregationSwitch(void) { return vTotalEnergyAggregationSwitchSparseLog.size(); }
	inline int ReturnSizeVectorTotalEnergyCoreSwitch(void) { return vTotalEnergyCoreSwitchSparseLog.size(); }
	double ReturnTotalEnergyAccessSwitchSparseLog(void);
	double ReturnTotalEnergyAggregationSwitchSparseLog(void);
	double ReturnTotalEnergyCoreSwitchSparseLog(void);
	void PrintTotalEnergyAccessSwitchSparseLog(void);
	void PrintTotalEnergyAggregationSwitchSparseLog(void);
	void PrintTotalEnergyCoreSwitchSparseLog(void);
	void PrintTotalEnergySwitchesSparseLog(void);
	void PrintPowerDrawAccessSwitch(void);
	void PrintPowerDrawAggregationSwitch(void);
	void PrintPowerDrawCoreSwitch(void);
	void PrintTotalPowerDrawSwitches(void);
	double ReturnPowerDrawSwitches(int ind);
private:
	Server* (*tpServers)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX];
	vector<Switch*> Switches;
	vector<Rack*> Racks;

	vector<FLOATINGPOINT> vTotalEnergyAccessSwitchSparseLog;
	vector<FLOATINGPOINT> vTotalEnergyAggregationSwitchSparseLog;
	vector<FLOATINGPOINT> vTotalEnergyCoreSwitchSparseLog;

	FLOATINGPOINT vTotalEnergyAccessSwitch;
	FLOATINGPOINT vTotalEnergyAggregationSwitch;
	FLOATINGPOINT vTotalEnergyCoreSwitch;

	FLOATINGPOINT vEnergyAccessSwitch;
    FLOATINGPOINT vEnergyAggregationSwitch;
    FLOATINGPOINT vEnergyCoreSwitch; 

	int locSwitchAccess;
	int locSwitchAggregation;
	int locSwitchCore;
};

