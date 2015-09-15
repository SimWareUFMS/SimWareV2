#include "Server.h"
#include <vector> 
#include <algorithm>

Server::Server(unsigned int cpuGen)
{
	clock = 0;
	howManySecondsOverEmergencyTemp = 0;
	howManyTimesDVFSChanged = 0;
	currentPerformanceStateOutof100 = 100;
	currentPowerDraw = 0.0;
	currentFanPowerDraw = 0.0;
	vRunningVMs.clear();
	vFinishedVMs.clear();
	currentCPUPowerFactor = 1.0;
	currentPerformanceFactor = 1.0;
	isOFF = false;
	isPOOL = false;
	isMigrating = false;
	isInitializing = false;
	rack = 0;
    link.clear();

	for (int i=0; i<SIZE_OF_HEAT_TIMING_BUFFER; ++i) {
		additionalHeatTimingBuffer[i] = 0.0;
		supplyTempTimingBuffer[i] = LOWEST_SUPPLY_TEMPERATURE;
	}

	cpuGeneration = cpuGen;
	switch (NUMBER_OF_CORES_IN_ONE_SERVER) {
	case 2: case 4:
		cpuTDP = 80.0; // Xeon E5502 / Xeon E3-1235
		break;
	case 8:
		cpuTDP = 105.0; // Xeon E3-2820
		break;
	case 10:
		cpuTDP = 130.0; // Xeon E7-2850
		break;
	case 16:
		cpuTDP = 210.0; // 8 cores X2
		break;
	default:
		cpuTDP = 100.0;
	}
	if (cpuTDP < 1.0)
		cout << "Error: cpu tdp is less than 1W" << endl;

	for (unsigned int i=0; i<cpuGeneration; ++i) {
		cpuTDP *= 0.8;
	}

	coolerMaxRPM = 3000.0;
	coolerMinRPM = 500.0;
	coolerMaxPower = 15.0;
	coolerMaxDieTemperature = 70.0;
	coolerMinTemperatureGap = coolerMaxDieTemperature - EMERGENCY_TEMPERATURE;

	memServer = TOTAL_OF_MEMORY_IN_ONE_SERVER;
	errorPrediction = 0;
	hitPrediction = 0;
	RMSE = 0.00;
	numberElements=0;
	errorMean = 0.0;
	powerOnServer = 0;
}

Server::~Server(void)
{
  vRunningVMs.clear();
  vFinishedVMs.clear();
  link.clear();
}

void Server::TurnOFF()
{
	isOFF = true;
	isMigrating = false;
	isPOOL = false;
	isInitializing = false;
	
	for (vector<Link_SV_to_SW *>::iterator it = link.begin(); it != link.end(); ++it) {
	  if ((*it)->IsActive()) {
	     (*it)->PowerOFF();
	  }
    }
}


void Server::TurnON()
{
	isOFF = false;
	isMigrating = false;
	isPOOL = false;
	isInitializing = true;

	for (vector<Link_SV_to_SW *>::iterator it = link.begin(); it != link.end(); ++it) {
	  if (!(*it)->IsActive()) {
	     (*it)->PowerON();
	  }
    }
}

void Server::FinishInitialization()
{
	isInitializing = false;
}

FLOATINGPOINT Server::VMRequiresThisMuchUtilization()
{
	if (isOFF) {
	   return 0.0;
	}
	if (isInitializing) {
  	   return 0.30; // searching the average use of a cpu
	}

	FLOATINGPOINT sum = 0.0;
	for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it) {
  		sum += (*it)->HowMuchCPULoadWillThisVMRequire(); // retorna cpuLoadRatio
	}
	return sum/NUMBER_OF_CORES_IN_ONE_SERVER; // this can be more than (1.0)
}

FLOATINGPOINT Server::VMRequiresThisMuchCPUScale()
{
	if (isOFF) {
	   return 0.0;
	}
	if (isInitializing) {
		return 5;  // searching the average use of a cpu
	}

	FLOATINGPOINT sum = 0.0;

	for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it)
		sum += (*it)->HowMuchCPULoadWillThisVMRequire(); // retorna cpuLoadRatio

	return sum; // this can be more than (NUMBER_OF_CORES_IN_ONE_SERVER)
}

long int Server::VMRequiresThisMemory()
{
	if (isOFF) {
	   return 0;
	}

	long int sum = 0;
	for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it)
		sum += (*it)->GetMemUseVM();
	return sum; 
}

FLOATINGPOINT Server::GetPowerDraw()
{
	if (isOFF) {
	   return 0.0;
	}
	else {
	   return currentPowerDraw;
	}
}

FLOATINGPOINT Server::GetFanPower()
{
	if (isOFF) {
	   return 0.0;
	}
	else {
	   return currentFanPowerDraw;
	}
}

FLOATINGPOINT Server::MaxUtilization()
{
	if (isOFF) {
	   return 0.0;
	}

	return (FLOATINGPOINT)currentPerformanceStateOutof100/100.0;
}

FLOATINGPOINT Server::CurrentUtilization()
{
	if (isOFF) {
	   return 0.0;
	}

	FLOATINGPOINT required = VMRequiresThisMuchUtilization();
	FLOATINGPOINT max = MaxUtilization();
	return ((required > max) ? max : required);
}

FLOATINGPOINT Server::AvailableUtilization()
{
	if (isOFF) { 
	   return 0.0;
	}

	FLOATINGPOINT avail = MaxUtilization() - CurrentUtilization();

	if (avail < 0.0)
		avail = 0.0;

	return avail;
}

void Server::EveryASecond(int clockSimul) 
{
	if (isOFF) {
		ClockPlusPlus();
		return;
	}

	if ((isPOOL) || (isInitializing)) {
		// Recalculate how much this server can perform
	    RecalculatePerformanceByTemperature();
		CalculatePowerDraw(CurrentUtilization(), CurrentInletTemperature());
		ClockPlusPlus();
		return;
	}

	// Recalculate how much this server can perform
	RecalculatePerformanceByTemperature();

	// run VMs
	FLOATINGPOINT sumUtil = VMRequiresThisMuchUtilization();
	FLOATINGPOINT maxUtil = MaxUtilization();
	long int sumMemory = VMRequiresThisMemory();

	if (PREDICITIONSHEDULER) {
	   // Monitor utilization CPU/Temperature
	   if (clockSimul%MONITORINGTIME == 0) {
	      insertVectorTemperature(CurrentInletTemperature());
 	   }
	   if (temperatureServer.size() > SIZEWINDOWNPREDICTION) {	    // Prediction Window  
          temperatureServer.erase(temperatureServer.begin());
	   }   
	}

	if (sumUtil <= maxUtil) { // run every VM for a sec
		for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it)
			(*it)->RunVMAndReturnActualTime(1.0);
	}
	else { // partially run every VM
		for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it) {
			//cout << "Execução Parcial " << maxUtil/sumUtil << " maxUtil = " << maxUtil << " sumUtil = " << sumUtil << endl; 
			(*it)->RunVMAndReturnActualTime(maxUtil/sumUtil);
		}
	}

	// detect finished VMs. TODO: this goto statement is correct but ugly
	BEFORE_DEFINING_ITERATOR:
	for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it) {
		if ((*it)->IsFinished()) {
			vFinishedVMs.push_back((*it));
			vRunningVMs.erase(it);
			goto BEFORE_DEFINING_ITERATOR;
		}
	}

	CalculatePowerDraw(CurrentUtilization(), CurrentInletTemperature());

	ClockPlusPlus();
}

void Server::ClockPlusPlus()
{
	// empty current additionalHeatTimingBuffer
	unsigned int slotIndex = clock % SIZE_OF_HEAT_TIMING_BUFFER;
	additionalHeatTimingBuffer[slotIndex] = 0.0;

	clock++;
}

void Server::MoveVMs(vector<VirtualMachine *> VMs)
{

  if ((isOFF) || (isPOOL) || (isInitializing)) {
     cout << "Error: Get Finished VM Vector called to a turned off or pool or initializing server !!!" << " Power OFF = " << isOFF << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
  }
  else {
     for (int i=0; i < VMs.size(); i++) {
		 vRunningVMs.push_back(VMs[i]);
     }
     CalculatePowerDraw(CurrentUtilization(), CurrentInletTemperature());
  }
}
  
vector<VirtualMachine *> Server::GetALLVMs(int ClockSim)
{
  vector<VirtualMachine *> VMsTemp;
  
  if ((isOFF) || (isPOOL) || (isInitializing)) {
     cout << "Error: Get Finished VM Vector called to a turned off or pool or initializing server !!!" << " Power OFF = " << isOFF << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
 	 return VMsTemp;
  }

  for(int i=0; i < vRunningVMs.size(); i++){
	  if ((vRunningVMs[i]->ReturnRunTimeSec() > 0) && (!vRunningVMs[i]->IsFinished())) {
    	 VMsTemp.push_back(new VirtualMachine(vRunningVMs[i]->ReturnRunTimeSec(),vRunningVMs[i]->ReturnAvgCPUTimeSec(),vRunningVMs[i]->ReturnJobNumber(), vRunningVMs[i]->ReturnMemUseVM(), vRunningVMs[i]->ReturncpuLoadRatio()));
         vRunningVMs[i]->InsertTimeSecMove(ClockSim+TRANSFER_TIME_VMS);
	  }
  }

  return VMsTemp;
}

vector<VirtualMachine *> Server::GetNVMs(int N,int ClockS)
{
  vector<VirtualMachine *> VMTemp;

  if ((isOFF) || (isPOOL) || (isInitializing)) {
     cout << "Error: Get Finished VM Vector called to a turned off or pool or initializing server !!!" << " Power OFF = " << isOFF << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
 	 return VMTemp;
  }
 
  SortVectorVM(); 

  if (N <= vRunningVMs.size()) {
	  for (int i=0; i < N; i++) {
		   VMTemp.push_back(new VirtualMachine(vRunningVMs[i]->ReturnRunTimeSec(),vRunningVMs[i]->ReturnAvgCPUTimeSec(),vRunningVMs[i]->ReturnJobNumber(), vRunningVMs[i]->ReturnMemUseVM()));
		   vRunningVMs[i]->InsertTimeSecMove(ClockS+TRANSFER_TIME_VMS);
	  }
  }
  
  return VMTemp;
}
void Server::CheckFinishMove(int clockSim)
{
  if ((isOFF) || (isPOOL) || (isInitializing)) {
     cout << "Error: Check Finish Move called to a turned off or pool or initializing server !!!" << " Power OFF = " << isOFF << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
     return; 
  }

  INI:
  for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it) {
      if ((*it)->ReturnIsMove()) {
         if ((*it)->ReturnTimeSecMove() == clockSim){
			vFinishedVMs.push_back((*it));
			vRunningVMs.erase(it);
			goto INI;
		 }
	  }
  }
  
  CalculatePowerDraw(CurrentUtilization(), CurrentInletTemperature());
}

bool Server::hasVMs()
{
	if (vRunningVMs.size() > 0) {
	   return true;
	}
	else {
	   return false;
	}
}

int  Server::HowManyVMs()
{
	return vRunningVMs.size();
}

void Server::AssignOneVM(VirtualMachine *vm)
{
	if ((isOFF) || (isPOOL) || (isInitializing)) {
   	   cout << "Error: VM assigned to a turned off or POOL or initializing server !!!" << " Power OFF = " << isOFF << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
	}

	vRunningVMs.push_back(vm);

	CalculatePowerDraw(CurrentUtilization(), CurrentInletTemperature());
}



VirtualMachine* Server::TakeAVM()
{
  if ((isOFF) || (isPOOL) || (isInitializing)) {
     cout << "Error: Take VM called to a turned off or POOL or initializing server !!!" << " Power OFF = " << isOFF << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
  }

  if (vRunningVMs.empty()) {
	  return NULL;
  }
  
  VirtualMachine* retVal = vRunningVMs.back();
  
  vRunningVMs.pop_back();
  
  CalculatePowerDraw(CurrentUtilization(), CurrentInletTemperature());
 
  return retVal;
}

void Server::RemoveTheLastAssignedVM()
{
	if ((isOFF) || (isPOOL) || (isInitializing)) {
	   cout << "Error: VM removed from a turned off or POOL or initializing server !!!" << " Power OFF = " << isOFF << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
	}
	vRunningVMs.pop_back();
}

void Server::SortVectorVM(void)
{
  sort(vRunningVMs.begin(), vRunningVMs.end(), sortCPU);
}

void Server::ListVMs(void)
{
	if ((isOFF) || (isPOOL) || (isInitializing)) {
	   cout << "Error: List VMs called to a turned off or POOL or initializing server !!!" << " Power OFF = " << isOFF << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
	}

	SortVectorVM();

	for (vector<VirtualMachine*>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it) {
		 cout << (*it)->ListVM();
	}
}
 
vector<VirtualMachine *>* Server::GetFinishedVMVector()
{
	if ((isOFF) || (isPOOL) || (isInitializing)) {
	   cout << "Error: Get Finished VM Vector called to a turned off or POOL or initializing server !!!" << " Power OFF = " << isOFF << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
	}

	return &vFinishedVMs;
}

void Server::RecalculatePerformanceByTemperature()
{
	if (isOFF) {
	   cout << "Error: SetInletTemperature called to a turned off" << endl;
	}

	int oldPerformanceStateOutof100 = currentPerformanceStateOutof100;

	FLOATINGPOINT inletTempNow = CurrentInletTemperature();

	if (inletTempNow <= (EMERGENCY_TEMPERATURE))
		currentPerformanceStateOutof100 = 100;
	else {
		currentPerformanceStateOutof100 = 100;
		howManySecondsOverEmergencyTemp++;
	}

	if (TEMPERATURE_SENSING_PERFORMANCE_CAPPING)
	{
		// 1/30 CPU power down for every 1'c up
		currentCPUPowerFactor = 1.0 - (inletTempNow-(EMERGENCY_TEMPERATURE))/30;
		currentPerformanceFactor = sqrt(currentCPUPowerFactor);
		//currentPerformanceFactor = currentCPUPowerFactor;
		if (currentCPUPowerFactor > 1.0)
			currentCPUPowerFactor = 1.0;
		if (currentPerformanceFactor > 1.0)
			currentPerformanceFactor = 1.0;
		currentPerformanceStateOutof100 = (int)(currentPerformanceFactor*100);
	}

	if (currentPerformanceStateOutof100 > 100)
		currentPerformanceStateOutof100 = 100;

	if (currentPerformanceStateOutof100 != oldPerformanceStateOutof100)
		howManyTimesDVFSChanged++;
}

unsigned int Server::HowManySecondsOverEmergencyTemp()
{
	return howManySecondsOverEmergencyTemp;
}

unsigned int Server::HowManyTimesDVFSChanged()
{
	return howManyTimesDVFSChanged;
}

void Server::CalculatePowerDraw(FLOATINGPOINT utilization, FLOATINGPOINT temperature)
{
	if (isOFF) {
		currentPowerDraw = currentFanPowerDraw = 0.0;
		return;
	}

	FLOATINGPOINT idlePower = cpuTDP; // assuming half power when idle
	FLOATINGPOINT currentCPUpower = cpuTDP*utilization;
	FLOATINGPOINT temperatureGap = coolerMaxDieTemperature - temperature;
	FLOATINGPOINT additionalFanPower;

	if (CONSTANT_FAN_POWER) {
		currentPowerDraw = currentCPUpower + idlePower;
		currentFanPowerDraw = coolerMaxPower*2; // two fans
		return;
	}

	// cpu cooling
	{
		FLOATINGPOINT targetRPM = (currentCPUpower/temperatureGap)/(cpuTDP/coolerMinTemperatureGap)*coolerMaxRPM;
		if (targetRPM < coolerMinRPM)
			targetRPM = coolerMinRPM;
		if (!FAN_RPM_NO_LIMIT) {
			if (targetRPM > coolerMaxRPM)
				targetRPM = coolerMaxRPM;
		}
		FLOATINGPOINT fanFactor = (targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM); // ? porque 3x
		additionalFanPower = coolerMaxPower * fanFactor;
	}

	// case cooling (two fans)
	{
		FLOATINGPOINT targetRPM = (currentCPUpower/cpuTDP*(coolerMaxRPM-coolerMinRPM) + coolerMinRPM /* -> for removing idle power */ ) * temperature/EMERGENCY_TEMPERATURE;
		if (targetRPM < coolerMinRPM)
			targetRPM = coolerMinRPM;
		if (!FAN_RPM_NO_LIMIT) {
			if (targetRPM > coolerMaxRPM)
				targetRPM = coolerMaxRPM;
		}
		FLOATINGPOINT fanFactor = (targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM);
		additionalFanPower += 2 *(coolerMaxPower * fanFactor);
	}
	currentPowerDraw = currentCPUpower + idlePower + additionalFanPower;
	currentFanPowerDraw = additionalFanPower;
}

bool Server::IsFinished()
{
	if (isOFF) {
	    return true;
	}

	if (!vRunningVMs.empty()) {
	   return false;
	}

	return true;
}

void Server::AddHeatToTimingBuffer(FLOATINGPOINT temperature, int timing)
{
	unsigned int slotIndex = (clock + timing) % SIZE_OF_HEAT_TIMING_BUFFER;
	additionalHeatTimingBuffer[slotIndex] += temperature;
}

void Server::SetSupplyTempToTimingBuffer(FLOATINGPOINT temperature, int timing)  
{
	unsigned int slotIndex = (clock + timing) % SIZE_OF_HEAT_TIMING_BUFFER;
	supplyTempTimingBuffer[slotIndex] = temperature;
}

FLOATINGPOINT Server::ReadSupplyTempToTimingBuffer()  
{
	unsigned int slotIndex = clock % SIZE_OF_HEAT_TIMING_BUFFER;
	return supplyTempTimingBuffer[slotIndex];
}

FLOATINGPOINT Server::ReadHeatFromTimingBuffer()
{
	unsigned int slotIndex = clock % SIZE_OF_HEAT_TIMING_BUFFER;
	return additionalHeatTimingBuffer[slotIndex];
}

FLOATINGPOINT Server::CurrentInletTemperature()
{
	unsigned int slotIndex = clock % SIZE_OF_HEAT_TIMING_BUFFER;
	return (supplyTempTimingBuffer[slotIndex]+additionalHeatTimingBuffer[slotIndex]);
}

FLOATINGPOINT Server::CurrentAddedTemperature()
{
	unsigned int slotIndex = clock % SIZE_OF_HEAT_TIMING_BUFFER;
	return additionalHeatTimingBuffer[slotIndex];
}

void Server::SetCPUGeneration(unsigned int gen)
{
	cpuGeneration = gen;
}

FLOATINGPOINT Server::returnPositionVectorWorkLoadServer(int k)
{
	return workLoadServer[k];
}

FLOATINGPOINT Server::returnPositionVectorUtilizationCPU(int k)
{
	return utilizationCPU[k];
}

long int Server::returnPositionVectorMemoryUseOfServer(int k)
{
	return memoryUseOfServer[k];
}

int Server::returnSizeVectorWorkLoadServer(void)
{
	return workLoadServer.size();
}

int Server::returnSizeVectorMemoryUseOfServe(void)
{
	return memoryUseOfServer.size();
}
	
int Server::returnSizeVectorUtilizationCPU(void)
{
	return utilizationCPU.size();
}

int Server::returnSizeVectorTemperature(void)
{
	return temperatureServer.size();
}

void Server::insertVectorUtilizationCPU(FLOATINGPOINT utilCPU)
{	
	utilizationCPU.push_back(utilCPU);
}

void Server::insertVectorUtilizationMemory(long int utilMemory)
{	
	memoryUseOfServer.push_back(utilMemory);
}

void Server::insertVectorTemperature(FLOATINGPOINT tempServer)
{	
	temperatureServer.push_back(tempServer);
}

vector<FLOATINGPOINT> Server::returnVectorTemperature(void)
{
	return temperatureServer;
}

vector<FLOATINGPOINT> Server::returnVectorCPU(void)
{
	return utilizationCPU;
}

void Server::insertTemperaturePredictionServer(FLOATINGPOINT temperature)
{	
	temperaturePredictionServer.push_back(temperature);
}

void Server::insertTimePredictionServer(unsigned int timeprediction)
{	
	timePredictionServer.push_back(timeprediction);
}

unsigned int Server::returnClock (void)
{
	return clock;
}

void Server::addErrorPrediction(void)
{
   errorPrediction += 1;
}

void Server::addHitPrediction(void)
{
   hitPrediction += 1;
}

unsigned int Server::returnErrorPrediction(void)
{
	return errorPrediction;
}

unsigned int Server::returnHitPrediction(void)
{
	return hitPrediction;
}

unsigned int Server::returnFirstTimePredictionServer(void)
{
	if (!timePredictionServer.empty()) {
		return timePredictionServer.front();
	}
	else {
		return -1;
	}
}

void Server::calculateRMSE()
{
  double diff = 0.0;
  double temperatureServer = 0.00;

  temperatureServer = CurrentInletTemperature();
  
  diff = temperaturePredictionServer[0] - temperatureServer;
  RMSE += pow(diff,2);
  numberElements += 1;

  errorMean += fabs(1-(temperaturePredictionServer[0]/temperatureServer));

  varianceErrorMean.push_back(fabs(1-(temperaturePredictionServer[0]/temperatureServer)));
   
  temperaturePredictionServer.erase(temperaturePredictionServer.begin());
  timePredictionServer.erase(timePredictionServer.begin());
}

FLOATINGPOINT Server::returnRMSE(void)
{
  return sqrt(RMSE / numberElements);
}

FLOATINGPOINT Server::returnErrorMean(void)
{
  return errorMean / numberElements;
}

FLOATINGPOINT Server::returnVarianceErrorMean(void)
{
 FLOATINGPOINT var = 0.00;
 FLOATINGPOINT mean = 0.00;

 mean = returnErrorMean();

  for(int i=0; i < varianceErrorMean.size(); i++) {
      var += pow(varianceErrorMean[i] - mean,2);
  }
  
  return var / (varianceErrorMean.size()-1);
}

FLOATINGPOINT Server::returnSDErrorMean(void){

	return sqrt(returnVarianceErrorMean());
}

void Server::addPOOL(void)
{
  isPOOL = true;
}

void Server::removePOOL(void)
{
  isPOOL = false;
}

void Server::timePowerOnServer(int clockSimul)
{
	powerOnServer = clockSimul + POWER_ON;
}


int Server::returnFinishPowerOnServer(void)
{
  return powerOnServer;
}

void Server::insertRackInServer(int Rck)
{
  rack = Rck;
}

void Server::insertLinkInServer(Link_SV_to_SW* lk)
{
	link.push_back(lk);
}