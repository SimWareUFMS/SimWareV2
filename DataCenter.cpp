#include "DataCenter.h"
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>

DataCenter::DataCenter(JobQueue* q, ServersPOOL* Pool, Topology* topology)
{
	fpSupplyTemperatureLog = 0.0;
	vSupplyTemperatureSparseLog.clear();

	fpTotalPowerFromComputingMachinesLog = 0.0;
	fpTotalPowerFromServerFansLog = 0.0;
	vTotalPowerFromComputingMachinesSparseLog.clear();
	vTotalPowerFromServerFansSparseLog.clear();

	fpTotalComputingPowerLog = 0.0;
	vTotalComputingPowerSparseLog.clear();

	fpTotalPowerFromCRACLog = 0.0;
	vTotalPowerFromCRACSparseLog.clear();

	fpUtilizationLog = 0.0;
	vUtilizationSparseLog.clear();

	currentSupplyTemp = LOWEST_SUPPLY_TEMPERATURE;
	currentSupplyTempBase = LOWEST_SUPPLY_TEMPERATURE;

	vTotalMigrationPolicyOne = 0;
	vTotalMigrationPolicyTwo = 0;
	vTotalMigrationPolicyThree = 0;
	vTotalMigrationPolicyOneLog = 0;
	vTotalMigrationPolicyTwoLog = 0;
	vTotalMigrationPolicyThreeLog = 0;

	vTotalMigrationsPolicyOnSparseLog.clear();
	vTotalMigrationsPolicyTwoSparseLog.clear();
	vTotalMigrationsPolicyThreeSparseLog.clear();


	pJobQueue = q;
	clock = 0;
	peakPower = 0.0;

	pPool = Pool;
	pTopology = topology;
	 
	avgArrivalTime = 0;
	clockPrevius = 0;
	avgRunTime = 0;
	
	totalSmallVMSArrived = 0;
	totalMediumVMSArrived = 0;
	totalBigVMSArrived = 0;

	totalVMs = 0;
	
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i)
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j)
			pServers[i][j] = new Server(0);


	for (int i=0; i<NUMBER_OF_CHASSIS; ++i)
		perServerPowerLog[i] = perServerTemperatureLog[i] = perServerComputingPowerLog[i] = 0.0;

	if (SCHEDULING_ALGORITHM == "best_performance")
		pSchedulingAlgorithm = new BestPerformanceSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD);
	else if (SCHEDULING_ALGORITHM == "uniform_task")
		pSchedulingAlgorithm = new UniformTaskSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD);
	else if (SCHEDULING_ALGORITHM == "low_temp_first")
		pSchedulingAlgorithm = new LowTemperatureFirstSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD);
	else if (SCHEDULING_ALGORITHM == "random")
		pSchedulingAlgorithm = new RandomSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD);
	else if (SCHEDULING_ALGORITHM == "min_hr")
		pSchedulingAlgorithm = new MinHRSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD);
	else if (SCHEDULING_ALGORITHM == "center_rack_first")
		pSchedulingAlgorithm = new CenterRackFirstSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD);
	else if (SCHEDULING_ALGORITHM == "2DA")
		pSchedulingAlgorithm = new TwoDimensionVASchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, pPool); 
	else if (SCHEDULING_ALGORITHM == "2DB")
		pSchedulingAlgorithm = new TwoDimensionVBSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD); 
	else if (SCHEDULING_ALGORITHM == "2DAPrediction")
		pSchedulingAlgorithm = new TwoDimensionVAPredictionSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, pPool, &currentSupplyTempBase);
	else if (SCHEDULING_ALGORITHM == "2DCPrediction")
		pSchedulingAlgorithm = new TwoDimensionVCPredictionSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, &currentSupplyTempBase);
	else {
		cout << "Error: unknown scheduling algorithm. Use default value (best_performance)" << endl;
		pSchedulingAlgorithm = new BestPerformanceSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD);
	}

    // Policy Migration
	policyOne = new PoliceLowUtilization(&pServers);
	policyTwo = new PoliceHighTemperature(&pServers, HeatRecirculationMatrixD, pPool);
	policyThree = new PoliceIdle(&pServers, pPool);

	perDataCenterUtilization = 0.0;

	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i)
		ModedAirTravelTimeFromCRAC[i] = AirTravelTimeFromCRAC[i] * COOL_AIR_TRAVEL_TIME_MULTIPLIER;

	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i) {
		if (ModedAirTravelTimeFromCRAC[i] > SIZE_OF_HEAT_TIMING_BUFFER)
			cout << "Error: HeatRecirculationTimingFromCRAC["<< i<<"] > " << SIZE_OF_HEAT_TIMING_BUFFER << endl;
	}

	for (int i=0; i<101; ++i) {
		HowManySecondsInThisInletAirTemperature[i] = 0;
		HowManySecondsInThisUtilization[i] = 0;
	}
	CRACDischargeAirTempChangeRate = 0.001 * CRAC_DISCHARGE_CHANGE_RATE_0_00x;
	//CRACDischargeAirTempChangeRate = 0.1;

    // Create Topology
	pTopology->CreateTopology(&pServers);

}


DataCenter::~DataCenter(void)
{
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i)
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j)
			delete pServers[i][j];

	delete pSchedulingAlgorithm;

	delete policyOne;
	delete policyTwo;
	delete policyThree;

    vTotalMigrationsPolicyOnSparseLog.clear();
	vTotalMigrationsPolicyTwoSparseLog.clear();
	vTotalMigrationsPolicyThreeSparseLog.clear();
	vSupplyTemperatureSparseLog.clear();
	vTotalPowerFromComputingMachinesSparseLog.clear();
	vTotalPowerFromServerFansSparseLog.clear();
	vTotalComputingPowerSparseLog.clear();
	vTotalPowerFromCRACSparseLog.clear();
	vUtilizationSparseLog.clear();
}

void DataCenter::EveryASecond(void)
{
  double cpuLoad = 0.00;
  
  // updated simulation clock
  pPool->UpdateClockSimulation(clock);

  // calculate variance
  if (!pJobQueue->IsFinished()) {
	 avgArrivalTime += (clock - clockPrevius);
	 varianceArrivalTime.push_back(clock - clockPrevius);
	 clockPrevius = clock;
  }

  while (!pJobQueue->IsFinished()) { // JobQueue is not empty
    	SingleJob* sj = pJobQueue->TakeFirst();
		for (int i=0; i < sj->numCPUs; ++i) {
		    qWaitingVMs.push(new VirtualMachine(sj->runTimeSec, sj->avgCPUTimeSec, sj->jobNumber, sj->usedMemKB));
			totalVMs += 1; // vms total arrived DC
			avgRunTime += sj->runTimeSec;
			varianceRunTime.push_back(sj->runTimeSec);

			// quantifies the VMs per CPU utilization
			if (sj->avgCPUTimeSec < 0) {
			   cpuLoad = 1; // 100% utilization;
			}
			else {
			   cpuLoad =  (double) sj->avgCPUTimeSec / (double) sj->runTimeSec;
			}
			if (cpuLoad < 0.35) {
			   totalSmallVMSArrived += 1;
			}
			else {
			   if ((cpuLoad >= 0.35) && (cpuLoad < 0.65)) {
				  totalMediumVMSArrived += 1;
			   }
			   else {
				  totalBigVMSArrived += 1;
			   }
			}
		}
  }
                                            
  // calculates the server pool window
/*  if ((clock%SUMVMARRIVALTIME==0) && (clock <= 329959)) { 
     pPool->insertWindowsSmallVMSArrived(totalSmallVMSArrived);
	 pPool->insertWindowsMediumVMSArrived(totalMediumVMSArrived);
	 pPool->insertWindowsBigVMSArrived(totalBigVMSArrived);
	 totalSmallVMSArrived = 0; totalMediumVMSArrived = 0; totalBigVMSArrived = 0;
  }
*/
  if (REASSIGN_VMS && (clock%16384==0)) { // re arranging
	 for (int i=0; i<NUMBER_OF_CHASSIS; ++i) { 
      	 for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
		  	 if (pServers[i][j]->IsOFF()) continue;
				VirtualMachine* pvm;
				while (pvm = (pServers[i][j]->TakeAVM())) {
					  if (pvm == NULL)
						 break;
					  qWaitingVMs.push(pvm);
				}
		 }
	 }
  }


  // Assign jobs to the servers
  pSchedulingAlgorithm->AssignVMs();

  // Optimization of servers

  if ((clock%120==0) && (clock!=0))  {
	 vTotalMigrationPolicyOneLog += policyOne->OptimizationServers(clock);
  }

  if ((clock%60==0) && (clock!=0))  {
	 vTotalMigrationPolicyTwoLog += policyTwo->OptimizationServers(clock);
  }

  if ((clock%120==0) && (clock!=0)) {
	 vTotalMigrationPolicyThreeLog += policyThree->OptimizationServers(clock);
  }

  // call EveryASecond to every server instance
  for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	  for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
		  if (PREDICITIONSHEDULER) {
			 if (clock == pServers[i][j]->returnFirstTimePredictionServer()){ // calculte RMSD
				pServers[i][j]->calculateRMSE();
			 }
		  } 
		  pServers[i][j]->EveryASecond(clock);
	  }
  }

  // verifies that the vms finished migrate
  // verifies that the servers power off
  // verifies that the servers power on
  
  pPool->EveryASecond(&pServers);

  // increases the clock simulator
  ++clock;


  // Calculates the energy consumption of switches and optimizes network topology
  pTopology->EveryASecond(clock);

  RecalculateHeatDistribution();

	// decide supply temperature for next 1 second
	if (CRAC_SUPPLY_TEMP_IS_CONSTANT_AT > 0) {
		currentSupplyTemp = CRAC_SUPPLY_TEMP_IS_CONSTANT_AT;
	} else if (INSTANT_CHANGE_CRAC_SUPPLY_AIR) {
		FLOATINGPOINT highestAddedTemp = 0.0;
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				FLOATINGPOINT local = pServers[i][j]->CurrentAddedTemperature();
				if (local > highestAddedTemp)
					highestAddedTemp = local;
			}
		}
		currentSupplyTemp = EMERGENCY_TEMPERATURE-highestAddedTemp;
	} else {
		FLOATINGPOINT highestInletTemp = 0.0;
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ( pServers[i][j]->IsOFF() || pServers[i][j]->IsMIGRATING()) {
					continue;
				}
				FLOATINGPOINT localInlet = pServers[i][j]->CurrentInletTemperature();
				if (localInlet > highestInletTemp)
					highestInletTemp = localInlet;
			}
		}

		// original
		/*if ((EMERGENCY_TEMPERATURE + TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS + SUPPLY_TEMPERATURE_OFFSET_ALPHA) > highestInletTemp)
			currentSupplyTempBase += CRACDischargeAirTempChangeRate;
		else
			currentSupplyTempBase -= CRACDischargeAirTempChangeRate;*/

		if (highestInletTemp < 28) {
           currentSupplyTempBase += (CRACDischargeAirTempChangeRate * 5);
		}
		else {
		   if ((highestInletTemp >= 28) && (highestInletTemp < 29.5))  {
              currentSupplyTempBase += (CRACDischargeAirTempChangeRate);
		   }
		   else {
			  if ((highestInletTemp >= 29.5) && (highestInletTemp < EMERGENCY_TEMPERATURE)) {
			     currentSupplyTempBase += 0;
			  }
			  else {
				 currentSupplyTempBase -= CRACDischargeAirTempChangeRate * 2;
			  }
		   }
		}
		if (currentSupplyTempBase < LOWEST_SUPPLY_TEMPERATURE)
			currentSupplyTempBase = LOWEST_SUPPLY_TEMPERATURE;
		currentSupplyTemp = currentSupplyTempBase;
	}

	// Write supply temperature to every server
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			// use matrix only 
			if (INSTANT_COOL_AIR_TRAVEL_TIME)
				pServers[i][j]->SetSupplyTempToTimingBuffer(currentSupplyTemp, 0);
			else {
				pServers[i][j]->SetSupplyTempToTimingBuffer(currentSupplyTemp, ModedAirTravelTimeFromCRAC[i]);
			}
		}
	}

	// record current temperature of servers
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			if (pServers[i][j]->IsOFF()) {
			   continue;
			}
			unsigned int local_temperature = (unsigned int)(pServers[i][j]->CurrentInletTemperature());
			if (local_temperature > 100)
				cout << "Error_1: local_temperature too big: Servidor " << i << " " << j << " Temperature " <<  local_temperature << endl;
			++HowManySecondsInThisInletAirTemperature[local_temperature];

			unsigned int local_utilization = (unsigned int)(pServers[i][j]->CurrentUtilization()*100);
			if (local_utilization > 100)
				cout << "Error_2: local_utilization too big: " << local_utilization << endl;
			++HowManySecondsInThisUtilization[local_utilization];
		}
	}

	// delete finished VMs
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			if ((pServers[i][j]->IsOFF()) || (pServers[i][j]->IsPOOL()) || (pServers[i][j]->IsINITIALIZING())) { 
				continue;
			}
			vector<VirtualMachine*>* pvFinishedVMs = pServers[i][j]->GetFinishedVMVector();
			for (vector<VirtualMachine*>::iterator it = pvFinishedVMs->begin(); it != pvFinishedVMs->end(); ++it) {
				pJobQueue->OneVMFinishedOnASingleJob((*it)->GetJobNumber(), (int)clock, (float)(i+j/10.0));
				delete (*it);
			}
			pvFinishedVMs->clear();
		}
	}

	// logs
	if (clock%PERIODIC_LOG_INTERVAL==0) {
		vSupplyTemperatureSparseLog.push_back(fpSupplyTemperatureLog/PERIODIC_LOG_INTERVAL);
		fpSupplyTemperatureLog = 0.0;

		vTotalPowerFromComputingMachinesSparseLog.push_back(fpTotalPowerFromComputingMachinesLog/PERIODIC_LOG_INTERVAL);
		fpTotalPowerFromComputingMachinesLog = 0.0;
		vTotalPowerFromServerFansSparseLog.push_back(fpTotalPowerFromServerFansLog/PERIODIC_LOG_INTERVAL);
		fpTotalPowerFromServerFansLog = 0.0;

		vTotalComputingPowerSparseLog.push_back(fpTotalComputingPowerLog/PERIODIC_LOG_INTERVAL);
		fpTotalComputingPowerLog = 0.0;

		vTotalPowerFromCRACSparseLog.push_back(fpTotalPowerFromCRACLog/PERIODIC_LOG_INTERVAL);
		fpTotalPowerFromCRACLog = 0.0;

		vUtilizationSparseLog.push_back(fpUtilizationLog/PERIODIC_LOG_INTERVAL/NUMBER_OF_CHASSIS/NUMBER_OF_SERVERS_IN_ONE_CHASSIS);
		fpUtilizationLog = 0.0;

		vTotalMigrationsPolicyOnSparseLog.push_back(vTotalMigrationPolicyOneLog);
		vTotalMigrationsPolicyTwoSparseLog.push_back(vTotalMigrationPolicyTwoLog);
		vTotalMigrationsPolicyThreeSparseLog.push_back(vTotalMigrationPolicyThreeLog);
		
		vTotalMigrationPolicyOne += vTotalMigrationPolicyOneLog;
		vTotalMigrationPolicyTwo += vTotalMigrationPolicyTwoLog;
		vTotalMigrationPolicyThree += vTotalMigrationPolicyThreeLog;
		
		vTotalMigrationPolicyOneLog = 0;
		vTotalMigrationPolicyTwoLog = 0;
		vTotalMigrationPolicyThreeLog = 0;
	}

	fpSupplyTemperatureLog += currentSupplyTemp;
	FLOATINGPOINT totalPowerDrawIT = TotalPowerDrawFromComputingMachines();
	FLOATINGPOINT totalPowerDrawFans = TotalPowerDrawFromServerFans();
	fpTotalPowerFromComputingMachinesLog += totalPowerDrawIT;
	fpTotalPowerFromServerFansLog += totalPowerDrawFans;
	FLOATINGPOINT totalPowerDrawCRAC = CalculateCurrentCRACPower(totalPowerDrawIT);
	fpTotalPowerFromCRACLog += totalPowerDrawCRAC;
	if (peakPower < (totalPowerDrawCRAC+totalPowerDrawIT))
		peakPower = totalPowerDrawCRAC+totalPowerDrawIT;

	fpTotalComputingPowerLog += TotalComputingPower();
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			perServerPowerLog[i] += pServers[i][j]->GetPowerDraw();
			perServerComputingPowerLog[i] += pServers[i][j]->MaxUtilization();
		}
		perServerTemperatureLog[i] += pServers[i][0]->CurrentInletTemperature();
	}
	FLOATINGPOINT sum = 0.0;
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
		    sum += pServers[i][j]->CurrentUtilization();
	    }
	}
	perDataCenterUtilization += sum;
	fpUtilizationLog += sum;

}

FLOATINGPOINT DataCenter::CalculateCurrentCRACPower(FLOATINGPOINT totalPowerDrawIT)
{
	FLOATINGPOINT cop = 0.0068 * currentSupplyTemp * currentSupplyTemp + 0.0008 * currentSupplyTemp + 0.458;
	return (totalPowerDrawIT/cop);
}

FLOATINGPOINT DataCenter::TotalComputingPower()
{
	FLOATINGPOINT retval = 0.0;
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i)
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j)
			retval += pServers[i][j]->MaxUtilization();
	return retval;
}

FLOATINGPOINT DataCenter::TotalPowerDrawFromComputingMachines()
{
	FLOATINGPOINT retval = 0.0;
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i)
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j)
			retval += pServers[i][j]->GetPowerDraw();
	return retval;
}

FLOATINGPOINT DataCenter::TotalPowerDrawFromServerFans()
{
	FLOATINGPOINT retval = 0.0;
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i)
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j)
			retval += pServers[i][j]->GetFanPower();
	return retval;
}

void DataCenter::RecalculateHeatDistribution()
{
	// calcuate power draw of each CHASSIS
	FLOATINGPOINT powerDraw[SIZE_OF_HR_MATRIX];
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		powerDraw[i] = 0.0;
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			powerDraw[i] += pServers[i][j]->GetPowerDraw();
		}
	}

	// calculate added heat
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		for (int j=0; j<NUMBER_OF_CHASSIS; ++j) {
			FLOATINGPOINT heatFromJtoI = powerDraw[j]*HeatRecirculationMatrixD[i][j];
			for (int k=0; k<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++k) {
				pServers[i][k]->AddHeatToTimingBuffer(heatFromJtoI, 0);
			} 
		}
	}
}

bool DataCenter::IsFinished()
{
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			if (!(pServers[i][j]->IsFinished()))
				return false;
		}
	}
	return true;
}

void DataCenter::PrintResults(ServersPOOL* pPool)
{
	double var = 0.0000;
	double averageArrivalTime = 0.00;
	double averageRunTime = 0.00;
	unsigned int sumErrorPrediction = 0;
	unsigned int sumHitPrediction = 0;
	double eMean = 0.00;

	// print timing information
	cout << "Total Execution Time (seconds): " << clock << endl;


	//Trace Statistics

	cout << endl;

	cout << "Trace Statistics" << endl << endl;
	
	averageArrivalTime = (double) avgArrivalTime / (double) varianceArrivalTime.size();
	
	cout << "Average time between arrivals of VMs: " << averageArrivalTime << endl;
	
	for(int i=0; i < varianceArrivalTime.size(); i++) {
		var += (pow((double (varianceArrivalTime[i]) - averageArrivalTime),2));
	}
	cout << "Variance between arrivals of VMs: " << (var / (varianceArrivalTime.size()-1)) << endl;
	cout << "Standard deviation arrivals of VMs: " << sqrt(var / (varianceArrivalTime.size()-1)) << endl << endl;

	averageRunTime = avgRunTime / (double) varianceRunTime.size();
	cout << "Average execution time of the VMs: " << averageRunTime << endl;

	var = 0.0000;
	for(int i=0; i < varianceRunTime.size(); i++) {
		var += (pow((double (varianceRunTime[i]) - averageRunTime),2));
	}
	cout << "Variance execution time of the VMs: " << (var / (varianceRunTime.size()-1)) << endl;
	cout << "Standard deviation arrivals of VMs: " << sqrt((var / (varianceRunTime.size()-1))) << endl << endl;

	if (PREDICITIONSHEDULER) {
	    cout << "Prediction Statistics" << endl << endl;

	    for (int i=0; i < NUMBER_OF_CHASSIS; ++i) {
		    for (int j=0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			    sumErrorPrediction += pServers[i][j]->returnErrorPrediction();
	            sumHitPrediction += pServers[i][j]->returnHitPrediction();
		    }
	    }

	    cout << "Total of hits in the Prediciton Range: " << sumHitPrediction << endl;
	    cout << "Total of error in the Prediciton Range: " << sumErrorPrediction << endl << endl;
	
	    cout << "Root Mean Squared Error (RMSE) " << endl << endl;

	    for (int i=0; i < NUMBER_OF_CHASSIS; ++i) {
		    cout << "Chassi " << i;
		    for (int j=0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ( pServers[i][j]->returnRMSE() >= 0) {
					cout << " " << pServers[i][j]->returnRMSE();
				}
				else {
					cout << " " << "0.000000";
				}
		    }
		    cout << endl;
	    }

	    cout << endl << endl;

	    cout << "Average Error of predicted values and real " << endl << endl;

	    for (int i=0; i < NUMBER_OF_CHASSIS; ++i) {
		    cout << "Chassi " << i;
		    for (int j=0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if (pServers[i][j]->returnErrorMean() >= 0) {
				   cout << " " << pServers[i][j]->returnErrorMean()*100 << "%";
				}
				else {
				   cout << " " << "0.00000" << "%";
				}
		    }
		    cout << endl;
	    }

        cout << endl << endl;

	    cout << "Variance of predicted values and real " << endl << endl;

	    for (int i=0; i < NUMBER_OF_CHASSIS; ++i) {
		    cout << "Chassi " << i;
	   	    for (int j=0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ( pServers[i][j]->returnVarianceErrorMean() > 0) {
				   cout << " " << pServers[i][j]->returnVarianceErrorMean();
				}
				else {
				   cout << " " << "0.000000000";	
				}
		    }
		    cout << endl;
	    }

	    cout << endl << endl;

	    cout << "Standard Deviation of predicted values and real " << endl << endl;

	    for (int i=0; i < NUMBER_OF_CHASSIS; ++i) {
		    cout << "Chassi " << i;
		    for (int j=0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if (pServers[i][j]->returnSDErrorMean() > 0) {
				   cout << " " << pServers[i][j]->returnSDErrorMean();
				}
				else {
				   cout << " " << "0.0000000";
				}
		    }
		    cout << endl;
	    }

	    cout << endl << endl;

	    cout << "Coefficient of variation of predicted values and real " << endl << endl;

	    for (int i=0; i < NUMBER_OF_CHASSIS; ++i) {
		    cout << "Chassi " << i;
		    for (int j=0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ((pServers[i][j]->returnSDErrorMean() / pServers[i][j]->returnErrorMean()) >= 0) {
			       cout << " " << pServers[i][j]->returnSDErrorMean() / pServers[i][j]->returnErrorMean();
				}
				else {
					cout << " " << "0.00000";
				}
		    }
		    cout << endl;
	    }
	    cout << endl << endl;
	}
 
   
	cout << "Statistics of Migration Policies " << endl << endl;

	cout << "Total Migrations Low Utilization: " << vTotalMigrationPolicyOne << endl;

	cout << "Total Migrations Low Utilization (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (int i=0; i<vTotalMigrationsPolicyOnSparseLog.size(); ++i)
		cout << vTotalMigrationsPolicyOnSparseLog[i] << "\t";
	cout << endl << endl;

	cout << "Total Migrations Hight Temperature: " << vTotalMigrationPolicyTwo << endl;

	cout << "Total Migrations Hight Temperature (every " << PERIODIC_LOG_INTERVAL << " secs)" << endl;
	for (int i=0; i<vTotalMigrationsPolicyTwoSparseLog.size(); ++i)
		cout << vTotalMigrationsPolicyTwoSparseLog[i] << "\t";
	cout << endl << endl;

	cout << "Total idle servers turn off: " << vTotalMigrationPolicyThree << endl;

	cout << "Total idle servers turn off (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (int i=0; i<vTotalMigrationsPolicyThreeSparseLog.size(); ++i)
     	cout << vTotalMigrationsPolicyThreeSparseLog[i] << "\t";
	cout << endl << endl;
	
	cout << "Statistics of the Virtual Machines " << endl << endl;	

	cout << "Total VMs arrived in the Data Center: " << totalVMs << endl;
	cout << "Total VMs scheduling in the Data Center: " << pSchedulingAlgorithm->returnTotalScheduling() << endl << endl;
	

	cout << "Statistics from the average energy consumption of data centers" << endl << endl; 
	// print average supply temperature
	FLOATINGPOINT sum = 0.0;
	FLOATINGPOINT sum2 = 0.0;
	FLOATINGPOINT sum3 = 0.0;
	FLOATINGPOINT sum4 = 0.0;
	FLOATINGPOINT sum5 = 0.0;
	FLOATINGPOINT sum6 = 0.0;
	FLOATINGPOINT sum7 = 0.0;
	FLOATINGPOINT sum8 = 0.0;
	FLOATINGPOINT sum9 = 0.0;

	for (size_t i=0; i<vSupplyTemperatureSparseLog.size(); ++i)
		sum += vSupplyTemperatureSparseLog[i];
	cout << "Average Supply Temperature: " << (sum/vSupplyTemperatureSparseLog.size()) << endl << endl;;
	
	// print peakPower
	cout << "peakPower: " << peakPower << endl << endl;;

	// Average Power Consumption
	sum = 0.0;
	for (size_t i=0; i<vTotalPowerFromComputingMachinesSparseLog.size(); ++i)
		sum += vTotalPowerFromComputingMachinesSparseLog[i];
	cout << "Average Power Consumption (Servers = Fans + Computing components): " << (sum/vTotalPowerFromComputingMachinesSparseLog.size()) << endl;

	sum3 = 0.0;
	for (size_t i=0; i<vTotalPowerFromServerFansSparseLog.size(); ++i)
		sum3 += vTotalPowerFromServerFansSparseLog[i];
	cout << "\tAverage Power Consumption (Fans): " << (sum3/vTotalPowerFromServerFansSparseLog.size()) << endl;
	cout << "\tAverage Power Consumption (Computing components): " << (sum/vTotalPowerFromComputingMachinesSparseLog.size()) - (sum3/vTotalPowerFromServerFansSparseLog.size()) << endl << endl;

	sum2 = 0.0;
	for (size_t i=0; i<vTotalPowerFromCRACSparseLog.size(); ++i)
		sum2 += vTotalPowerFromCRACSparseLog[i];
	cout << "Average Power Consumption (CRAC): " << (sum2/vTotalPowerFromCRACSparseLog.size()) << endl << endl;

	
	sum4 = pTopology->ReturnTotalEnergyAccessSwitchSparseLog()/pTopology->ReturnSizeVectorTotalEnergyAccessSwitch();
	cout << "Average Power Consumption (Switch Access): " << sum4 << endl;;
	sum5 = pTopology->ReturnTotalEnergyAggregationSwitchSparseLog()/pTopology->ReturnSizeVectorTotalEnergyAggregationSwitch();
	cout << "Average Power Consumption (Switch Aggregation): " << sum5 << endl; 
    sum6 = pTopology->ReturnTotalEnergyCoreSwitchSparseLog()/pTopology->ReturnSizeVectorTotalEnergyCoreSwitch();
	cout << "Average Power Consumption (Switch Core): " << sum6 << endl;
	sum7 = (pTopology->ReturnTotalEnergyAccessSwitchSparseLog() + pTopology->ReturnTotalEnergyAggregationSwitchSparseLog() + pTopology->ReturnTotalEnergyCoreSwitchSparseLog()) / pTopology->ReturnSizeVectorTotalEnergyAccessSwitch();
    cout << "Average Power Consumption (Total = Switch Access + Switch Aggregation + Switch Core): " << sum7 << endl << endl;

	cout << "Average Power Consumption (Total = CRAC + Servers + Switches): " << ((sum + sum2)/vTotalPowerFromCRACSparseLog.size())+sum7 << endl << endl;


	cout << "Statistical Data Center Energy Consumption" << endl << endl; 


	cout << "Energy (Servers = Fans + Computing components): " << (sum*PERIODIC_LOG_INTERVAL) << endl;
	cout << "\tEnergy (Fans): " << (sum3*PERIODIC_LOG_INTERVAL) << endl;
	cout << "\tEnergy (Computing components): " << ((sum-sum3)*PERIODIC_LOG_INTERVAL) << endl << endl;

	cout << "Energy (CRAC): " << (sum2*PERIODIC_LOG_INTERVAL) << endl << endl;

	sum8 = (pTopology->ReturnTotalEnergyAccessSwitch() + pTopology->ReturnTotalEnergyAggregationSwitch() + pTopology->ReturnTotalEnergyCoreSwitch()); 
	cout << "Energy (Total = Switch Access + Switch Aggregation + Switch Core): " << sum8 << endl;
	
	cout << "\tEnergy (Switch Access): " << pTopology->ReturnTotalEnergyAccessSwitch() << endl;
	cout << "\tEnergy (Switch Aggregation): " << pTopology->ReturnTotalEnergyAggregationSwitch() << endl;
	cout << "\tEnergy (Switch Core): " << pTopology->ReturnTotalEnergyCoreSwitch() << endl << endl;

	cout << "Energy (Total = CRAC + Servers + Switches): " << (((sum+sum2)*PERIODIC_LOG_INTERVAL) + sum8) << endl << endl;

	sum9 = pTopology->ReturnTotalEnergyAccessSwitchSparseLog() + pTopology->ReturnTotalEnergyAggregationSwitchSparseLog() + pTopology->ReturnTotalEnergyCoreSwitchSparseLog();
	cout << "PUE: " << ((sum+sum2+sum9)/(sum+sum9)) << endl;
	cout << "tPUE: " << ((sum+sum2)/(sum-sum3)) << endl;
	cout << endl;

	// average data center utilization
	cout << "Average Utilization (Data Center Level): " << (perDataCenterUtilization/NUMBER_OF_CHASSIS/NUMBER_OF_SERVERS_IN_ONE_CHASSIS/clock) << endl;

	// utilization log
	cout << "Utilization of the data center (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (size_t i=0; i<vUtilizationSparseLog.size(); ++i)
		cout << vUtilizationSparseLog[i] << "\t";
	cout << endl << endl;
	
	// computing power log
	cout << "Total computing power (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (size_t i=0; i<vTotalComputingPowerSparseLog.size(); ++i)
		cout << vTotalComputingPowerSparseLog[i] << "\t";
	cout << endl << endl;

	// per Chassis log
	cout << "Per Chassis Avg Power: " << endl;
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		cout << (perServerPowerLog[i]/clock/NUMBER_OF_SERVERS_IN_ONE_CHASSIS) << "\t";
		if (i%10==9)
			cout << endl;
	}
	cout << endl;
	cout << "Per Chassis Avg Computing Power (max utilization):" << endl;
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		cout << (perServerComputingPowerLog[i]/clock/NUMBER_OF_SERVERS_IN_ONE_CHASSIS) << "\t";
		if (i%10==9)
			cout << endl;
	}
	cout << endl;
	cout << "Per Chassis Avg Inlet Temperature: " << endl;
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		cout << (perServerTemperatureLog[i]/clock) << "\t";
		if (i%10==9)
			cout << endl;
	}
	cout << endl;
	cout << "Per Chassis Seconds (avg) Over Emergency Temp: " << endl;
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		unsigned int timeOverEmergencyTemp = 0;
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j)
			timeOverEmergencyTemp += pServers[i][j]->HowManySecondsOverEmergencyTemp();
		cout << ((FLOATINGPOINT)timeOverEmergencyTemp/(FLOATINGPOINT)NUMBER_OF_SERVERS_IN_ONE_CHASSIS) << "\t";
		if (i%10==9)
			cout << endl;
	}
	cout << endl;

	cout << "Per Server Seconds (avg) Over Emergency Temp: " << endl;
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		cout << "Chassi " << i;
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			cout << " " << pServers[i][j]->HowManySecondsOverEmergencyTemp() << "\t";
		}
		cout << endl;
	}
	cout << endl;

	cout << "Per Server - How many times DVFS changed:" << endl;
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j)
			cout << pServers[i][j]->HowManyTimesDVFSChanged() << "\t";
		if (i%10==9)
			cout << endl;
	}
	cout << endl;

	// Power (every)
	cout << "Total Power Draw (Computing) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (size_t i=0; i<vTotalPowerFromComputingMachinesSparseLog.size(); ++i)
		cout << vTotalPowerFromComputingMachinesSparseLog[i] << "\t";
	cout << endl << endl;

	cout << "Total Power Draw (CRAC) (every " << PERIODIC_LOG_INTERVAL << " secs): " << endl;
	for (size_t i=0; i<vTotalPowerFromCRACSparseLog.size(); ++i)
		cout << (vTotalPowerFromCRACSparseLog[i]) << "\t";
	cout << endl << endl;


	cout << "Total Power Draw (Switch Access) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	pTopology->PrintPowerDrawAccessSwitch();
	cout << endl;

	cout << "Total Power Draw (Switch Aggregation) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	pTopology->PrintPowerDrawAggregationSwitch();
	cout << endl;
	
	cout << "Total Power Draw (Switch Core) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	pTopology->PrintPowerDrawCoreSwitch();
	cout << endl;
	
	cout << "Total Power Draw (Total = Switch Access + Switch Aggregation + Switch Core) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	pTopology->PrintTotalPowerDrawSwitches();
    cout << endl;

	cout << "Total Power Draw (Computing + CRAC + Switches) (every " << PERIODIC_LOG_INTERVAL << " secs)" << endl;
	for (size_t i=0; i<vTotalPowerFromCRACSparseLog.size(); ++i)
		cout << (vTotalPowerFromComputingMachinesSparseLog[i]+vTotalPowerFromCRACSparseLog[i]+pTopology->ReturnPowerDrawSwitches(i)) << "\t";
	cout << endl << endl;

	// energy (every)
	cout << "Total Energy Draw (Computing) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (size_t i=0; i<vTotalPowerFromComputingMachinesSparseLog.size(); ++i)
		cout << (vTotalPowerFromComputingMachinesSparseLog[i]*PERIODIC_LOG_INTERVAL) << "\t";
	cout << endl << endl;

	cout << "Total Energy Draw (CRAC) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (size_t i=0; i<vTotalPowerFromCRACSparseLog.size(); ++i)
		cout << (vTotalPowerFromCRACSparseLog[i]*PERIODIC_LOG_INTERVAL) << "\t";
	cout << endl << endl;

	cout << "Total Energy Draw (Switch Access) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	pTopology->PrintTotalEnergyAccessSwitchSparseLog();
	cout << endl;

	cout << "Total Energy Draw (Switch Aggregation) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	pTopology->PrintTotalEnergyAggregationSwitchSparseLog();
	cout << endl;
	
	cout << "Total Energy Draw (Switch Core) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	pTopology->PrintTotalEnergyCoreSwitchSparseLog();
	cout << endl;
	
	cout << "Total Energy Draw (Total = Switch Access + Switch Aggregation + Switch Core) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	pTopology->PrintTotalEnergySwitchesSparseLog();
	cout << endl;

	cout << "Total Energy Draw (Computing + CRAC + Switches) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (size_t i=0; i<vTotalPowerFromCRACSparseLog.size(); ++i)
		cout << ((vTotalPowerFromComputingMachinesSparseLog[i]+vTotalPowerFromCRACSparseLog[i]+pTopology->ReturnPowerDrawSwitches(i))*PERIODIC_LOG_INTERVAL) << "\t";
	cout << endl << endl;
	
	// print supply temperature log
	cout << "Supply Temperature (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (size_t i=0; i<vSupplyTemperatureSparseLog.size(); ++i)
		cout << vSupplyTemperatureSparseLog[i] << "\t";
	cout << endl << endl;

	// print HowManySecondsInThisInletAirTemperature
	cout << "Inlet Air Temperature Distribution of the server at the highest temperature (seconds):" << endl;
	for (unsigned int i=0; i<100; ++i) {
		if (HowManySecondsInThisInletAirTemperature[i]!=0)
			cout << HowManySecondsInThisInletAirTemperature[i] << "\t" << i << endl;
	}
	cout << endl;

	// print HowManySecondsInThisUtilization
	cout << "Utilization Distribution (seconds) :" << endl;
	for (unsigned int i=0; i<100; ++i) {
		cout << HowManySecondsInThisUtilization[i] << "\t" << i << endl;
	}
	cout << endl;

}

void DataCenter::PrintVector(void)
{
/*	for (int i=0; i<NUMBER_OF_CHASSIS; i++) {
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; j++) {
			cout << "Chassi " << i << " Servidor " << j << "Numero de elementos no vetor" << pServers[i][j]->returnSizeVectorUtilizationCPU() << endl;
			for (int k=0; k < pServers[i][j]->returnSizeVectorUtilizationCPU(); k++) {
				cout << pServers[i][j]->returnPositionVectorUtilizationCPU(k) << " " << endl;
			}
		}
	}

	for (int i=0; i<NUMBER_OF_CHASSIS; i++) {
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; j++) {
			cout << "Chassi " << i << " Servidor " << j << "Numero de elementos no vetor" << pServers[i][j]->returnSizeVectorMemoryUseOfServe() << endl;
			for (int k=0; k < pServers[i][j]->returnSizeVectorMemoryUseOfServe(); k++) {
				cout << pServers[i][j]->returnPositionVectorMemoryUseOfServer(k) << " " << endl;
			}
		}
	}*/


	cout << "Vetor com tempo de 180s" << endl;
	for(int i=0; i < numberVMsArrivingDC.size(); i++) {
		cout << numberVMsArrivingDC[i] << endl;
	}
}