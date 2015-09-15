#pragma once

#include <list>
#include <vector>
#include <string>
#include <iostream>

#include "Constants.h"
#include "Links.h"

using namespace std;

class Switch
{
public:
	Switch(void);
	~Switch(void);

	inline bool IsOFF(void) { return isOFF; }
	inline void PowerOFF() {isOFF = true; }
	inline void PowerON() {isOFF = false; }

	//virtual void EstablishingConnection(int speed_, double delay_, bool active_, Server* server_, Switch* switch_) = 0;
	virtual double ReturnsEnergyConsumption(void) = 0;
	virtual string GetClassName(void) = 0;
	
protected:
  // Switch energy model
  double eChassis;
  double eLineCard;
  double ePort;

  // Switch energy model
  bool switches_eDVFS_enabled;	// enable DVFS
  bool switches_eDNS_enabled;	// enable DNS
  double switches_eDNS_delay;	//  time required to power down the switch defaul = 0.01

  bool isOFF;
};

//*------------------------------------------------------------
class Switch_ACCESS : public Switch
{
public:
	Switch_ACCESS(double eCha, double eLine, double ePor, bool eDVFS, bool eDNS, double eDNSDelay);
	~Switch_ACCESS(void);

	void EstablishingConnection_SV_to_SW(int speed_, double delay_, bool active_, Server* server_, Switch* switch_);
	void EstablishingConnection_Top_To_Bottom(int speed_, double delay_, bool active_, Switch* switch_, Switch* switch_t);

	Link_SV_to_SW* ReturnLink(void);	
	
	double ReturnsEnergyConsumption(void);
	bool ChecksIfThereIsActiveConnections(void);
	void PowerOFFAllLinks(void);
	
	string GetClassName(void) { return name; }
private: 
    vector<Link_SV_to_SW *> Bottom;
	vector<Link_SW_to_SW *> Top;
	string name;
};

//*------------------------------------------------------------
class Switch_AGGREGATION : public Switch
{
public:
	Switch_AGGREGATION(double eCha, double eLine, double ePor, bool eDVFS, bool eDNS, double eDNSDelay);
	~Switch_AGGREGATION(void);

	void EstablishingConnection_Bottom_To_Top(int speed_, double delay_, bool active_, Switch* switch_, Switch* switch_t);
    void EstablishingConnection_Top_To_Bottom(int speed_, double delay_, bool active_, Switch* switch_, Switch* switch_t);

	double ReturnsEnergyConsumption(void);

	string GetClassName(void) { return name; }

private:
    vector<Link_SW_to_SW *> Top;
	vector<Link_SW_to_SW *> Bottom;
	string name;
};

//*------------------------------------------------------------
class Switch_CORE : public Switch
{
public:
	Switch_CORE(double eCha, double eLine, double ePor, bool eDVFS, bool eDNS, double eDNSDelay);
	~Switch_CORE(void);

	void EstablishingConnection_Bottom_To_Top(int speed_, double delay_, bool active_, Switch* switch_, Switch* switch_t);
	
	double ReturnsEnergyConsumption(void);
	
	string GetClassName(void) { return name; }

private:
	vector<Link_SW_to_SW *> Top;
	vector<Link_SW_to_SW *> Bottom;
	string name;
};
