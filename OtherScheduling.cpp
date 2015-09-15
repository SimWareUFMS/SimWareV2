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
			  totalScheduling += 1;
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
					   totalScheduling += 1;
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
