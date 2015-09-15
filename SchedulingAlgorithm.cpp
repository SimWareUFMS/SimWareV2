#include "SchedulingAlgorithm.h"
#include <vector>



SchedulingAlgorithm::~SchedulingAlgorithm(void)
{
}

RandomSchedulingAlgorithm::RandomSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX])
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;

	srand((unsigned int)time(NULL));

	PREDICITIONSHEDULER = false;
}

void RandomSchedulingAlgorithm::AssignVMs()
{
	while (!pqVMsToGo->empty()) {
		int bestI = rand()%NUMBER_OF_CHASSIS;
		int bestJ = rand()%NUMBER_OF_SERVERS_IN_ONE_CHASSIS;
		if (bestI < 0 || bestJ < 0 || bestI > NUMBER_OF_CHASSIS || bestJ > NUMBER_OF_SERVERS_IN_ONE_CHASSIS)
			cout << "Error: No servers to assign a VM" << endl;
		(*ppServers)[bestI][bestJ]->AssignOneVM(pqVMsToGo->front());
		pqVMsToGo->pop();
	}
}

LowTemperatureFirstSchedulingAlgorithm::LowTemperatureFirstSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX])
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;

	srand((unsigned int)time(NULL));

	PREDICITIONSHEDULER = false;
}

void LowTemperatureFirstSchedulingAlgorithm::AssignVMs()
{
	int besttI;
	int besttJ;

	// assign VMs to Servers
	while (!pqVMsToGo->empty()) {
		// assign with qWaitingVMs.top()
		besttI = -1;
		besttJ = -1;
		FLOATINGPOINT bestAvailability = 1000000.0; // any big number
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ((*ppServers)[i][j]->IsOFF()) continue;
				FLOATINGPOINT sumofVM = (*ppServers)[i][j]->VMRequiresThisMuchCPUScale() + pqVMsToGo->front()->GetCPULoadRatio();
				if (sumofVM >= NUMBER_OF_CORES_IN_ONE_SERVER) // it will be over occupied ...
					continue;
				FLOATINGPOINT localInlet = (*ppServers)[i][j]->CurrentInletTemperature();
				if (localInlet < bestAvailability) {
					besttI = i; 
					besttJ = j;
					bestAvailability = localInlet;
				}
			}
		}
		if (besttI < 0 || besttJ < 0) {
			// second iteration. all servers are busy. assign to the lowest inlet temp
			for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
				for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
					if ((*ppServers)[i][j]->IsOFF()) continue;
					FLOATINGPOINT sumofVM = (*ppServers)[i][j]->VMRequiresThisMuchCPUScale();
					if (sumofVM >= NUMBER_OF_CORES_IN_ONE_SERVER) // it will be over occupied ...
						continue;
					FLOATINGPOINT localInlet = (*ppServers)[i][j]->CurrentInletTemperature();
					if (localInlet < bestAvailability) {
						besttI = i; 
						besttJ = j;
						bestAvailability = localInlet;
					}
				}
			}
		}
		if (besttI < 0 || besttJ < 0) {
			// still can not decide .. place randomly
			besttI = rand()%NUMBER_OF_CHASSIS;
			besttJ = rand()%NUMBER_OF_SERVERS_IN_ONE_CHASSIS;
		}
		(*ppServers)[besttI][besttJ]->AssignOneVM(pqVMsToGo->front());
		pqVMsToGo->pop();
	}
}

UniformTaskSchedulingAlgorithm::UniformTaskSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX])
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;

	PREDICITIONSHEDULER = false;
}

void UniformTaskSchedulingAlgorithm::AssignVMs()
{
	// assign VMs to Servers
	while (!pqVMsToGo->empty()) {
		// assign with qWaitingVMs.top()
		int bestI, bestJ = -1;
		FLOATINGPOINT bestAvailability = 1000000.0; // any big number
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ((*ppServers)[i][j]->IsOFF()) continue;
				FLOATINGPOINT sumofVM = (*ppServers)[i][j]->VMRequiresThisMuchCPUScale();
				if (sumofVM < bestAvailability) { // assign to the machine with least jobs
					bestI = i; bestJ = j; 
					bestAvailability = sumofVM;
				}
			}
		}
		if (bestI < 0 || bestJ < 0)
			cout << "Error: No servers to assign a VM" << endl;
		(*ppServers)[bestI][bestJ]->AssignOneVM(pqVMsToGo->front());
		pqVMsToGo->pop();
	}
}

BestPerformanceSchedulingAlgorithm::BestPerformanceSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX])
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;

	PREDICITIONSHEDULER = false;
}

void BestPerformanceSchedulingAlgorithm::AssignVMs()
{
	// assign VMs to Servers
	while (!pqVMsToGo->empty()) {
		// assign with qWaitingVMs.top()
		int bestI, bestJ = -1;
		FLOATINGPOINT bestAvailability = 1000000.0; // any big number
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ((*ppServers)[i][j]->IsOFF()) continue;
				FLOATINGPOINT sumofVM = (*ppServers)[i][j]->VMRequiresThisMuchUtilization();
				FLOATINGPOINT maxutil = (*ppServers)[i][j]->MaxUtilization();
				FLOATINGPOINT ratio = (sumofVM/maxutil);
				if (ratio < bestAvailability) {
					bestI = i; bestJ = j;
					bestAvailability = ratio;
				}
			}
		}
		if (bestI < 0 || bestJ < 0)
			cout << "Error: No servers to assign a VM" << endl;
		(*ppServers)[bestI][bestJ]->AssignOneVM(pqVMsToGo->front());
		pqVMsToGo->pop();
	}

}

MinHRSchedulingAlgorithm::MinHRSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX])
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;

	PREDICITIONSHEDULER = false;

	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i) {
		HRF[i] = 0.0;
		for (int j=0; j<SIZE_OF_HR_MATRIX; ++j) {
			HRF[i] += (*pHeatRecirculationMatrixD)[j][i];
		}
	}

	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i) {
		FLOATINGPOINT reference = 100000.0; // some big number
		int toInsert = -1;
		for (int j=0; j<SIZE_OF_HR_MATRIX; ++j) {
			if (HRF[j] < reference) {
				toInsert = j;
				reference = HRF[j];
			}
		}
		HRF[toInsert] = 10000000.0; // some bigger number
		HRFSortedIndex[i] = toInsert;
	}
	
	srand((unsigned int)time(NULL));
}

void MinHRSchedulingAlgorithm::AssignVMs()
{
	// assign VMs to Servers
	while (!pqVMsToGo->empty()) {
		// assign with qWaitingVMs.top()
		int bestI, bestJ = -1;
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ((*ppServers)[HRFSortedIndex[i]][j]->IsOFF()) continue;

				FLOATINGPOINT sumofVM = (*ppServers)[HRFSortedIndex[i]][j]->VMRequiresThisMuchCPUScale() + pqVMsToGo->front()->GetCPULoadRatio();
				if (sumofVM >= NUMBER_OF_CORES_IN_ONE_SERVER) // it's full with the new workload
					continue;
				bestI = HRFSortedIndex[i];
				bestJ = j;
				goto ASSIGNING_FINISHED;
			}
		}
		// data center is full. assign similarly but more aggressively.
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ((*ppServers)[HRFSortedIndex[i]][j]->IsOFF()) continue;

				FLOATINGPOINT sumofVM = (*ppServers)[HRFSortedIndex[i]][j]->VMRequiresThisMuchCPUScale();
				if (sumofVM >= NUMBER_OF_CORES_IN_ONE_SERVER) // it's full even before placing the workload
					continue;
				bestI = HRFSortedIndex[i];
				bestJ = j;
				goto ASSIGNING_FINISHED;
			}
		}
		// still could not decide, which means, every one is full. locate workload randomly.
		bestI = rand()%NUMBER_OF_CHASSIS;
		bestJ = rand()%NUMBER_OF_SERVERS_IN_ONE_CHASSIS;
ASSIGNING_FINISHED:
		(*ppServers)[bestI][bestJ]->AssignOneVM(pqVMsToGo->front());
		pqVMsToGo->pop();
	}
}

XintSchedulingAlgorithm::XintSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX])
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;

	PREDICITIONSHEDULER = false;
}

void XintSchedulingAlgorithm::AssignVMs()
{
	// assign VMs to Servers
	while (!pqVMsToGo->empty()) {
		int bestI, bestJ = -1;
		FLOATINGPOINT thishastobemin = 1000.0;
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ((*ppServers)[i][j]->IsOFF()) continue;
				(*ppServers)[i][j]->AssignOneVM(pqVMsToGo->front()); // temporarilly assign to i,j
				FLOATINGPOINT local_thishastobemin = GetHighestTemperatureIncrease();
				if (thishastobemin > local_thishastobemin) {
					bestI = i; bestJ = j;
					thishastobemin = local_thishastobemin;
				}
				(*ppServers)[i][j]->RemoveTheLastAssignedVM(); // de-assign after calculating hr effect
			}
		}
		(*ppServers)[bestI][bestJ]->AssignOneVM(pqVMsToGo->front());
		pqVMsToGo->pop();
	}
}

FLOATINGPOINT XintSchedulingAlgorithm::GetHighestTemperatureIncrease()
{
	FLOATINGPOINT powerDraw[SIZE_OF_HR_MATRIX];
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		powerDraw[i] = 0.0;
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			powerDraw[i] += (*ppServers)[i][j]->GetPowerDraw();
		}
	}
	FLOATINGPOINT tempIncrease;
	FLOATINGPOINT biggestTempIncrease = -100.0;
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		tempIncrease = 0.0;
		for (int j=0; j<NUMBER_OF_CHASSIS; ++j) {
			tempIncrease += powerDraw[j]*(*pHeatRecirculationMatrixD)[i][j];
		}
		if (tempIncrease > biggestTempIncrease)
			biggestTempIncrease = tempIncrease;
	}
	return biggestTempIncrease;
}


CenterRackFirstSchedulingAlgorithm::CenterRackFirstSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX])
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;

	PREDICITIONSHEDULER = false;

	int machineNumber[SIZE_OF_HR_MATRIX] = {
		11,36,37,12,13,38,39,14,15,40,
		6,16,31,41,42,32,17,7,8,18,33,43,44,34,19,9,10,20,35,45,
		1,21,26,46,47,27,22,2,3,23,28,48,49,29,24,4,5,25,30,50
	};
/*
	11,12,13,14,15,
	36,37,38,39,40,

	6,7,8,9,10,
	16,17,18,19,20,
	31,32,33,34,35,
	41,42,43,44,45,

	1,2,3,4,5,
	21,22,23,24,25,
	26,27,28,29,30,
	46,47,48,49,50,
*/

	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i)
		HRFSortedIndex[i] = machineNumber[i] -1;
}

void CenterRackFirstSchedulingAlgorithm::AssignVMs()
{
	// assign VMs to Servers
	while (!pqVMsToGo->empty()) {
		// assign with qWaitingVMs.top()
		int bestI, bestJ = -1;
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ((*ppServers)[HRFSortedIndex[i]][j]->IsOFF()) continue;

				FLOATINGPOINT sumofVM = (*ppServers)[HRFSortedIndex[i]][j]->VMRequiresThisMuchCPUScale() + pqVMsToGo->front()->GetCPULoadRatio();
				if (sumofVM >= NUMBER_OF_CORES_IN_ONE_SERVER) // it's full with the new workload
					continue;
				bestI = HRFSortedIndex[i];
				bestJ = j;
				goto ASSIGNING_FINISHED;
			}
		}
		// data center is full. assign similarly but more aggressively.
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ((*ppServers)[HRFSortedIndex[i]][j]->IsOFF()) continue;

				FLOATINGPOINT sumofVM = (*ppServers)[HRFSortedIndex[i]][j]->VMRequiresThisMuchCPUScale();
				if (sumofVM >= NUMBER_OF_CORES_IN_ONE_SERVER) // it's full even before placing the workload
					continue;
				bestI = HRFSortedIndex[i];
				bestJ = j;
				goto ASSIGNING_FINISHED;
			}
		}
		// still could not decide, which means, every one is full. locate workload randomly.
		bestI = rand()%NUMBER_OF_CHASSIS;
		bestJ = rand()%NUMBER_OF_SERVERS_IN_ONE_CHASSIS;
ASSIGNING_FINISHED:
		(*ppServers)[bestI][bestJ]->AssignOneVM(pqVMsToGo->front());
		pqVMsToGo->pop();
	}
}


TwoDimensionVASchedulingAlgorithm::TwoDimensionVASchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], ServersPOOL* ppool)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;
	ppollServers = ppool;

	PREDICITIONSHEDULER = false;

	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i) {
		HRF[i] = 0.0;
		for (int j=0; j<SIZE_OF_HR_MATRIX; ++j) {
			HRF[i] += (*pHeatRecirculationMatrixD)[j][i];
		}
	}
}


     
void TwoDimensionVASchedulingAlgorithm::AssignVMs()
{
  int k = 0; 
  int sumRemovePOOL = 0; 
  int full = 0;
  POOL sv;


  SORTSERVER serverScheduling[CHASSIxSERVER];

  if (!pqVMsToGo->empty()) {
     for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	     for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
		     if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	
		        continue;
		     }
			 serverScheduling[k].chassi = i;
		     serverScheduling[k].server = j;
		     serverScheduling[k].temperature = (*ppServers)[i][j]->CurrentInletTemperature();
			 serverScheduling[k].temperatureFuture = 0.0;
		     serverScheduling[k].utilizationCPU = (*ppServers)[i][j]->VMRequiresThisMuchUtilization();
			 serverScheduling[k].ranking = (0.40*(serverScheduling[k].temperature/32)) + (0,10*(HRF[i]/0,001)) + (0,50*(serverScheduling[k].utilizationCPU));
		     serverScheduling[k].assignVM = false;
		     k++;
	     }
     }
     quickSort(serverScheduling, 0, k-1);
  } 

  // assign VMs to Servers
  while (!pqVMsToGo->empty())  {
	    full = 0;
        for (int l=0; l < k; ++l) {
            if (!pqVMsToGo->empty()) {
			   if ((serverScheduling[l].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) < 0.90) {
			      serverScheduling[l].utilizationCPU += (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER);
			      (*ppServers)[serverScheduling[l].chassi][serverScheduling[l].server]->AssignOneVM(pqVMsToGo->front());
			      pqVMsToGo->pop();
		       }
			   else {
                  if (serverScheduling[l].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER) >= .90) {
			         full++;
			      }
			      continue;
		       }
		    }
		    else {
			   break;
		    }
		}
		if (full == k) {
		   sv = ppollServers->RemoveServerPOOL(ppServers);
		   if (sv.chassi != -1) {
              serverScheduling[k].chassi = sv.chassi;
		      serverScheduling[k].server = sv.server;
		      serverScheduling[k].temperature = (*ppServers)[sv.chassi][sv.server]->CurrentInletTemperature();
		      serverScheduling[k].utilizationCPU = (*ppServers)[sv.chassi][sv.server]->VMRequiresThisMuchUtilization();
		      serverScheduling[k].temperatureFuture = 0;
    	      serverScheduling[k].ranking = (0.40*(serverScheduling[k].temperature/32)) + (0,10*(HRF[sv.chassi]/0,001)) + (0,50*(serverScheduling[k].utilizationCPU));
		      (*ppServers)[serverScheduling[k].chassi][serverScheduling[k].server]->AssignOneVM(pqVMsToGo->front());
		      pqVMsToGo->pop();
		      k++;
		      sumRemovePOOL++;
		      full = 0;
		   }
		   else{
			   cout << "Error! no servers in the pool - Scheduling Algorithm - TwoDimensionVASchedulingAlgorithm!!!" << endl;
			   break;
		   }
		}
  }
 
  if (sumRemovePOOL > 0) {
	  ppollServers->ServerPowerON(ppServers, sumRemovePOOL);
  }
 
}


TwoDimensionVAPredictionSchedulingAlgorithm::TwoDimensionVAPredictionSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], ServersPOOL* ppool, FLOATINGPOINT* tempArcondicionado)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;
	ppollServers = ppool;
	totalScheduling = 0;
	PREDICITIONSHEDULER = true;
	temp = tempArcondicionado;

	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i) {
		HRF[i] = 0.0;
		for (int j=0; j<SIZE_OF_HR_MATRIX; ++j) {
			HRF[i] += (*pHeatRecirculationMatrixD)[j][i];
		}
	}
}
void TwoDimensionVAPredictionSchedulingAlgorithm::AssignVMs()
{
  SORTSERVER serverScheduling[CHASSIxSERVER];
  vector<double> predictionTemp;
  vector<FLOATINGPOINT> vectorTemporary;
  POOL sv;

  int k = 0; 
  int sumRemovePOOL = 0; 
  int iServer = 0; int full = 0;
    
  if (!pqVMsToGo->empty()) {
     for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	     for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) { 
	         if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	 
		 	    continue;
		     }
			 serverScheduling[k].chassi = i;
		     serverScheduling[k].server = j;
		     serverScheduling[k].temperature = (*ppServers)[i][j]->CurrentInletTemperature();
			 serverScheduling[k].utilizationCPU = (*ppServers)[i][j]->VMRequiresThisMuchUtilization();

  		     if ((*ppServers)[i][j]->returnSizeVectorTemperature() == SIZEWINDOWNPREDICTION) {
				predictionTemp = runPolynom( (*ppServers)[i][j]->returnVectorTemperature() );  // run prediction 
			 	if (!predictionTemp.empty()) {
		           if ( predictionTemp[CLENGTH-1] >= 10 && predictionTemp[CLENGTH-1] <= 32) {
				      serverScheduling[k].temperatureFuture = predictionTemp[CLENGTH-1];
  			          (*ppServers)[i][j]->insertTemperaturePredictionServer(predictionTemp[CLENGTH-1]);
				      (*ppServers)[i][j]->insertTimePredictionServer((*ppServers)[i][j]->returnClock()+(CLENGTH*MONITORINGTIME));
				      (*ppServers)[i][j]->addHitPrediction();
			   	      //cout << "Servidor " << i << " " << j << " Temperatura predita " << predictionTemp[CLENGTH-1] << " Tempo de Simulação " << (*ppServers)[i][j]->returnClock() << " Tempo Futuro " << (*ppServers)[i][j]->returnClock()+(CLENGTH*MONITORINGTIME) << endl;
				   }
			  	   else {
                      if ( predictionTemp[0] >= 10 && predictionTemp[0] <= 32) {
					     serverScheduling[k].temperatureFuture = predictionTemp[0];
						 (*ppServers)[i][j]->insertTemperaturePredictionServer(predictionTemp[0]);
				         (*ppServers)[i][j]->insertTimePredictionServer((*ppServers)[i][j]->returnClock()+(CLENGTH*MONITORINGTIME));
				         (*ppServers)[i][j]->addHitPrediction();
						 //cout << "Servidor " << i << " " << j << " Temperatura predita [0] " << predictionTemp[0] << " Tempo de Simulação " << (*ppServers)[i][j]->returnClock() << " Tempo Futuro " << (*ppServers)[i][j]->returnClock()+(CLENGTH*MONITORINGTIME) << endl;
				      }
				      else {
					     (*ppServers)[i][j]->addErrorPrediction();
					     serverScheduling[k].temperatureFuture = (*ppServers)[i][j]->CurrentInletTemperature();
    	  	 	         //cout << "Erro no Servidor " << i << " " << j << " Temperatura predita " << predictionTemp[CLENGTH-1] << " Tempo de Simulação " << (*ppServers)[i][j]->returnClock() << " Temperatura Atual " << (*ppServers)[i][j]->CurrentInletTemperature() << endl;
				         //vectorTemporary = (*ppServers)[i][j]->returnVectorTemperature();
					     //cout << "Temperatura do Servidor " << i << " " << j;
				         //for (int l=0; l < vectorTemporary.size(); l++){
					     //    cout << " " << vectorTemporary[l];
				         //}
					     //cout << endl;
					     //cout << "predição do Servidor " << i << " " << j;
				         //for (int k=0; k < predictionTemp.size(); k++){
					     //    cout << " " << predictionTemp[k];
				         //} 
					     //cout << endl;
				      }
				  }
				  serverScheduling[k].ranking = (0.40*(serverScheduling[k].temperatureFuture/32)) + (0.10*(HRF[i]/0.001)) + (0.50*(serverScheduling[k].utilizationCPU));
				}
				predictionTemp.erase(predictionTemp.begin(), predictionTemp.end());
			 }
			 else {
				serverScheduling[k].temperatureFuture = 0;
    	        serverScheduling[k].ranking = (0.40*(serverScheduling[k].temperature/32)) + (0.10*(HRF[i]/0.001)) + (0.50*(serverScheduling[k].utilizationCPU));
			 }	 
    	     k++;
	     }
     }
     quickSort(serverScheduling, 0, k-1);
  } 

  // assign VMs to Servers
  while (!pqVMsToGo->empty())  {
	    full = 0;
        for(int i=0; i < k; i++) {
           if (!pqVMsToGo->empty()) {
			  if (((serverScheduling[i].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) <= 0.90) && (serverScheduling[i].temperature <= 29))  {
			     //cout << "Servidor " << serverScheduling[i].chassi << " " << serverScheduling[i].server << " recebeu uma VM com " << (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER) << " %CPU" << " Temperatura " << (*ppServers)[serverScheduling[i].chassi][serverScheduling[i].server]->CurrentInletTemperature() << " Score " << serverScheduling[i].ranking << endl;
    		     serverScheduling[i].utilizationCPU += (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER);
			     (*ppServers)[serverScheduling[i].chassi][serverScheduling[i].server]->AssignOneVM(pqVMsToGo->front());
				 totalScheduling += 1;
			     pqVMsToGo->pop();
		      }
			  else {
                 if ((serverScheduling[i].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER) > 0.90) || (serverScheduling[i].temperature > 29)) {
					full++;
				 }
				 continue;
		      }
		   }
		   else {
			  break;
		   }
		}
		if (full == k) {
		   sv = ppollServers->RemoveServerPOOL(ppServers);
		   if (sv.chassi != -1) {
              serverScheduling[k].chassi = sv.chassi;
		      serverScheduling[k].server = sv.server;
		      serverScheduling[k].temperature = (*ppServers)[sv.chassi][sv.server]->CurrentInletTemperature();
		      serverScheduling[k].utilizationCPU = (*ppServers)[sv.chassi][sv.server]->VMRequiresThisMuchUtilization();
		      serverScheduling[k].temperatureFuture = 0;
    	      serverScheduling[k].ranking = (0.40*(serverScheduling[k].temperature/32)) + (0.10*(HRF[sv.chassi]/0.001)) + (0.50*(serverScheduling[k].utilizationCPU));
		      (*ppServers)[serverScheduling[k].chassi][serverScheduling[k].server]->AssignOneVM(pqVMsToGo->front());
			  totalScheduling += 1;
		      pqVMsToGo->pop();
		      //cout << "Moveu do POOL Servidor " << serverScheduling[k].chassi << " " << serverScheduling[k].server << " Temperatura " << serverScheduling[k].temperature << " CPU " << serverScheduling[k].utilizationCPU << " ranking " << serverScheduling[k].ranking << endl;
		      k++;
		      sumRemovePOOL++;
		      full = 0;
		   }
		   else{
			   cout << "Error! no servers in the pool - Scheduling Algorithm - TwoDimensionVAPredictionSchedulingAlgorithm" << endl;
			   break;
		   }
		}
  }
 
  if (sumRemovePOOL > 0) {
	  ppollServers->ServerPowerON(ppServers, sumRemovePOOL);
  }

  predictionTemp.clear();
  vectorTemporary.clear();

/*  
// VERSÃO B
  iServer = 0;

  while (!pqVMsToGo->empty())  {
	    if (((serverScheduling[iServer].utilizationCPU + pqVMsToGo->front()->GetCPULoadRatio()) <= NUMBER_OF_CORES_IN_ONE_SERVER)) {
 		   serverScheduling[iServer].utilizationCPU += pqVMsToGo->front()->GetCPULoadRatio();
		   (*ppServers)[serverScheduling[iServer].posi][serverScheduling[iServer].posj]->AssignOneVM(pqVMsToGo->front());
		   pqVMsToGo->pop();
		   continue;
		}
		else {
		   iServer += 1;
		   if ( (iServer > k-1) && (!pqVMsToGo->empty()) ) {
		      sv = ppollServers->RemoveServerPOOL(ppServers);

			  serverScheduling[k].posi = sv.chassi;
		      serverScheduling[k].posj = sv.server;
		      serverScheduling[k].temperature = (*ppServers)[sv.chassi][sv.server]->CurrentInletTemperature();
		      serverScheduling[k].utilizationCPU = (*ppServers)[sv.chassi][sv.server]->VMRequiresThisMuchCPUScale();
		      serverScheduling[k].temperatureFuture = 0;
    	      serverScheduling[k].ranking = (0.40*(serverScheduling[k].temperature/32)) + (0,10*(HRF[sv.chassi]/0,001)) + (0,50*(serverScheduling[k].utilizationCPU/NUMBER_OF_CORES_IN_ONE_SERVER));
		      (*ppServers)[serverScheduling[k].posi][serverScheduling[k].posj]->AssignOneVM(pqVMsToGo->front());
		      pqVMsToGo->pop();
		      //cout << "Moveu do POOL Servidor " << serverScheduling[k].posi << " " << serverScheduling[k].posj << " Temperatura " << serverScheduling[k].temperature << " CPU " << serverScheduling[k].utilizationCPU << " ranking " << serverScheduling[k].ranking << endl;
		      k++;
		      sumRemovePOOL++;
			  continue;
		   }
		   else {
			  continue;
		   }
	    }
  }
  
  if (sumRemovePOOL > 0) {
	  //cout << "ligar " << sumRemovePOOL << " Servidores " << endl;
	  ppollServers->ServerPowerON(ppServers, sumRemovePOOL);
  }*/
}


TwoDimensionVBSchedulingAlgorithm::TwoDimensionVBSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX])
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;

	PREDICITIONSHEDULER = false;

	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i) {
		HRF[i] = 0.0;
		for (int j=0; j<SIZE_OF_HR_MATRIX; ++j) {
			HRF[i] += (*pHeatRecirculationMatrixD)[j][i];
		}
	}
}

void TwoDimensionVBSchedulingAlgorithm::AssignVMs()
{
  int k = 0;
  bool scheduling = false;
  double averageTemp = 0;

  SORTSERVER serverScheduling[CHASSIxSERVER];  

  if (!pqVMsToGo->empty()) {
     for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	     for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
	         if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	 
		 	    continue;
		     }
		     serverScheduling[k].chassi = i;
		     serverScheduling[k].server = j;
		     serverScheduling[k].temperature = (*ppServers)[i][j]->CurrentInletTemperature();
			 serverScheduling[k].temperatureFuture = 0.0;
		     serverScheduling[k].utilizationCPU = (*ppServers)[i][j]->VMRequiresThisMuchCPUScale();
		     serverScheduling[k].ranking = (0.30*(serverScheduling[k].temperature/35)) + (0.20*(HRF[i]/0,01)) + (0.50*(serverScheduling[k].utilizationCPU/10));
		     serverScheduling[k].assignVM = false;
		     k++;
	     }
     }
     quickSort(serverScheduling, 0, k-1);
	 averageTemp = (serverScheduling[0].temperature+serverScheduling[k-1].temperature) / 2;
  } 

ASSIGNING_VMS:
  // assign VMs to Servers 1º Round
  while (!pqVMsToGo->empty()) {
        scheduling = false;
        for (int i=0; i < k; i++) {
     	   if (((serverScheduling[i].utilizationCPU + pqVMsToGo->front()->GetCPULoadRatio()) <= NUMBER_OF_CORES_IN_ONE_SERVER) && (serverScheduling[i].assignVM == false) && (serverScheduling[i].temperature <= averageTemp)) {
    		   serverScheduling[i].utilizationCPU += pqVMsToGo->front()->GetCPULoadRatio();
			   serverScheduling[i].assignVM = true;
			  (*ppServers)[serverScheduling[i].chassi][serverScheduling[i].server]->AssignOneVM(pqVMsToGo->front());
			  pqVMsToGo->pop();
			  scheduling = true;
			  break;
			}
		}
		if (scheduling == false) {      // assign VMs to Servers 2º Round
		   int ii = 0;
		   while (!pqVMsToGo->empty()) {
			     if (ii <= k-1) {
			        if (((serverScheduling[ii].utilizationCPU + pqVMsToGo->front()->GetCPULoadRatio()) <= NUMBER_OF_CORES_IN_ONE_SERVER)) {
                 	   serverScheduling[ii].utilizationCPU += pqVMsToGo->front()->GetCPULoadRatio();
					   (*ppServers)[serverScheduling[ii].chassi][serverScheduling[ii].server]->AssignOneVM(pqVMsToGo->front());
			           pqVMsToGo->pop();
			           ii++;
					   continue;
					} 
					else {
					   ii++;
					   continue;
					}
				}
		        else {
					if (!pqVMsToGo->empty()) {
	   				   for (int i=0; i < k; i++) {
					       serverScheduling[i].assignVM = false;
					   }
					   goto ASSIGNING_VMS;
				    }
		       }
	      }
       }
   }
}




TwoDimensionVCPredictionSchedulingAlgorithm::TwoDimensionVCPredictionSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], FLOATINGPOINT* tempArcondicionado)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;
	totalScheduling = 0;
	PREDICITIONSHEDULER = true;
	temp = tempArcondicionado;


	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i) {
		HRF[i] = 0.0;
		for (int j=0; j<SIZE_OF_HR_MATRIX; ++j) {
			HRF[i] += (*pHeatRecirculationMatrixD)[j][i];
		}
	}
}
void TwoDimensionVCPredictionSchedulingAlgorithm::AssignVMs()
{
  SORTSERVER serverScheduling[CHASSIxSERVER];
  vector<double> predictionTemp;
  vector<FLOATINGPOINT> vectorTemporary;
  

  int k = 0; 
  int sumRemovePOOL = 0; 
  int iServer = 0; int full = 0;
    
  if (!pqVMsToGo->empty()) {
     for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	     for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) { 
	         if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	 
		 	    continue;
		     }
			 serverScheduling[k].chassi = i;
		     serverScheduling[k].server = j;
		     serverScheduling[k].temperature = (*ppServers)[i][j]->CurrentInletTemperature();
			 serverScheduling[k].utilizationCPU = (*ppServers)[i][j]->VMRequiresThisMuchUtilization();

  		     if ((*ppServers)[i][j]->returnSizeVectorTemperature() == SIZEWINDOWNPREDICTION) {
				predictionTemp = runPolynom( (*ppServers)[i][j]->returnVectorTemperature() );  // run prediction 
			 	if (!predictionTemp.empty()) {
		           if ( predictionTemp[CLENGTH-1] >= 10 && predictionTemp[CLENGTH-1] <= 32) {
				      serverScheduling[k].temperatureFuture = predictionTemp[CLENGTH-1];
  			          (*ppServers)[i][j]->insertTemperaturePredictionServer(predictionTemp[CLENGTH-1]);
				      (*ppServers)[i][j]->insertTimePredictionServer((*ppServers)[i][j]->returnClock()+(CLENGTH*MONITORINGTIME));
				      (*ppServers)[i][j]->addHitPrediction();
				   }
			  	   else {
                      if ( predictionTemp[0] >= 10 && predictionTemp[0] <= 32) {
					     serverScheduling[k].temperatureFuture = predictionTemp[0];
						 (*ppServers)[i][j]->insertTemperaturePredictionServer(predictionTemp[0]);
				         (*ppServers)[i][j]->insertTimePredictionServer((*ppServers)[i][j]->returnClock()+(CLENGTH*MONITORINGTIME));
				         (*ppServers)[i][j]->addHitPrediction();
				      }
				      else {
					     (*ppServers)[i][j]->addErrorPrediction();
					     serverScheduling[k].temperatureFuture = (*ppServers)[i][j]->CurrentInletTemperature();
				      }
				  }
				   // cout << " Sever " << serverScheduling[k].chassi << " " << serverScheduling[k].server << "  Temp Futu " << 0.40*(serverScheduling[k].temperatureFuture/32) << " Recirc " << 0.10*(HRF[i]/0.001) << " CPU " << 0.50*(serverScheduling[k].utilizationCPU) << endl;
				  serverScheduling[k].ranking = (0.40*(serverScheduling[k].temperatureFuture/32)) + (0.10*(HRF[i]/0.001)) + (0.50*(serverScheduling[k].utilizationCPU));
				}
				predictionTemp.erase(predictionTemp.begin(), predictionTemp.end());
			 }
			 else {
				serverScheduling[k].temperatureFuture = 0;
     	        serverScheduling[k].ranking = (0.40*(serverScheduling[k].temperature/32)) + (0.10*(HRF[i]/0.001)) + (0.50*(serverScheduling[k].utilizationCPU));
 			    // cout << " Sever " << serverScheduling[k].chassi << " " << serverScheduling[k].server << "  Temp " << 0.40*(serverScheduling[k].temperature/32) << " Recirc " << 0.10*(HRF[i]/0.001) << " CPU " << 0.50*(serverScheduling[k].utilizationCPU) << endl;
			 }	 
    	     k++;
	     }
     }
     quickSort(serverScheduling, 0, k-1);
  } 


  // assign VMs to Servers
  while (!pqVMsToGo->empty())  {
        for(int i=0; i < k; i++) {
           if (!pqVMsToGo->empty()) {
			  if (((serverScheduling[i].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) <= 0.90) && (serverScheduling[i].temperature <= 29) && (serverScheduling[i].temperatureFuture <= 29)) {
				  //cout << " VM escalonada " << (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER) << " Server " <<  serverScheduling[i].chassi << " " << serverScheduling[i].server << " Temp " << serverScheduling[i].temperature << endl;
    		     serverScheduling[i].utilizationCPU += (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER);
			     (*ppServers)[serverScheduling[i].chassi][serverScheduling[i].server]->AssignOneVM(pqVMsToGo->front());
			     pqVMsToGo->pop();
				 totalScheduling += 1;
		      }
			  else {
   				 continue;
		      }
		   }
		   else {
			  break;
		   }
		}
  }
 
  predictionTemp.clear();
  vectorTemporary.clear();

}
