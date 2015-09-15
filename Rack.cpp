#include "Rack.h"


Rack::Rack(int id)
{
 Server_in_Rack.clear();
 identifier = id;
}


Rack::~Rack(void)
{
 Server_in_Rack.clear();
}


void Rack::InsertServerToRack(Server* rserver)
{
 Server_in_Rack.push_back(rserver);
}

Server* Rack::ReturnServerFromRack(int pos)
{
  return Server_in_Rack[pos];
}

int Rack::ReturnTotalServerFromRack(void)
{
  return Server_in_Rack.size();
}