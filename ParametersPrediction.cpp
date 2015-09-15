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
/*Author: Rainer Hegger. Last modified, Sep 20, 2000 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <iostream>
#include <vector>
#include "tisean/tsa.h"


using namespace std;


#define WID_STR "Estimates the time delayed mutual information\n\t\
of the data set"

#define BOX 1024

unsigned long length=ULONG_MAX;
unsigned long exclude_=0;
unsigned int column=1;
unsigned int verbosity_=0xff;
long partitions=16;
long corrlength=20;
long *array_,*h1,*h11,**h2;

// FALSE-NEAREST
char stdo_=1,dimset=0;
char *column_=NULL;
unsigned long theiler=0;
unsigned int delay=1,maxdim=5,minemb=1;
unsigned int comp=1,maxemb=5;
double rt=2.0;
double eps0 = 1.0e-5;
double **series;
double aveps,vareps;
double varianz_;

int ibox=BOX-1;
long **box,*list;
unsigned int *vcomp,*vemb;
unsigned long toolarge=0;


/*void show_options(char *progname)
{
  what_i_do(progname,WID_STR);
  fprintf(stderr," Usage: %s [Options]\n\n",progname);
  fprintf(stderr," Options:\n");
  fprintf(stderr,"Everything not being a valid option will be interpreted"
          " as a possible"
          " datafile.\nIf no datafile is given stdin is read. Just - also"
          " means stdin\n");
  fprintf(stderr,"\t-l # of points to be used [Default is all]\n");
  fprintf(stderr,"\t-x # of lines to be ignored [Default is 0]\n");
  fprintf(stderr,"\t-c column to read  [Default is 1]\n");
  fprintf(stderr,"\t-b # of boxes [Default is 16]\n");
  fprintf(stderr,"\t-D max. time delay [Default is 20]\n");
  fprintf(stderr,"\t-o output file [-o without name means 'datafile'.mut;"
	  "\n\t\tNo -o means write to stdout]\n");
  fprintf(stderr,"\t-V verbosity level [Default is 1]\n\t\t"
          "0='only panic messages'\n\t\t"
          "1='+ input/output messages'\n");
  fprintf(stderr,"\t-h  show these options\n");
  fprintf(stderr,"\n");
  exit(0);
}

void scan_options(int n,char** in)
{
  char *out;

  if ((out=check_option(in,n,'l','u')) != NULL)
    sscanf(out,"%lu",&length);
  if ((out=check_option(in,n,'x','u')) != NULL)
    sscanf(out,"%lu",&exclude_);
  if ((out=check_option(in,n,'c','u')) != NULL)
    sscanf(out,"%u",&column);
  if ((out=check_option(in,n,'b','u')) != NULL)
    sscanf(out,"%lu",&partitions);
  if ((out=check_option(in,n,'D','u')) != NULL)
    sscanf(out,"%lu",&corrlength);
  if ((out=check_option(in,n,'V','u')) != NULL)
    sscanf(out,"%u",&verbosity_);
  if ((out=check_option(in,n,'o','o')) != NULL) {
    stout=0;
    if (strlen(out) > 0)
      file_out=out;
  }
}*/


double make_cond_entropy(long t)
{
  long i=0,j=0,hi=0,hii=0,count=0;
  double hpi=0.00,hpj=0.00,pij=0.00,cond_ent=0.0,norm=0.00;

  for (i=0;i<partitions;i++) {
      h1[i]=h11[i]=0;
      for (j=0;j<partitions;j++) {
          h2[i][j]=0;
	  }
  }
  for (i=0;i<length;i++) {
      if (i >= t) {
         hii=array_[i];
         hi=array_[i-t];
         h1[hi]++;
         h11[hii]++;
         h2[hi][hii]++;
         count++;
      }
  }

  norm = 1.0 / (double)count;
  cond_ent = 0.0;
   
  for (i=0;i<partitions;i++) {
      hpi=(double)(h1[i])*norm;
      if (hpi > 0.0) {
         for (j=0;j<partitions;j++) {
	         hpj=(double)(h11[j])*norm;
	         if (hpj > 0.0) {
	            pij=(double)h2[i][j]*norm;
	            if (pij > 0.0) {
	               cond_ent += pij*log(pij/hpj/hpi);
	            }
	         }
        }
     }
  }

  return cond_ent;
}

int runMutual(vector<double> vetorPredicao)
{
  char stdi=0;
  long tau=0,i=0;
  double *series=NULL;
  double min = 0.00;
  double interval = 0.00;
  double shannon = 0.00;
  double firstmin = 0.00;
  double val = 0.00;
  int delay_ = 0;
  // initializaed var global

  length=ULONG_MAX;
  exclude_=0;
  column=1;   
  verbosity_=0xff;
  partitions=16;
  corrlength=20;

  *array_ = NULL;
  *h1     = NULL;
  *h11    = NULL;
  **h2    = NULL;


  
  /*if (scan_help(argc,argv))
    show_options(argv[0]);
  
  scan_options(argc,argv);
#ifndef OMIT_WHAT_I_DO
  if (verbosity_&VER_INPUT)
    what_i_do(argv[0],WID_STR);
#endif

  infile=search_datafile(argc,argv,&column,verbosity_);
  if (infile == NULL)
    stdi=1;

  if (file_out == NULL) {
    if (!stdi) {
      check_alloc(file_out=(char*)calloc(strlen(infile)+5,(size_t)1));
      strcpy(file_out,infile);
      strcat(file_out,".mut");
    }
    else {
      check_alloc(file_out=(char*)calloc((size_t)10,(size_t)1));
      strcpy(file_out,"stdin.mut");
    }
  }
  if (!stout)
    test_outfile(file_out);


  series=(double*)get_series(infile,&length,exclude_,column,verbosity_);
 */

  series=(double*)malloc(vetorPredicao.size()*sizeof(double));

  for(int i=0; i < vetorPredicao.size(); i++) {
	  series[i] = vetorPredicao[i];
  } 

  length = vetorPredicao.size();

  rescale_data(series,length,&min,&interval);

  check_alloc(h1=(long *)malloc(sizeof(long)*partitions));
  check_alloc(h11=(long *)malloc(sizeof(long)*partitions));
  check_alloc(h2=(long **)malloc(sizeof(long *)*partitions));

  for (i=0;i<partitions;i++) { 
    check_alloc(h2[i]=(long *)malloc(sizeof(long)*partitions));
  }
  
  check_alloc(array_=(long *)malloc(sizeof(long)*length));
 
  for (i=0;i<length;i++) {
    if (series[i] < 1.0) {
      array_[i]=(long)(series[i]*(double)partitions);
	}
    else {
      array_[i]=partitions-1;
	}
  }

  free(series);

  shannon=make_cond_entropy(0);

  if (corrlength >= length) {
    corrlength=length-1;
  }

   firstmin = make_cond_entropy(1);
   for (tau=2; tau<=corrlength; tau++) {
	  val = make_cond_entropy(tau);
      // printf("%ld %e\n",tau,val);
      if (val < firstmin) { 
		 delay_ = tau;
	     break;
	  }
	  else {
		  firstmin = val;
	  }
  }

  free(h1);
  free(h11);
  
  for (i=0; i<partitions; i++) { 
      free(h2[i]);
  }
  
  free(h2);
  
  free(array_);

  return delay_;
}

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
/*Author: Rainer Hegger. Last modified: Dec 10, 2005 */
/*Changes:
  12/10/05: It's multivariate now
  12/16/05: Scaled <eps> and sigma(eps)
*/



#define WID_STR "Determines the fraction of false nearest neighbors."



/*void show_options(char *progname)
{
  what_i_do(progname,WID_STR);
  fprintf(stderr," Usage: %s [options]\n",progname);
  fprintf(stderr," Options:\n");
  fprintf(stderr,"Everything not being a valid option will be interpreted"
          " as a possible"
          " datafile.\nIf no datafile is given stdin is read. Just - also"
          " means stdin\n");
  fprintf(stderr,"\t-l # of data [default: whole file]\n");
  fprintf(stderr,"\t-x # of lines to ignore [default: 0]\n");
  fprintf(stderr,"\t-c columns to read [default: 1]\n");
  fprintf(stderr,"\t-m min. test embedding dimension [default: %u]\n",minemb);
  fprintf(stderr,"\t-M # of components,max. emb. dim. [default: %u,%u]\n",
	  comp,maxemb);
  fprintf(stderr,"\t-d delay [default: 1]\n");
  fprintf(stderr,"\t-f escape factor [default: %.2lf]\n",rt);
  fprintf(stderr,"\t-t theiler window [default: 0]\n");
  fprintf(stderr,"\t-o output file [default: 'datafile'.fnn; without -o"
	  " stdout]\n");
  fprintf(stderr,"\t-V verbosity level [default: 3]\n\t\t"
          "0='only panic messages'\n\t\t"
          "1='+ input/output messages'\n\t\t"
          "2='+ information about the current state\n");
  fprintf(stderr,"\t-h show these options\n");
  exit(0);
}

void scan_options(int n,char **in)
{
  char *out;

  if ((out=check_option(in,n,'l','u')) != NULL)
    sscanf(out,"%lu",&length);
  if ((out=check_option(in,n,'x','u')) != NULL)
    sscanf(out,"%lu",&exclude_);
  if ((out=check_option(in,n,'c','s')) != NULL)
    column=out;
  if ((out=check_option(in,n,'m','u')) != NULL)
    sscanf(out,"%u",&minemb);
  if ((out=check_option(in,n,'M','2')) != NULL) {
    sscanf(out,"%u,%u",&comp,&maxemb);
    maxdim=comp*(maxemb+1);
    dimset=1;
  }
  if ((out=check_option(in,n,'d','u')) != NULL)
    sscanf(out,"%u",&delay);
  if ((out=check_option(in,n,'f','f')) != NULL)
    sscanf(out,"%lf",&rt);
  if ((out=check_option(in,n,'t','u')) != NULL)
    sscanf(out,"%lu",&theiler);
  if ((out=check_option(in,n,'V','u')) != NULL)
    sscanf(out,"%u",&verbosity_);
  if ((out=check_option(in,n,'o','o')) != NULL) {
    stdo_=0;
    if (strlen(out) > 0)
      outfile=out;
  }
}*/



/*
void mmb(unsigned int hdim,unsigned int hemb,double eps)
{
  unsigned long i = 0 ;
  long x = 0, y = 0;

  for (x=0;x<BOX;x++){
	  for (y=0;y<BOX;y++){
          box[x][y] = -1;
	  }
  }

  for (i=0;i<length-(maxemb+1)*delay;i++) {
    x=(long)(series[0][i]/eps)&ibox;
    y=(long)(series[hdim][i+hemb]/eps)&ibox;
    list[i]=box[x][y];
    box[x][y]=i;
  }
}

char find_nearest(long n,unsigned int dim,double eps)
{
  long x,y,x1,x2,y1,i,i1,ic,ie;
  long element,which= -1;
  double dx,maxdx,mindx=1.1,hfactor,factor;

  ic=vcomp[dim];
  ie=vemb[dim];
  x=(long)(series[0][n]/eps)&ibox;
  y=(long)(series[ic][n+ie]/eps)&ibox;
  
  for (x1=x-1;x1<=x+1;x1++) {
    x2=x1&ibox;
    for (y1=y-1;y1<=y+1;y1++) {
      element=box[x2][y1&ibox];
      while (element != -1) {
	if (labs(element-n) > theiler) {
	  maxdx=fabs(series[0][n]-series[0][element]);
	  for (i=1;i<=dim;i++) {
	    ic=vcomp[i];
	    i1=vemb[i];
	    dx=fabs(series[ic][n+i1]-series[ic][element+i1]);
	    if (dx > maxdx)
	      maxdx=dx;
	  }
	  if ((maxdx < mindx) && (maxdx > 0.0)) {
	    which=element;
	    mindx=maxdx;
	  }
	}
	element=list[element];
      }
    }
  }

  if ((which != -1) && (mindx <= eps) && (mindx <= varianz_/rt)) {
    aveps += mindx;
    vareps += mindx*mindx;
    factor=0.0;
    for (i=1;i<=comp;i++) {
      ic=vcomp[dim+i];
      ie=vemb[dim+i];
      hfactor=fabs(series[ic][n+ie]-series[ic][which+ie])/mindx;
      if (hfactor > factor) 
	factor=hfactor;
    }
    if (factor > rt)
      toolarge++;
    return 1;
  }
  return 0;
}

int main(int argc,char **argv)
{
  char stdi=0;
  FILE *file=NULL;
  double min,inter=0.0,ind_inter,epsilon,av,ind_var;
  char *nearest,alldone;
  long i;
  unsigned int dim,emb;
  unsigned long donesofar;

  if (scan_help(argc,argv))
    show_options(argv[0]);
  
  scan_options(argc,argv);
#ifndef OMIT_WHAT_I_DO
  if (verbosity_&VER_INPUT)
    what_i_do(argv[0],WID_STR);
#endif

  infile=search_datafile(argc,argv,NULL,verbosity_);
  if (infile == NULL)
    stdi=1;

  if (outfile == NULL) {
    if (!stdi) {
      check_alloc(outfile=(char*)calloc(strlen(infile)+5,(size_t)1));
      strcpy(outfile,infile);
      strcat(outfile,".fnn");
    }
    else {
      check_alloc(outfile=(char*)calloc((size_t)10,(size_t)1));
      strcpy(outfile,"stdin.fnn");
    }
  }
  if (!stdo_)
    test_outfile(outfile);

  if (column_ == NULL)
    series=(double**)get_multi_series(infile,&length,exclude_,&comp,"",dimset,
				      verbosity_);
  else
    series=(double**)get_multi_series(infile,&length,exclude_,&comp,column_,
				      dimset,verbosity_);

  for (i=0;i<comp;i++) {
    rescale_data(series[i],length,&min,&ind_inter);
    variance(series[i],length,&av,&ind_var);
    if (i == 0) {
      varianz_=ind_var;
      inter=ind_inter;
    }
    else {
      varianz_=(varianz_>ind_var)?ind_var:varianz_;
      inter=(inter<ind_inter)?ind_inter:inter;
    }
  }

  check_alloc(list=(long*)malloc(sizeof(long)*length));
  check_alloc(nearest=(char*)malloc(length));
  check_alloc(box=(long**)malloc(sizeof(long*)*BOX));
  for (i=0;i<BOX;i++)
    check_alloc(box[i]=(long*)malloc(sizeof(long)*BOX));

  if (!stdo_) {
    file=fopen(outfile,"w");
    if (verbosity_&VER_INPUT)
      fprintf(stderr,"Opened %s for writing\n",outfile);
  }
  else {
    if (verbosity_&VER_INPUT)
      fprintf(stderr,"Writing to stdout\n");
  }
  check_alloc(vcomp=(unsigned int*)malloc(sizeof(int)*(maxdim)));
  check_alloc(vemb=(unsigned int*)malloc(sizeof(int)*(maxdim)));
  for (i=0;i<maxdim;i++) {
    if (comp == 1) {
      vcomp[i]=0;
      vemb[i]=i;
    }
    else {
      vcomp[i]=i%comp;
      vemb[i]=(i/comp)*delay;
    }
  }
  for (emb=minemb;emb<=maxemb;emb++) {
    dim=emb*comp-1;
    epsilon=eps0;
    toolarge=0;
    alldone=0;
    donesofar=0;
    aveps=0.0;
    vareps=0.0;
    for (i=0;i<length;i++)
      nearest[i]=0;
    if (verbosity_&VER_USR1)
      fprintf(stderr,"Start for dimension=%u\n",dim+1);
    while (!alldone && (epsilon < 2.*varianz_/rt)) {
      alldone=1;
      mmb(vcomp[dim],vemb[dim],epsilon);
      for (i=0;i<length-maxemb*delay;i++)
	if (!nearest[i]) {
	  nearest[i]=find_nearest(i,dim,epsilon);
	  alldone &= nearest[i];
	  donesofar += (unsigned long)nearest[i];
	}
      if (verbosity_&VER_USR1)
	fprintf(stderr,"Found %lu up to epsilon=%e\n",donesofar,epsilon*inter);
      epsilon*=sqrt(2.0);
      if (!donesofar)
	eps0=epsilon;
    }
    if (donesofar == 0) {
      fprintf(stderr,"Not enough points found!\n");
      exit(FALSE_NEAREST_NOT_ENOUGH_POINTS);
    }
    aveps *= (1./(double)donesofar);
    vareps *= (1./(double)donesofar);
    if (stdo_) {
      fprintf(stdout,"%u %e %e %e\n",dim+1,(double)toolarge/(double)donesofar,
	      aveps*inter,sqrt(vareps)*inter);
      fflush(stdout);
    }
    else {
      fprintf(file,"%u %e %e %e\n",dim+1,(double)toolarge/(double)donesofar,
	      aveps*inter,sqrt(vareps)*inter);
      fflush(file);
    }
  }
  if (!stdo_)
    fclose(file);

  if (infile != NULL)
    free(infile);
  if (outfile != NULL)
    free(outfile);
  free(series);
  free(list);
  free(nearest);
  for (i=0;i<BOX;i++)
    free(box[i]);
  free(box);

  return 0;
}
*/