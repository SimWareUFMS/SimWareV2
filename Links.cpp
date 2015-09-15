#include "Switches.h"

using namespace std;


Link_SW_to_SW::Link_SW_to_SW(int spe_, double del_, bool act_, Switch* swi_a, Switch* swi_b)
{
	speed = spe_; 
	delay = del_; 
	active = act_;
	Bottom = swi_a;
	Top = swi_b;
}


Link_SW_to_SW::~Link_SW_to_SW(void)
{

}

//*------------------------------------------

Link_SV_to_SW::Link_SV_to_SW(int spe_, double del_, bool act_, Server* ser_, Switch* swi_)
{
	speed_ = spe_; 
	delay_ = del_; 
	active_ = true;
	Bottom_ = ser_;
	Top_ = swi_;
}


Link_SV_to_SW::~Link_SV_to_SW(void)
{

}
