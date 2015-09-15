#pragma once

#include <vector>
#include <math.h>

#include "Constants.h"
#include "VirtualMachine.h"
#include "Links.h"

using namespace std;

extern bool TEMPERATURE_SENSING_PERFORMANCE_CAPPING;
extern int TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS;
extern int NUMBER_OF_CORES_IN_ONE_SERVER;
extern bool CONSTANT_FAN_POWER;
extern bool FAN_RPM_NO_LIMIT;
extern long int TOTAL_OF_MEMORY_IN_ONE_SERVER;
extern int SIZEWINDOWNPREDICTION;
extern int MONITORINGTIME;
extern bool PREDICITIONSHEDULER;
extern int TRANSFER_TIME_VMS;
extern int POWER_ON;

extern bool sortCPU(VirtualMachine* VM_A, VirtualMachine* VM_B);

class Server
{
public:
	Server(unsigned int cpuGen);
	~Server(void);
	void RecalculatePerformanceByTemperature();
	FLOATINGPOINT GetPowerDraw();
	FLOATINGPOINT GetFanPower();
	void EveryASecond(int clockSimul);
	bool IsFinished();
	FLOATINGPOINT AvailableUtilization();
	FLOATINGPOINT CurrentUtilization();
	void AssignOneVM(VirtualMachine *);
	void RemoveTheLastAssignedVM();
	vector<VirtualMachine *>* GetFinishedVMVector();
	FLOATINGPOINT VMRequiresThisMuchUtilization();
	FLOATINGPOINT VMRequiresThisMuchCPUScale();
	long int VMRequiresThisMemory();
	FLOATINGPOINT MaxUtilization();
	void TurnOFF();
	void TurnON();
	inline bool IsOFF() { return isOFF; }
	inline bool IsPOOL() { return isPOOL; }
	inline bool IsMIGRATING() { return isMigrating; }
	inline bool IsINITIALIZING() { return isInitializing; }
	VirtualMachine* TakeAVM();
	void AddHeatToTimingBuffer(FLOATINGPOINT temperature, int timing);
	void SetSupplyTempToTimingBuffer(FLOATINGPOINT temperature, int timing);
	unsigned int HowManySecondsOverEmergencyTemp();
	unsigned int HowManyTimesDVFSChanged();
	FLOATINGPOINT CurrentInletTemperature();
	FLOATINGPOINT CurrentAddedTemperature();
	void SetCPUGeneration(unsigned int gen);
	FLOATINGPOINT returnPositionVectorWorkLoadServer(int k); 
	long int returnPositionVectorMemoryUseOfServer(int k); 
	FLOATINGPOINT returnPositionVectorUtilizationCPU(int k);
	int returnSizeVectorWorkLoadServer(void);
	int returnSizeVectorMemoryUseOfServe(void);
	int returnSizeVectorUtilizationCPU(void);
	int returnSizeVectorTemperature(void);
	void insertVectorUtilizationCPU(FLOATINGPOINT utilCPU);
	void insertVectorUtilizationMemory(long int utilMemory);
	void insertVectorTemperature(FLOATINGPOINT tempServer);
    void insertTemperaturePredictionServer(FLOATINGPOINT temperature);
	void insertTimePredictionServer(unsigned int timeprediction);
	vector<FLOATINGPOINT> returnVectorTemperature(void);
	vector<FLOATINGPOINT> returnVectorCPU(void);
	unsigned int returnClock(void);
	void addErrorPrediction(void);
	void addHitPrediction(void);
	unsigned int returnErrorPrediction(void);
	unsigned int returnHitPrediction(void);
	unsigned int returnFirstTimePredictionServer(void);
	void calculateRMSE(void);
	FLOATINGPOINT returnRMSE( void);
	FLOATINGPOINT returnErrorMean(void);
	FLOATINGPOINT returnVarianceErrorMean(void);
	FLOATINGPOINT returnSDErrorMean(void);
	void SortVectorVM(void);
	void ListVMs(void);
	void addPOOL(void);
	void removePOOL(void);
	void MoveVMs(vector<VirtualMachine *> VMs);
	vector<VirtualMachine *> GetALLVMs(int ClockSim);
	vector<VirtualMachine *> GetNVMs(int N, int ClockS);
	int  HowManyVMs();
	bool hasVMs();
	void CheckFinishMove(int clockSim);
	inline void setMigrating(bool Migrat) { isMigrating = Migrat; }
	inline bool ReturnMigrating() { return isMigrating; }
	void FinishInitialization();
	FLOATINGPOINT ReadSupplyTempToTimingBuffer(); 
	void timePowerOnServer(int clockSimul);
	int returnFinishPowerOnServer(void);
	void insertRackInServer(int Rck);
	void insertLinkInServer(Link_SV_to_SW* lk);
	
private:
	// Definição dos Vetores de predição 
	vector<FLOATINGPOINT> workLoadServer;
	vector<long int> memoryUseOfServer;
	vector<FLOATINGPOINT> utilizationCPU;
	vector<FLOATINGPOINT> temperatureServer;
	vector<FLOATINGPOINT> temperaturePredictionServer;
	vector<unsigned int> timePredictionServer;
	vector<FLOATINGPOINT> varianceErrorMean;


    FLOATINGPOINT RMSE;
	FLOATINGPOINT errorMean;
	int numberElements;
	unsigned int errorPrediction;
	unsigned int hitPrediction;
	unsigned int cpuGeneration;
	unsigned int clock;
	unsigned int endTimeServerActive;
	long int memServer;
	unsigned int howManySecondsOverEmergencyTemp;
	unsigned int howManyTimesDVFSChanged;
	bool isOFF;
	bool isPOOL;
	bool isMigrating;
	bool isInitializing;

	vector<VirtualMachine *> vRunningVMs;
	vector<VirtualMachine *> vFinishedVMs;
	vector<VirtualMachine *> vMoveVMs;

	FLOATINGPOINT currentPowerDraw;
	FLOATINGPOINT currentFanPowerDraw;
	FLOATINGPOINT currentCPUPowerFactor;
	FLOATINGPOINT currentPerformanceFactor;
	int currentPerformanceStateOutof100;
	FLOATINGPOINT supplyTempTimingBuffer[SIZE_OF_HEAT_TIMING_BUFFER];
	FLOATINGPOINT additionalHeatTimingBuffer[SIZE_OF_HEAT_TIMING_BUFFER];
	FLOATINGPOINT cpuTDP;
	FLOATINGPOINT coolerMinTemperatureGap;
	FLOATINGPOINT coolerMaxRPM;
	FLOATINGPOINT coolerMinRPM;
	FLOATINGPOINT coolerMaxDieTemperature;
	FLOATINGPOINT coolerMaxPower;
	void ClockPlusPlus();
	void CalculatePowerDraw(FLOATINGPOINT utilization, FLOATINGPOINT temperature);
	FLOATINGPOINT ReadHeatFromTimingBuffer();
	int powerOnServer;
	int rack;
	vector<Link_SV_to_SW*> link;
};
