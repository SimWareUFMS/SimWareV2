/*
 *   This file is part of TISEAN
 *
 *   Copyright (c) 1998-2007 Rainer Hegger, Holger Kantz, Thomas Schreiber
 *
 *   TISEAN is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   TISEAN is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with TISEAN; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
/*Author: Rainer Hegger Last modified: May 23th, 1998 */


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "tisean/tisean_cec.h"
#include "tisean/tsa.h"
#include <iostream>

#define SIZE_STEP 1000

using namespace std;

void check_alloc(void *pnt)
{
  if (pnt == NULL) {
    fprintf(stderr,"check_alloc: Couldn't allocate enough memory. Exiting\n");
    exit(CHECK_ALLOC_NOT_ENOUGH_MEMORY);
  }
}

void variance(double *s,unsigned long l,double *av,double *var)
{
  unsigned long i=0;
  double h=0.0;
  
  *av=0.0; 
  *var=0.0;

  for (i=0;i<l;i++) {
      h=s[i];
      *av += h;
      *var += h*h;
  }
  
  *av /= (double)l;
  *var=sqrt(fabs((*var)/(double)l-(*av)*(*av)));
  
  if (*var == 0.0) {
     fprintf(stderr,"Variance of the data is zero. Exiting!\n\n");
     exit(VARIANCE_VAR_EQ_ZERO);
  }
}


void solvele(double **mat,double *vec,unsigned int n)
{
  double vswap=0.0,*mswap,*hvec,max=0.0,h=0.0,pivot=0.0,q=0.0;
  int i=0,j=0,k=0,maxi=0;

  for (i=0;i<n-1;i++) {
      max=fabs(mat[i][i]);
      maxi=i;
      for (j=i+1;j<n;j++)
          if ((h=fabs(mat[j][i])) > max) {
	         max=h;
   	         maxi=j;
          }
      if (maxi != i) {
         mswap=mat[i];
         mat[i]=mat[maxi];
         mat[maxi]=mswap;
         vswap=vec[i];
         vec[i]=vec[maxi];
         vec[maxi]=vswap;
      }
    
      hvec=mat[i];
      pivot=hvec[i];
      if (fabs(pivot) == 0.0) {
         fprintf(stderr,"Singular matrix! Exiting!\n");
         exit(SOLVELE_SINGULAR_MATRIX);
      }
      for (j=i+1;j<n;j++) {
          q= -mat[j][i]/pivot;
          mat[j][i]=0.0;
          for (k=i+1;k<n;k++) {
	          mat[j][k] += q*hvec[k];
		  }
          vec[j] += q*vec[i];
      }
  }
  vec[n-1] /= mat[n-1][n-1];
  for (i=n-2;i>=0;i--) {
      hvec=mat[i];
      for (j=n-1;j>i;j--) {
          vec[i] -= hvec[j]*vec[j];
	  }
      vec[i] /= hvec[i];
  }
}

/*void solvele(double **mat,double *vec,unsigned int n)
{
  double vswap,*mswap,*hvec;
  double max = 0.00;
  double h = 0.00;
  double pivot = 0.00;
  double q = 0.00;
  int i,j,k,maxi = 0;

  for (i=0;i<n-1;i++) {
    max=fabs(mat[i][i]);
    maxi=i;
    for (j=i+1;j<n;j++)
      if ((h=fabs(mat[j][i])) > max) {
    	 max=h;
	     maxi=j;
      }
    if (maxi != i) {
      mswap=mat[i];
      mat[i]=mat[maxi];
      mat[maxi]=mswap;
      vswap=vec[i];
      vec[i]=vec[maxi];
      vec[maxi]=vswap;
    }
    
    hvec=mat[i];
    pivot=hvec[i];
    if (fabs(pivot) == 0.0) {
      fprintf(stderr,"Singular matrix! Exiting!\n");
      exit(SOLVELE_SINGULAR_MATRIX);
    }
    for (j=i+1;j<n;j++) {
      q= -mat[j][i]/pivot;
      mat[j][i]=0.0;
      for (k=i+1;k<n;k++)
	mat[j][k] += q*hvec[k];
      vec[j] += q*vec[i];
    }
  }
  vec[n-1] /= mat[n-1][n-1];
  for (i=n-2;i>=0;i--) {
    hvec=mat[i];
    for (j=n-1;j>i;j--)
      vec[i] -= hvec[j]*vec[j];
    vec[i] /= hvec[i];
  }
}
*/

void rescale_data(double *x,unsigned long l,double *min,double *interval)
{
  int i=0;
  
  *min=*interval=x[0];
  
  for (i=1;i<l;i++) {
      if (x[i] < *min) *min=x[i];
      if (x[i] > *interval) *interval=x[i];
  }
  *interval -= *min;

  if (*interval != 0.0) {
     for (i=0;i<l;i++)
         x[i]=((x[i]- *min) / *interval);
  }
  else {
     fprintf(stderr,"rescale_data: data ranges from %e to %e. It makes\n"
	    "\t\tno sense to continue. Exiting!\n\n",*min,*min+(*interval));
     exit(RESCALE_DATA_ZERO_INTERVAL);
 }
}

#undef SIZE_STEP