#pragma once

#include "Constants.h"

using namespace std;

class Switch;
class Server;

class Link_SV_to_SW 
{
public:
	Link_SV_to_SW(int spe_, double del_, bool act_, Server* ser_, Switch* swi_);
    ~Link_SV_to_SW(void);
	inline bool IsActive(void) { return active_; }
	inline void PowerOFF(void) { active_ = false; } 
	inline void PowerON(void)  { active_ = true; }
private:
	int speed_;  
	double delay_;  
	bool active_;
	Server* Bottom_;
	Switch* Top_;
};

class Link_SW_to_SW 
{
public:
	Link_SW_to_SW(int spe_, double del_, bool act_, Switch* swi_a, Switch* swi_b);
    ~Link_SW_to_SW(void);
	inline bool IsActive(void) { return active; }
	inline void PowerOFF(void) { active = false; } 
	inline void PowerON(void)  { active = true; }
private:
	int speed;  
	double delay;  
	bool active;
	Switch* Bottom;
	Switch* Top;
};





