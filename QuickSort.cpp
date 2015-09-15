#include "stdio.h" 
#include "Constants.h"
#include "VirtualMachine.h"
#include <vector>

void quickSort(SORTSERVER *server, int begin, int end)
{
   int i, j;
   double half;
   SORTSERVER aux;

   i = begin;
   j = end;

   half = server[(begin + end) / 2].ranking;

   do
   {
      while(server[i].ranking < half) {
           i++;
	  }
      while(server[j].ranking > half) {
           j--;
	  }
      if (i <= j) {
         aux = server[i];
         server[i] = server[j];
         server[j] = aux;
         i++;
         j--;
      }
   }while(i <= j);

   if (begin < j) { 
	  quickSort(server, begin, j);
   }
   if (i < end) {
      quickSort(server, i, end);
   }
}

bool sortCPU(VirtualMachine* VM_A, VirtualMachine* VM_B)
{
  if (VM_A->GetCPULoadRatio() > VM_B->GetCPULoadRatio()) // > UP or < DOWN
	  return true;
  return false;
}

bool sortTemperature(POOL SV_A, POOL SV_B)
{
	if (SV_A.temperature > SV_B.temperature) // > UP or < DOWN
	  return true;
  return false;
}


bool sortPOOLServer(POOL S_A, POOL S_B)
{
	if (S_A.temperature < S_B.temperature) // > UP or < DOWN
	  return true;
  return false;
}


bool sortCPUOPT(STRUCTEMP SVOPT_A, STRUCTEMP SVOPT_B)
{
	if (SVOPT_A.utilizationCPU < SVOPT_B.utilizationCPU) // > UP or < DOWN
	  return true;
  return false;
}

bool sortTempOPT(STRUCTEMP SVOP_A, STRUCTEMP SVOP_B)
{
	if (SVOP_A.temperature > SVOP_B.temperature) // > UP or < DOWN
	  return true;
  return false;
}