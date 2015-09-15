#include "OptimizationAlgorithmServers.h"
#include <vector>


OptimizationAlgorithmServers::~OptimizationAlgorithmServers(void)
{
}


PoliceLowUtilization::PoliceLowUtilization(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX])
{
	ppoServers = pso;
}

int PoliceLowUtilization::OptimizationServers(int clockSimulation)
{
	vector<STRUCTEMP> ServerOPT;
    vector<STRUCTEMP> VMsToMigrate;
	vector<VirtualMachine *> listVMs;

	STRUCTEMP temp;

	int l = 0; 
	int totalMigration = 0;

	for (int i=0; i<NUMBER_OF_CHASSIS; i++) {
	    for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; j++) {
            if ( ((*ppoServers)[i][j]->IsOFF()) || ((*ppoServers)[i][j]->IsPOOL()) || ((*ppoServers)[i][j]->IsMIGRATING()) || ((*ppoServers)[i][j]->IsINITIALIZING()) || ((*ppoServers)[i][j]->VMRequiresThisMuchUtilization() == 0)) { 	 
		 	   continue;
			}
			temp.chassi = i;
			temp.server = j;
			temp.temperature = (*ppoServers)[i][j]->CurrentInletTemperature();
			temp.utilizationCPU =(*ppoServers)[i][j]->VMRequiresThisMuchUtilization();
			ServerOPT.push_back(temp);
		}
	}

	// order by CPU utilization  
	sort(ServerOPT.begin(), ServerOPT.end(), sortCPUOPT); 

	for (int k = 0; k < ServerOPT.size(); k++) {
        if  ((ServerOPT[k].utilizationCPU > 0) && (ServerOPT[k].utilizationCPU <= 0.30)) {
			temp.chassi = ServerOPT[k].chassi;
			temp.server = ServerOPT[k].server;
			temp.temperature = ServerOPT[k].temperature;
			temp.utilizationCPU = ServerOPT[k].utilizationCPU;
			VMsToMigrate.push_back(temp);
		}
		else {
			break;
		}
 	}

	if (VMsToMigrate.size() > 1) {
   	   l = ServerOPT.size()-1; 
       for (int k=0; k < VMsToMigrate.size(); k++){
	       while (l != k) {
		         if ((VMsToMigrate[k].utilizationCPU + ServerOPT[l].utilizationCPU) <= 0.90) {
		            listVMs = (*ppoServers)[VMsToMigrate[k].chassi][VMsToMigrate[k].server]->GetALLVMs(clockSimulation);
					totalMigration += listVMs.size();
					(*ppoServers)[VMsToMigrate[k].chassi][VMsToMigrate[k].server]->setMigrating(true);
					(*ppoServers)[ServerOPT[l].chassi][ServerOPT[l].server]->MoveVMs(listVMs);
					ServerOPT[l].utilizationCPU=(*ppoServers)[ServerOPT[l].chassi][ServerOPT[l].server]->VMRequiresThisMuchUtilization();
					break;
                 }
		         else {
		            l--;
		         }
	      }
	   }
	}
	ServerOPT.clear();
    VMsToMigrate.clear();
	listVMs.clear();

	return totalMigration;
}


PoliceHighTemperature::PoliceHighTemperature(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], const FLOATINGPOINT HeatRecirculationMatrix[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], ServersPOOL* opool)
{
	ppoServers = pso;
	opoolServers = opool;

	for (int i=0; i < SIZE_OF_HR_MATRIX; i++) {
		for (int j=0; j < SIZE_OF_HR_MATRIX; j++) {
			HeatRecirculation[i][j] = HeatRecirculationMatrix[i][j];
		}
	}
}

int PoliceHighTemperature::OptimizationServers(int clockSimulation)
{
	vector<STRUCTEMP> ServerOPT;
    vector<STRUCTEMP> VMsToMigrate;
	vector<VirtualMachine *> listVMs;

	STRUCTEMP temp;
	POOL sv;

	int l = 0; int totalMigration = 0; int lowChassi = 0; int lowServer = 0;
	double lowTemp = 0.00; double calcTemp = 0.00; int totalVMs = 0;
	int sumRemovePOOL = 0;

	FLOATINGPOINT power[SIZE_OF_HR_MATRIX];
	FLOATINGPOINT powerserver = 0.0;
	FLOATINGPOINT hFromJtoI = 0.0;
	FLOATINGPOINT estimateTemperatureServer[CHASSIS][SERVERS];

	memset((void *)estimateTemperatureServer, 0.00, CHASSIS*SERVERS*sizeof(FLOATINGPOINT));  
	
	for (int i=0; i<NUMBER_OF_CHASSIS; i++) {
	    for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; j++) {
            if (((*ppoServers)[i][j]->IsOFF()) || ((*ppoServers)[i][j]->IsPOOL()) || ((*ppoServers)[i][j]->IsMIGRATING()) || ((*ppoServers)[i][j]->IsINITIALIZING())) { 	 
		 	   continue;
			}
			temp.chassi = i;
			temp.server = j;
			temp.temperature = (*ppoServers)[i][j]->CurrentInletTemperature();
			temp.utilizationCPU =(*ppoServers)[i][j]->VMRequiresThisMuchUtilization();
			ServerOPT.push_back(temp);
		}
	}

	sort(ServerOPT.begin(), ServerOPT.end(), sortTempOPT); // order by Temperature  

	// identifica os servidores acima da temperatura máxima permitida

	for (int k = 0; k < ServerOPT.size(); k++) {
        if  (ServerOPT[k].temperature > 29.99) {
			temp.chassi = ServerOPT[k].chassi;
			temp.server = ServerOPT[k].server;
			temp.temperature = ServerOPT[k].temperature;
			temp.utilizationCPU = ServerOPT[k].utilizationCPU;
			VMsToMigrate.push_back(temp);
		}
		else {
			break;
		}
 	}

	if (VMsToMigrate.size() >= 1) {
       for (int k=0; k < VMsToMigrate.size(); k++) {

		   // Calcula o consumo de todos os chassis
 			for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		        power[i] = 0.0;
		        for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			        power[i] += (*ppoServers)[i][j]->GetPowerDraw();
		        }
	        }
	   
           //calcula o consumo energético do servidor super aquecido
		   powerserver = (*ppoServers)[VMsToMigrate[k].chassi][VMsToMigrate[k].server]->GetPowerDraw();

	       // calcula o calor de gerado por cada chassi
		   for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		       for (int j=0; j<NUMBER_OF_CHASSIS; ++j) {
				   if (VMsToMigrate[k].chassi != j) {
					  FLOATINGPOINT hFromJtoI = (power[j]+powerserver)*HeatRecirculation[i][j];
				   }
				   else {
					   FLOATINGPOINT hFromJtoI = (power[j]-powerserver)*HeatRecirculation[i][j];
				   }
  			       for (int l=0; l < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++l) {
				       estimateTemperatureServer[i][l] += hFromJtoI;
			       } 
		       }
	       }
		   
		  // cout << "Chassi " << ServerOPT[i].chassi << " Servidor " << ServerOPT[i].server << " Temperatua do Servidor " << ServerOPT[i].temperature << " Calor Gerado "  << estimateTemperatureServer[ServerOPT[i].chassi][ServerOPT[i].server] << " Temperatura CRAC " << (*ppoServers)[ServerOPT[i].chassi][ServerOPT[i].server]->ReadSupplyTempToTimingBuffer() << " Temperatura Estimada " << calctemp << " Power Off " << (*ppoServers)[ServerOPT[i].chassi][ServerOPT[i].server]->IsOFF() << endl;

		   for (int i = ServerOPT.size()-1; i >= 0; i--) {
			   //calcula a temperatura estimada com a nova carga de trabalho
			   calcTemp = estimateTemperatureServer[ServerOPT[i].chassi][ServerOPT[i].server] + (*ppoServers)[ServerOPT[i].chassi][ServerOPT[i].server]->ReadSupplyTempToTimingBuffer();
		       // Migra 100%
			   if ((calcTemp <= 27) && ((VMsToMigrate[k].utilizationCPU + ServerOPT[i].utilizationCPU) <= 0.90)) {
		          listVMs = (*ppoServers)[VMsToMigrate[k].chassi][VMsToMigrate[k].server]->GetALLVMs(clockSimulation);
				  totalMigration += listVMs.size();
				  (*ppoServers)[VMsToMigrate[k].chassi][VMsToMigrate[k].server]->setMigrating(true);
				  (*ppoServers)[ServerOPT[i].chassi][ServerOPT[i].server]->MoveVMs(listVMs);
				  ServerOPT[i].utilizationCPU=(*ppoServers)[ServerOPT[i].chassi][ServerOPT[i].server]->VMRequiresThisMuchUtilization();
				  break;
               }
			   else {
				   // Migra 50%
				   if ((calcTemp > 27) && (calcTemp <= 28)  && ((VMsToMigrate[k].utilizationCPU + ServerOPT[i].utilizationCPU) <= 0.90)) {
					  totalVMs = (*ppoServers)[VMsToMigrate[k].chassi][VMsToMigrate[k].server]->HowManyVMs() / 2;
    			      listVMs = (*ppoServers)[VMsToMigrate[k].chassi][VMsToMigrate[k].server]->GetNVMs(totalVMs, clockSimulation);
   			          totalMigration += listVMs.size();
				      (*ppoServers)[VMsToMigrate[k].chassi][VMsToMigrate[k].server]->setMigrating(true);
				      (*ppoServers)[ServerOPT[i].chassi][ServerOPT[i].server]->MoveVMs(listVMs);
				      ServerOPT[i].utilizationCPU=(*ppoServers)[ServerOPT[i].chassi][ServerOPT[i].server]->VMRequiresThisMuchUtilization();
					  break;
				   }
				   else {
					  // Migra 25%
					  if ((calcTemp > 28) && (calcTemp <= 29)  && ((VMsToMigrate[k].utilizationCPU + ServerOPT[i].utilizationCPU) <= 0.90)) {
					     totalVMs = (*ppoServers)[VMsToMigrate[k].chassi][VMsToMigrate[k].server]->HowManyVMs() / 4;
    			         listVMs = (*ppoServers)[VMsToMigrate[k].chassi][VMsToMigrate[k].server]->GetNVMs(totalVMs, clockSimulation);
   			             totalMigration += listVMs.size();
				         (*ppoServers)[VMsToMigrate[k].chassi][VMsToMigrate[k].server]->setMigrating(true);
				         (*ppoServers)[ServerOPT[i].chassi][ServerOPT[i].server]->MoveVMs(listVMs);
				         ServerOPT[i].utilizationCPU=(*ppoServers)[ServerOPT[i].chassi][ServerOPT[i].server]->VMRequiresThisMuchUtilization();
						 break;
				      } 
		           }
			   }
		   }
		   if (!(*ppoServers)[VMsToMigrate[k].chassi][VMsToMigrate[k].server]->IsMIGRATING()) {
 		      // Retira uma Maquina do POOL
			  sv = opoolServers->RemoveServerPOOL(ppoServers);
			  if (sv.chassi != -1) {
				 listVMs = (*ppoServers)[VMsToMigrate[k].chassi][VMsToMigrate[k].server]->GetALLVMs(clockSimulation);
				 totalMigration += listVMs.size();
				 (*ppoServers)[VMsToMigrate[k].chassi][VMsToMigrate[k].server]->setMigrating(true);
				 (*ppoServers)[sv.chassi][sv.server]->MoveVMs(listVMs);

				 temp.chassi = sv.chassi;
				 temp.server = sv.server;
				 temp.temperature = (*ppoServers)[sv.chassi][sv.server]->CurrentInletTemperature();
				 temp.utilizationCPU =(*ppoServers)[sv.chassi][sv.server]->VMRequiresThisMuchUtilization();
			     ServerOPT.push_back(temp);
				 sumRemovePOOL++;
			  }	
			  else{
			     cout << "Error! no servers in the pool - PoliceHighTemperature" << endl;
			     break;
		      }
		   }
	   }
	} 

	if (sumRemovePOOL > 0) {
	   opoolServers->ServerPowerON(ppoServers, sumRemovePOOL);
    }

	ServerOPT.clear();
    VMsToMigrate.clear();
	listVMs.clear();

    return totalMigration;
}


PoliceIdle::PoliceIdle(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], ServersPOOL* opool)
{
	ppoServers = pso;
	opoolServers = opool;
}


int PoliceIdle::OptimizationServers(int clockSimulation)
{
  int powerOff = 0;

  // Power Off Servers idle 
  for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
      for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
		  if (((*ppoServers)[i][j]->IsOFF()) || ((*ppoServers)[i][j]->IsPOOL()) || ((*ppoServers)[i][j]->IsMIGRATING()) || ((*ppoServers)[i][j]->IsINITIALIZING())) {
			 continue;
		  }
          if  (!(*ppoServers)[i][j]->hasVMs()) { 
			  powerOff += 1;
			  opoolServers->InsertVectorServersPowerOFF(i, j, (*ppoServers)[i][j]->CurrentInletTemperature());
			  (*ppoServers)[i][j]->TurnOFF();  
		  }
      }
  }

  return powerOff;
}