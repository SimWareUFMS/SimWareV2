#include <list>

#include "Constants.h"
#include "Topology.h"

using namespace std;


Topology::Topology(void)
{
  Switches.clear();
  Racks.clear();

  vTotalEnergyAccessSwitchSparseLog.clear();
  vTotalEnergyAggregationSwitchSparseLog.clear();
  vTotalEnergyCoreSwitchSparseLog.clear();

  vTotalEnergyAccessSwitch = 0.00;
  vTotalEnergyAggregationSwitch = 0.00;
  vTotalEnergyCoreSwitch = 0.00;

  vEnergyAccessSwitch = 0.00;
  vEnergyAggregationSwitch = 0.00;
  vEnergyCoreSwitch = 0.00; 

  locSwitchAccess = 0;
  locSwitchAggregation = 0;
  locSwitchCore = 0;
}

Topology::~Topology(void)
{
  Switches.clear();
  Racks.clear();

  vTotalEnergyAccessSwitchSparseLog.clear();
  vTotalEnergyAggregationSwitchSparseLog.clear();
  vTotalEnergyCoreSwitchSparseLog.clear();
}


void Topology::CreateTopology(Server* (*tservers)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX])
{
 int NumberOfRacks = 0; 
 int k = 0; int insert = 0;
 int rck = 0;
 int pt = 0;
 int aggre = 0;
 int core = 0;

 // calculates the number of racks
 NumberOfRacks = ceil((double) NUMBER_OF_CHASSIS /  (double) NUMBER_CHASSI_RACK);

 // create racks
 for(int i=0;i < NumberOfRacks; i++) {
	 Racks.push_back(new Rack(i+1));
 }
   
 // join servers on rack
 k=0;
 insert = 0;
 for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
     for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
		 if (insert < (NUMBER_CHASSI_RACK*NUMBER_OF_SERVERS_IN_ONE_CHASSIS)) {
	         Racks[k]->InsertServerToRack((*tservers)[i][j]);
			 (*tservers)[i][j]->insertRackInServer(k); // identifies rack in server
		     insert++;
	     }
		 else {
			 k++;
	         Racks[k]->InsertServerToRack((*tservers)[i][j]);
			 (*tservers)[i][j]->insertRackInServer(k); // identifies rack in server
		     insert=1;
		 }
	 }
 }

 // create switchss access
 for(int i=0;i < SWITCH_ACCESS; i++) {
	 Switches.push_back(new Switch_ACCESS(EACCE_CHASSIS, EACCE_LINECARD, EACCE_PORT, false, false, 0));
 }


 //connects the servers with the access switches
 rck = 0;
 pt = 0;
 for (int l=0; l < SWITCH_ACCESS; l+=LEVEL_OF_REDUNDANCY_OF_LINKS_N0N1) {
	 for (int m=0; m < (NUMBER_CHASSI_RACK*NUMBER_OF_SERVERS_IN_ONE_CHASSIS); m++) {
		 if (pt <= (NUMBER_OF_PORTS_SWITCH_ACCESS - LEVEL_OF_REDUNDANCY_OF_LINKS_N0N1)) {
            if (m < Racks[rck]->ReturnTotalServerFromRack()) {
			   for (int n=0; n < LEVEL_OF_REDUNDANCY_OF_LINKS_N0N1; n++) {
                   // cout << "l " << l << " m " << m << " n " << n << " rck " << rck << " pt " << pt << endl;
				   (*(Switch_ACCESS*) Switches[l+n]).EstablishingConnection_SV_to_SW(SWITCH_ACCESS_SPEED, SWITCH_ACCESS_DELAY, true, Racks[rck]->ReturnServerFromRack(m), Switches[l+n]); // create link
 		           (*Racks[rck]->ReturnServerFromRack(m)).insertLinkInServer((*(Switch_ACCESS*) Switches[l+n]).ReturnLink()); // identifies link in server
		       } 
			   pt++;
		    }
		    else {
		       break;
		    }
		 } 
		 else {
            pt = 0;
			l += LEVEL_OF_REDUNDANCY_OF_LINKS_N0N1;
			if (m < Racks[rck]->ReturnTotalServerFromRack()) {
			   for (int n=0; n < LEVEL_OF_REDUNDANCY_OF_LINKS_N0N1; n++) {
				   // cout << "l " << l << " m " << m << " n " << n << " rck " << rck << " pt " << pt << endl;
				   (*(Switch_ACCESS*) Switches[l+n]).EstablishingConnection_SV_to_SW(SWITCH_ACCESS_SPEED, SWITCH_ACCESS_DELAY, true, Racks[rck]->ReturnServerFromRack(m), Switches[l+n]); // create link
 		           (*Racks[rck]->ReturnServerFromRack(m)).insertLinkInServer((*(Switch_ACCESS*) Switches[l+n]).ReturnLink()); // identifies link in server
			   }
			   pt++;
		    }
		 }
	 }
	 rck++;
	 pt = 0;
	 if (rck >= Racks.size()) {
		break;
	 }
 }
 
 // create switchss aggregation
 locSwitchAggregation = Switches.size();

 for (int i=0;i < SWITCH_AGGREGATION; i++) {
	 Switches.push_back(new Switch_AGGREGATION(EAGGR_CHASSIS, EAGGR_LINECARD, EAGGR_PORT, false, false, 0));
 }

  //connects access switches with the aggregation switches
 aggre = locSwitchAggregation;

 for (int i=0; i < Switches.size(); i++) {
     if (Switches[i]->GetClassName() != "Switch_ACCESS") {
	    break;
	 }
	 for (int n=0; n < LEVEL_OF_REDUNDANCY_OF_LINKS_N1N2; n++) {
		 // cout << "i " << i << " n " << n << " aggre " << aggre+n << endl;
         (*(Switch_ACCESS*) Switches[i]).EstablishingConnection_Top_To_Bottom(SWITCH_AGGREGATION_SPEED, SWITCH_AGGREGATION_DELAY, true, Switches[i], Switches[aggre+n]); // create link
         (*(Switch_AGGREGATION*) Switches[aggre+n]).EstablishingConnection_Bottom_To_Top(SWITCH_AGGREGATION_SPEED, SWITCH_AGGREGATION_DELAY, true, Switches[aggre+n], Switches[i]);
	 }
     aggre += LEVEL_OF_REDUNDANCY_OF_LINKS_N1N2; 

	 if (aggre >= Switches.size()) {
		 aggre = locSwitchAggregation;
	 }
 }
 

 // create switches core
 locSwitchCore = Switches.size();

 for(int i=0;i < SWITCH_CORE; i++) {
	 Switches.push_back(new Switch_CORE(ECORE_CHASSIS, ECORE_LINECARD, ECORE_PORT, false, false, 0));
 }

 //connects aggregation switches with the core switches
 core = locSwitchCore;

 for (int i=locSwitchAggregation; i < Switches.size(); i++) {
     if (Switches[i]->GetClassName() != "Switch_AGGREGATION") {
	    break;
	 }
	 for (int n=0; n < LEVEL_OF_REDUNDANCY_OF_LINKS_N2N3; n++) {
		  //cout << "i " << i << " n " << n << " cotr " << core+n << endl;
         (*(Switch_AGGREGATION*) Switches[i]).EstablishingConnection_Top_To_Bottom(SWITCH_CORE_SPEED, SWITCH_CORE_DELAY, true, Switches[i], Switches[core+n]); // create link
         (*(Switch_CORE*) Switches[core+n]).EstablishingConnection_Bottom_To_Top(SWITCH_CORE_SPEED, SWITCH_CORE_DELAY, true, Switches[core+n], Switches[i]);
	 }
     core += LEVEL_OF_REDUNDANCY_OF_LINKS_N2N3; 

	 if (core >= Switches.size()) {
		core = locSwitchCore;
	 }
 }

 cout << "Topology Created !!!!" << endl;
 	 
}

void Topology::EveryASecond(int clock_)
{

  // power off access switches
  for (int i=0; i < SWITCH_ACCESS; i++) {
       if (!(*(Switch_ACCESS*) Switches[i]).ChecksIfThereIsActiveConnections()) {
          Switches[i]->PowerOFF();
	   }
       if (((*(Switch_ACCESS*) Switches[i]).ChecksIfThereIsActiveConnections()) && (Switches[i]->IsOFF())) {
          Switches[i]->PowerON();
	   }
  }


  for (int i=0; i < Switches.size(); i++) {
	  if (Switches[i]->IsOFF()) {
		  continue;
	  }
	  if (Switches[i]->GetClassName() == "Switch_ACCESS") {
 		 vEnergyAccessSwitch += Switches[i]->ReturnsEnergyConsumption();
	  }
	  else {
		 if (Switches[i]->GetClassName() == "Switch_AGGREGATION") {
			vEnergyAggregationSwitch += Switches[i]->ReturnsEnergyConsumption(); 
		 }
		 else {
			vEnergyCoreSwitch += Switches[i]->ReturnsEnergyConsumption();
		 }
	 }
  } 

  if (clock_%PERIODIC_LOG_INTERVAL==0) {
	 vTotalEnergyAccessSwitchSparseLog.push_back(vEnergyAccessSwitch/PERIODIC_LOG_INTERVAL);
     vTotalEnergyAggregationSwitchSparseLog.push_back(vEnergyAggregationSwitch/PERIODIC_LOG_INTERVAL);
     vTotalEnergyCoreSwitchSparseLog.push_back(vEnergyCoreSwitch/PERIODIC_LOG_INTERVAL);

	 vTotalEnergyAccessSwitch += vEnergyAccessSwitch;
	 vTotalEnergyAggregationSwitch += vEnergyAggregationSwitch;
	 vTotalEnergyCoreSwitch += vEnergyCoreSwitch;
 
	 vEnergyAccessSwitch = 0.00;
	 vEnergyAggregationSwitch = 0.00;
     vEnergyCoreSwitch = 0.00; 
  }

};


double Topology::ReturnTotalEnergyAccessSwitchSparseLog(void)
{
  double sum = 0.00;

  for(int i=0; i < vTotalEnergyAccessSwitchSparseLog.size(); i++) {
	  sum += vTotalEnergyAccessSwitchSparseLog[i];
  }
  return sum;
}

double Topology::ReturnTotalEnergyAggregationSwitchSparseLog(void)
{
  double sum = 0.00;

  for(int i=0; i < vTotalEnergyAggregationSwitchSparseLog.size(); i++) {
	  sum += vTotalEnergyAggregationSwitchSparseLog[i];
  }
  return sum;
}

double Topology::ReturnTotalEnergyCoreSwitchSparseLog(void)
{
  double sum = 0.00;

  for(int i=0; i <vTotalEnergyCoreSwitchSparseLog.size(); i++) {
	  sum += vTotalEnergyCoreSwitchSparseLog[i];
  }
  return sum;
}

void Topology::PrintPowerDrawAccessSwitch(void)
{
  for (int i=0; i<vTotalEnergyAccessSwitchSparseLog.size(); ++i) {
	  cout << vTotalEnergyAccessSwitchSparseLog[i] << "\t";
  }
  cout << endl;
}
void Topology::PrintPowerDrawAggregationSwitch(void)
{
  for (int i=0; i< vTotalEnergyAggregationSwitchSparseLog.size(); ++i) {
	  cout << vTotalEnergyAggregationSwitchSparseLog[i] << "\t";
  }
  cout << endl;
}
void Topology::PrintPowerDrawCoreSwitch(void)
{
  for (int i=0; i< vTotalEnergyCoreSwitchSparseLog.size(); ++i) {
	  cout << vTotalEnergyCoreSwitchSparseLog[i] << "\t";
  }
  cout << endl;
}

void Topology::PrintTotalPowerDrawSwitches(void)
{
  for (int i=0; i< vTotalEnergyAccessSwitchSparseLog.size(); ++i) {
	  cout << vTotalEnergyAccessSwitchSparseLog[i] + vTotalEnergyAggregationSwitchSparseLog[i] + vTotalEnergyCoreSwitchSparseLog[i] << "\t";
  }
  cout << endl;
}


double Topology::ReturnPowerDrawSwitches(int ind)
{
  return vTotalEnergyAccessSwitchSparseLog[ind] + vTotalEnergyAggregationSwitchSparseLog[ind] + vTotalEnergyCoreSwitchSparseLog[ind];
}


void Topology::PrintTotalEnergyAccessSwitchSparseLog(void)
{
  for (int i=0; i<vTotalEnergyAccessSwitchSparseLog.size(); ++i) {
	  cout << (vTotalEnergyAccessSwitchSparseLog[i] * PERIODIC_LOG_INTERVAL)<< "\t";
  }
  cout << endl;
}
void Topology::PrintTotalEnergyAggregationSwitchSparseLog(void)
{
  for (int i=0; i< vTotalEnergyAggregationSwitchSparseLog.size(); ++i) {
	  cout << (vTotalEnergyAggregationSwitchSparseLog[i] * PERIODIC_LOG_INTERVAL) << "\t";
  }
  cout << endl;
}
void Topology::PrintTotalEnergyCoreSwitchSparseLog(void)
{
  for (int i=0; i< vTotalEnergyCoreSwitchSparseLog.size(); ++i) {
	  cout << (vTotalEnergyCoreSwitchSparseLog[i] * PERIODIC_LOG_INTERVAL) << "\t";
  }
  cout << endl;
}

void Topology::PrintTotalEnergySwitchesSparseLog(void)
{
  for (int i=0; i< vTotalEnergyAccessSwitchSparseLog.size(); ++i) {
	  cout << ((vTotalEnergyAccessSwitchSparseLog[i] + vTotalEnergyAggregationSwitchSparseLog[i] + vTotalEnergyCoreSwitchSparseLog[i]) * PERIODIC_LOG_INTERVAL) << "\t";
  }
  cout << endl;
}