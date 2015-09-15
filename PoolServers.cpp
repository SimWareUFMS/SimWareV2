#include "PoolServers.h"
#include <algorithm>

using namespace std;


ServersPOOL::ServersPOOL(void)
{
 minimumServersPool = 99999;
 clockSimulation = 0;

 serversPowerOFF.clear();
 serversPowerON.clear();
 serversPOOL.clear();
 windowsSmallVMSArrived.clear();
 windowsMediumVMSArrived.clear();
 windowsBigVMSArrived.clear();
}

ServersPOOL::~ServersPOOL(void)
{
 serversPowerOFF.clear();
 serversPowerON.clear();
 serversPOOL.clear();
 windowsSmallVMSArrived.clear();
 windowsMediumVMSArrived.clear();
 windowsBigVMSArrived.clear();
}

void ServersPOOL::InsertVectorServersPOOL(int Chassi, int Server, double Temperature)
{
     POOL server;

     server.chassi = Chassi;
	 server.server = Server;
	 server.temperature = Temperature;

	 serversPOOL.push_back(server);
}

void ServersPOOL::InsertVectorServersPowerOFF(int Chassi, int Server, double Temperature)
{
     POOL server;

	 server.chassi = Chassi;
	 server.server = Server;
	 server.temperature = Temperature;

	 serversPowerOFF.push_back(server);
}


void ServersPOOL::InsertVectorServersPowerON(int Chassi, int Server, double Temperature)
{
     POOL server;

	 server.chassi = Chassi;
	 server.server = Server;
	 server.temperature = Temperature;

	 serversPowerON.push_back(server);
}


void ServersPOOL::ListVectorServersPOOL(void)
{
	cout << "Vector Size " <<  serversPOOL.size() << "   " << endl;
	for(int i = 0; i <  serversPOOL.size(); i++){
		cout <<  serversPOOL[i].chassi << " " << serversPOOL[i].server << " " << serversPOOL[i].temperature << endl;
	}
}

void ServersPOOL::ListVectorPowerOFF(void)
{
	cout << "Vector Size " << serversPowerOFF.size() << "   " << endl;
	for(int i = 0; i <serversPowerOFF.size(); i++){
		cout << serversPowerOFF[i].chassi << " " << serversPowerOFF[i].server << " " << serversPowerOFF[i].temperature << endl;
	}
}

void ServersPOOL:: SortVectorServersPowerOFF(void)
{

  sort(serversPowerOFF.begin(), serversPowerOFF.end(), sortPOOLServer);
}

void ServersPOOL::SortVectorServersPOOL(void)
{
  sort(serversPOOL.begin(), serversPOOL.end(), sortPOOLServer);
}

POOL ServersPOOL::RemoveServerPOOL(Server* (*sv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX])
{
 POOL server;
     
 if (serversPOOL.size() > 0) {
    //update the temperature
	for (int i=0; i < serversPOOL.size(); i++){
        serversPOOL[i].temperature=(*sv)[serversPOOL[i].chassi][serversPOOL[i].server]->CurrentInletTemperature();
    }

    SortVectorServersPOOL();
 }

 if (serversPOOL.size() > 0) {
	 server.chassi = serversPOOL[0].chassi;
	 server.server = serversPOOL[0].server;
	 (*sv)[serversPOOL[0].chassi][serversPOOL[0].server]->removePOOL();
	 serversPOOL.erase(serversPOOL.begin());
 }
 else {
	   server.chassi = -1;
	   server.server = -1;
 }
 
 //calculates the minimum pool size

 if (serversPOOL.size() < minimumServersPool) {
    minimumServersPool = serversPOOL.size();
 }

 return server;
}

void ServersPOOL::ServerPowerON(Server* (*sv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], int wakeUP)
{
 
  if (serversPowerOFF.size() > 0) {
     //update the temperature
	 for (int i=0; i < serversPowerOFF.size(); i++){
	     serversPowerOFF[i].temperature=(*sv)[serversPowerOFF[i].chassi][serversPowerOFF[i].server]->CurrentInletTemperature();
     }
   
     SortVectorServersPowerOFF();
  }

  for (int j=0; j < wakeUP; j++) {
      if (serversPowerOFF.size() > 0) {
	     InsertVectorServersPowerON(serversPowerOFF[0].chassi, serversPowerOFF[0].server, serversPowerOFF[0].temperature);
		 (*sv)[serversPowerOFF[0].chassi][serversPowerOFF[0].server]->TurnON();
		 (*sv)[serversPowerOFF[0].chassi][serversPowerOFF[0].server]->timePowerOnServer(clockSimulation);
		 serversPowerOFF.erase(serversPowerOFF.begin());
	  }
	  else {
         cout << "All servers are power on!!!" << endl;
		 break;
	  }
  }
}

void ServersPOOL::DefineNewSizePool(void)
{
  vector<double> predictionSmallVMSArrived;
  vector<double> predictionMediumVMSArrived;
  vector<double> predictionBigVMSArrived;

  double serversSmallInPool = 0.00;
  double serversMediumInPool = 0.00;
  double serversBigInPool = 0.00;



  if (windowsSmallVMSArrived.size() == SIZEWINDOWNPREDICTIONPOOLSERVER) {
     predictionSmallVMSArrived = runRBF(windowsSmallVMSArrived);  // run prediction
	 predictionMediumVMSArrived = runRBF(windowsMediumVMSArrived); 
	 predictionBigVMSArrived = runRBF(windowsBigVMSArrived); 
  
	 serversSmallInPool = 0;
	 serversMediumInPool = 0;
     serversBigInPool = 0;

	 if (!predictionSmallVMSArrived.empty()) {
        if (predictionSmallVMSArrived[0] >=0) {
           serversSmallInPool = predictionSmallVMSArrived[0];  // next time        
		}
	 }
     if (!predictionMediumVMSArrived.empty()) {
	    if (predictionMediumVMSArrived[0] >=0) {
           serversMediumInPool = predictionMediumVMSArrived[0];  //next time       
		}
	 }
     if (!predictionBigVMSArrived.empty()) {
	    if (predictionBigVMSArrived[0] >=0) {
           serversBigInPool = predictionBigVMSArrived[0];      //next time       
		}
	 }

	 // calculate

	 serversSmallInPool = ceil( (double) (serversSmallInPool * 0.35) / (double) NUMBER_OF_CORES_IN_ONE_SERVER);  
	 serversMediumInPool = ceil( (double) (serversMediumInPool * 0.65) / (double) NUMBER_OF_CORES_IN_ONE_SERVER);  
	 serversBigInPool = ceil( (double) (serversBigInPool * 1.00) / (double) NUMBER_OF_CORES_IN_ONE_SERVER);

	 //cout << "previsão para os próximos dois minutos " << serversSmallInPool + serversMediumInPool + serversBigInPool  << endl;

    /*if ((serversSmallInPool + serversMediumInPool + serversBigInPool) < returnMinimumServersPool()){
       //pPool->DefineNewSizePool(serversSmallInPool + serversMediumInPool + serversBigInPool);
      }*/

     predictionSmallVMSArrived.clear();
     predictionMediumVMSArrived.clear();
     predictionBigVMSArrived.clear();
  }
}


void ServersPOOL::EveryASecond(Server* (*psv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX])
{

 // Turn OFF servers and server insert in the pool	
 if (clockSimulation == 1) {
    PowerOFFDC(psv);
 }

 // verifies that the vms finished migrate

 for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
  	 for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
	  	 if (((*psv)[i][j]->IsOFF()) || (!(*psv)[i][j]->IsMIGRATING()) || ((*psv)[i][j]->IsPOOL()) || ((*psv)[i][j]->IsINITIALIZING())) {
		    continue;
		 }
		 (*psv)[i][j]->CheckFinishMove(clockSimulation);
         if ((!(*psv)[i][j]->hasVMs()) && ((*psv)[i][j]->IsMIGRATING())) { 
			InsertVectorServersPowerOFF(i, j, (*psv)[i][j]->CurrentInletTemperature());
			(*psv)[i][j]->TurnOFF();  
		}
	}
 }

  // checks if any server finished power on
 BEFORE_ITERATOR:
 for (vector<POOL>::iterator it = serversPowerON.begin(); it != serversPowerON.end(); ++it) {
	 if ((*psv)[(*it).chassi][(*it).server]->returnFinishPowerOnServer() == clockSimulation) {
		(*psv)[(*it).chassi][(*it).server]->FinishInitialization();
        (*psv)[(*it).chassi][(*it).server]->addPOOL();
	    InsertVectorServersPOOL((*it).chassi, (*it).server, (*it).temperature);
	    serversPowerON.erase(it);
		goto BEFORE_ITERATOR;
	 }
 }
 
}

void ServersPOOL::InsertWindowsSmallVMSArrived(int VMsSmall)
{
  if (VMsSmall != 0) {
     windowsSmallVMSArrived.push_back(VMsSmall);
  }
  else {
     windowsSmallVMSArrived.push_back(1);
  }

  if (windowsSmallVMSArrived.size() > SIZEWINDOWNPREDICTIONPOOLSERVER) {  // Prediction Window  
     windowsSmallVMSArrived.erase(windowsSmallVMSArrived.begin());  //sliding window
  }
}


void ServersPOOL::InsertWindowsMediumVMSArrived(int VMsMedium)
{
  if (VMsMedium != 0) {
     windowsMediumVMSArrived.push_back(VMsMedium);
  }
  else {
     windowsMediumVMSArrived.push_back(1);
  }
  if (windowsMediumVMSArrived.size() > SIZEWINDOWNPREDICTIONPOOLSERVER) {  // Prediction Window  
     windowsMediumVMSArrived.erase(windowsMediumVMSArrived.begin());  //sliding window
  }
}


void ServersPOOL::InsertWindowsBigVMSArrived(int VMsBig)
{
  if (VMsBig != 0) {
	 windowsBigVMSArrived.push_back(VMsBig);
  }
  else {
     windowsBigVMSArrived.push_back(1);
  }
  if (windowsBigVMSArrived.size() > SIZEWINDOWNPREDICTIONPOOLSERVER) {  // Prediction Window  
     windowsBigVMSArrived.erase(windowsBigVMSArrived.begin());  //sliding window
  }
 }

void ServersPOOL::PowerOFFDC(Server* (*psv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX])
{
 POOL servers;	
 vector<POOL> sortTempServers;
 int sumPowerOFF = 0; 
 int sumPoolServers = 0; 

 for (int i=0; i < NUMBER_OF_CHASSIS; ++i) {
     for (int j=0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
   	     if ((*psv)[i][j]->IsOFF() || (*psv)[i][j]->hasVMs() || (*psv)[i][j]->IsMIGRATING() || (*psv)[i][j]->IsINITIALIZING()) {
            continue;
		 }
		 servers.chassi = i;
		 servers.server = j;
		 servers.temperature = (*psv)[i][j]->CurrentInletTemperature();
		 sortTempServers.push_back(servers);
     }
 }

 //order servers by temperature
 sort(sortTempServers.begin(), sortTempServers.end(), sortTemperature);

 //power off 80% servers
 for (int i=0; i < (0.80 * (NUMBER_OF_CHASSIS*NUMBER_OF_SERVERS_IN_ONE_CHASSIS)); i++) {	
     if ((*psv)[sortTempServers[i].chassi][sortTempServers[i].server]->IsOFF()){
        continue;
	 }
     InsertVectorServersPowerOFF(sortTempServers[i].chassi, sortTempServers[i].server, (*psv)[sortTempServers[i].chassi][sortTempServers[i].server]->CurrentInletTemperature());
	(*psv)[sortTempServers[i].chassi][sortTempServers[i].server]->TurnOFF();
	 sumPowerOFF += 1;
 }
 
 // delete power off servers
 sortTempServers.erase(sortTempServers.begin(), sortTempServers.begin()+sumPowerOFF);

 // insert 10% servers in the pool
 for (int j=0; j < (0.10 * (NUMBER_OF_CHASSIS*NUMBER_OF_SERVERS_IN_ONE_CHASSIS)); j++) {	
     if ((*psv)[sortTempServers[j].chassi][sortTempServers[j].server]->IsOFF()){
        continue;
	 }
	 (*psv)[sortTempServers[j].chassi][sortTempServers[j].server]->addPOOL();
	 InsertVectorServersPOOL(sortTempServers[j].chassi, sortTempServers[j].server, (*psv)[sortTempServers[j].chassi][sortTempServers[j].server]->CurrentInletTemperature());
	 sumPoolServers += 1;
 }
 // clear vector servers
 sortTempServers.clear(); 
}

void ServersPOOL::UpdateClockSimulation(int clocksimul)
{
  clockSimulation = clocksimul;
}


void ServersPOOL::Print(void)
{

 cout << "Small VMS " << endl;
 for (int i=0; i < windowsSmallVMSArrived.size(); i++) {
	 cout << windowsSmallVMSArrived[i] << endl;
 }

 cout << endl;
 cout << "Medium VMS " << endl;
 for (int j=0; j < windowsMediumVMSArrived.size(); j++) {
	 cout << windowsMediumVMSArrived[j] << endl;
 }

 cout << endl;
 cout << "Big VMS " << endl;
 for (int k=0; k < windowsBigVMSArrived.size(); k++) {
	 cout << windowsBigVMSArrived[k] << endl;
 }
 
}
