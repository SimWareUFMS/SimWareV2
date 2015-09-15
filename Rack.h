#pragma once

#include <vector>

#include "Constants.h"
#include "Server.h"

using namespace std;

extern int NUMBER_CHASSI_RACK;


class Rack 
{
public:
	Rack(int id);
	~Rack(void);
	void InsertServerToRack(Server* rserver);
	Server* ReturnServerFromRack(int pos); 
	int ReturnTotalServerFromRack(void);

private:
	int identifier;
	vector<Server*> Server_in_Rack;
	
};