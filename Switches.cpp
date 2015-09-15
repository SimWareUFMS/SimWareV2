#include "Switches.h"

using namespace std;

Switch::Switch(void)
{
  eChassis=0;
  eLineCard=0;
  ePort=0;
  switches_eDVFS_enabled=false;	
  switches_eDNS_enabled=false;	
  switches_eDNS_delay=0.0;
  isOFF = false;
}

Switch::~Switch(void)
{

}

// METHODS ACCESS SWITCH

Switch_ACCESS::Switch_ACCESS(double eCha, double eLine, double ePor, bool eDVFS, bool eDNS, double eDNSDelay)
{
  name = "Switch_ACCESS";
  eChassis = eCha;
  eLineCard = eLine;
  ePort = ePor;
  switches_eDVFS_enabled = eDVFS;
  switches_eDNS_enabled = eDNS;	
  switches_eDNS_delay= eDNSDelay;
  isOFF = false;
}

void Switch_ACCESS::EstablishingConnection_SV_to_SW(int speed_, double delay_, bool active_, Server* server_, Switch* switch_)
{
  Bottom.push_back(new Link_SV_to_SW(speed_, delay_, active_, server_, switch_));
}

void Switch_ACCESS::EstablishingConnection_Top_To_Bottom(int speed_, double delay_, bool active_, Switch* switch_, Switch* switch_t)
{
  Top.push_back(new Link_SW_to_SW(speed_, delay_, active_, switch_, switch_t));
}

Link_SV_to_SW* Switch_ACCESS::ReturnLink(void)
{
   return Bottom[Bottom.size()-1];
}

double Switch_ACCESS::ReturnsEnergyConsumption(void)
{
  double energycosumption = 0.00;
  int port = 0;

  for (vector<Link_SV_to_SW *>::iterator it = Bottom.begin(); it != Bottom.end(); ++it) {
	  if ((*it)->IsActive()) {
	     port++;
	  }
  }
	
  for (vector<Link_SW_to_SW *>::iterator its = Top.begin(); its != Top.end(); ++its) {
	  if ((*its)->IsActive()) {
	     port++;
	  }
  }

  energycosumption = eChassis + eLineCard + (port * ePort);
    
  return energycosumption;
}


bool Switch_ACCESS::ChecksIfThereIsActiveConnections(void)
{
  int port = 0;

  for (vector<Link_SV_to_SW *>::iterator it = Bottom.begin(); it != Bottom.end(); ++it) {
	  if ((*it)->IsActive()) {
	     port++;
	  }
  }

  if (port > 0) {
	 return true;
  }
  else {
	  return false;
  }
}

void Switch_ACCESS::PowerOFFAllLinks(void)
{
  for (vector<Link_SW_to_SW *>::iterator it = Top.begin(); it != Top.end(); ++it) {
	  if ((*it)->IsActive()) {
         (*it)->PowerOFF();
	  }
  }
}



// METHODS  AGGREGATION SWITCH

Switch_AGGREGATION::Switch_AGGREGATION(double eCha, double eLine, double ePor, bool eDVFS, bool eDNS, double eDNSDelay)
{
  name = "Switch_AGGREGATION";
  eChassis = eCha;
  eLineCard = eLine;
  ePort = ePor;
  switches_eDVFS_enabled = eDVFS;
  switches_eDNS_enabled = eDNS;	
  switches_eDNS_delay= eDNSDelay;
  isOFF = false;
}

void Switch_AGGREGATION::EstablishingConnection_Bottom_To_Top(int speed_, double delay_, bool active_, Switch* switch_, Switch* switch_t)
{
  Bottom.push_back(new Link_SW_to_SW(speed_, delay_, active_, switch_, switch_t));
}

void Switch_AGGREGATION::EstablishingConnection_Top_To_Bottom(int speed_, double delay_, bool active_, Switch* switch_, Switch* switch_t)
{
  Top.push_back(new Link_SW_to_SW(speed_, delay_, active_, switch_, switch_t));
}

double Switch_AGGREGATION::ReturnsEnergyConsumption(void)
{
  double energycosumption = 0.00;
  int port = 0;

  for (vector<Link_SW_to_SW *>::iterator it = Bottom.begin(); it != Bottom.end(); ++it) {
	  if ((*it)->IsActive()) {
	     port++;
	  }
  }
	
  for (vector<Link_SW_to_SW *>::iterator its = Top.begin(); its != Top.end(); ++its) {
	  if ((*its)->IsActive()) {
	     port++;
	  }
  }

  energycosumption = eChassis + eLineCard + (port * ePort);

  return energycosumption;
}


// METHODS  CORE SWITCH
Switch_CORE::Switch_CORE(double eCha, double eLine, double ePor, bool eDVFS, bool eDNS, double eDNSDelay)
{
  name = "Switch_CORE";
  eChassis = eCha;
  eLineCard = eLine;
  ePort = ePor;
  switches_eDVFS_enabled = eDVFS;
  switches_eDNS_enabled = eDNS;	
  switches_eDNS_delay= eDNSDelay;
  isOFF = false;
}


void Switch_CORE::EstablishingConnection_Bottom_To_Top(int speed_, double delay_, bool active_, Switch* switch_, Switch* switch_t)
{
  Bottom.push_back(new Link_SW_to_SW(speed_, delay_, active_, switch_, switch_t));
}

double Switch_CORE::ReturnsEnergyConsumption(void)
{
  double energycosumption = 0.00;
  int port = 0;

  for (vector<Link_SW_to_SW *>::iterator it = Bottom.begin(); it != Bottom.end(); ++it) {
	  if ((*it)->IsActive()) {
	     port++;
	  }
  }
	
  for (vector<Link_SW_to_SW *>::iterator its = Top.begin(); its != Top.end(); ++its) {
	  if ((*its)->IsActive()) {
	     port++;
	  }
  }

  energycosumption = eChassis + eLineCard + (port * ePort);

  return energycosumption;
}


