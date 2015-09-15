/*   PREDICTION ALGORITHM RBF
 *
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
 *
 * Author: Rainer Hegger. Last modified: Mar 11, 2002 
 * Author of the modifications : Marcos Paulo Moro
 * Changes: 31/03/2015
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "tisean/tsa.h"
#include <math.h>
#include <iostream>
#include <vector>
#include <time.h>

#include "Constants.h"

using namespace std;

extern void rescale_data(double *x,unsigned long l,double *min,double *interval);
extern void variance(double *s,unsigned long l,double *av,double *var);
extern void check_alloc(void *pnt);
extern void solvele(double **mat,double *vec,unsigned int n);

extern bool PRINTSCREEN;
extern int DIM;
extern int DELAY;
extern int CENTER;
extern int STEP;
extern unsigned int COLUMN;
extern unsigned long LENGTH;
extern long CLENGTH;
extern char MAKECAST;
extern int N;
extern char CAST;

#define WID_STR "Fits a RBF-model to the data"
//#define WID_STR "Fits a polynomial to the data"

char stdo=1;
char setdrift=1;

unsigned int verbosity=0xff;
unsigned long INSAMPLE=ULONG_MAX,exclude=0;

double *series,*coefs;
double varianz,interval,minimo;
double **center;

char sinsample=0;
unsigned int pars=1,hpar;
long *coding;
long maxencode;
double *results;
double std_dev;

vector<double> PREDICTION;

/*void show_options(char *progname)
{
  what_i_do(progname,WID_STR);
  fprintf(stderr," Usage: %s [options]\n",progname);
  fprintf(stderr," Options:\n");
  fprintf(stderr,"Everything not being a valid option will be interpreted"
          " as a possible"
          " datafile.\nIf no datafile is given stdin is read. Just - also"
          " means stdin\n");
  fprintf(stderr,"\t-l # of data to use [default: all from file]\n");
  fprintf(stderr,"\t-x # of lines to be ignored [default: 0]\n");
  fprintf(stderr,"\t-c column to read [default: %u]\n",COLUMN);
  fprintf(stderr,"\t-m embedding dimension [default: %d]\n",DIM);
  fprintf(stderr,"\t-d delay [default: %d]\n",DELAY);
  fprintf(stderr,"\t-p number of centers [default: %d]\n",CENTER);
  fprintf(stderr,"\t-X deactivate drift [default: activated]\n");
  fprintf(stderr,"\t-s steps to forecast [default: %d]\n",STEP);
  fprintf(stderr,"\t-n # of points for insample [default: # of data]\n");
  fprintf(stderr,"\t-L steps to cast [default: none]\n");
  fprintf(stderr,"\t-o output file name [default: 'datafile'.rbf]\n");
  fprintf(stderr,"\t-V verbosity level [default: 1]\n\t\t"
          "0='only panic messages'\n\t\t"
          "1='+ input/output messages'\n");
  fprintf(stderr,"\t-h show these options\n");
  exit(0);
}
*/
/*void scan_options(int n,char **in)
{
  char *out;

  if ((out=check_option(in,n,'l','u')) != NULL)
    sscanf(out,"%lu",&LENGTH);
  if ((out=check_option(in,n,'x','u')) != NULL)
    sscanf(out,"%lu",&exclude);
  if ((out=check_option(in,n,'c','u')) != NULL)
    sscanf(out,"%u",&COLUMN);
  if ((out=check_option(in,n,'m','u')) != NULL)
    sscanf(out,"%u",&DIM);
  if ((out=check_option(in,n,'d','u')) != NULL)
    sscanf(out,"%u",&DELAY);
  if ((out=check_option(in,n,'p','u')) != NULL)
    sscanf(out,"%u",&CENTER);
  if ((out=check_option(in,n,'X','n')) != NULL)
    setdrift=0;
  if ((out=check_option(in,n,'s','u')) != NULL)
    sscanf(out,"%u",&STEP);
  if ((out=check_option(in,n,'V','u')) != NULL)
    sscanf(out,"%u",&verbosity);
  if ((out=check_option(in,n,'n','u')) != NULL)
    sscanf(out,"%lu",&INSAMPLE);
  if ((out=check_option(in,n,'L','u')) != NULL) {
    MAKECAST=1;
    sscanf(out,"%lu",&CLENGTH);
  }
  if ((out=check_option(in,n,'o','o')) != NULL) {
    stdo=0;
    if (strlen(out) > 0)
      outfile=out;
  }
}
*/

double avdistanceRBF(void)
{
  int i,j,k;
  double dist=0.0;
  
  for (i=0;i<CENTER;i++) {
	  for (j=0;j<CENTER;j++) {
		  if (i != j) {
	         for (k=0;k<DIM;k++) {
	             dist += sqr(center[i][k]-center[j][k]);
			 }
		  }
	  }
  }
  return sqrt(dist/(CENTER-1)/CENTER/DIM);
}

double rbf(double *act,double *cen)
{
  static double denum = 0.00;
  double r = 0.00;
  int i = 0;

  denum = 2.0*varianz*varianz;

  for (i=0;i<DIM;i++){
      r += sqr(*(act-i*DELAY)-cen[i]);
  }
  return exp(-r/denum);
}

void driftRBF(void) 
{
  double *force = NULL;
  double h = 0.00;
  double h1 = 0.00;
  double step=1e-2;
  double step1 = 0.00;
  int i,j,k,l,d2=DIM;

  check_alloc(force=(double*)malloc(sizeof(double)*d2));

  for (l=0;l<20;l++) {
      for (i=0;i<CENTER;i++) {
          for (j=0;j<d2;j++) {
              force[j]=0.0;
              for (k=0;k<CENTER;k++) {
                  if (k != i) {
                     h=center[i][j]-center[k][j];
                     force[j] += h/sqr(h)/fabs(h);
                  }
              }
          }
          h=0.0;
          for (j=0;j<d2;j++) { 
              h += sqr(force[j]);
		  }
          step1 = step/sqrt(h);
          for (j=0;j<d2;j++) {
              h1 = step1*force[j];
              if (((center[i][j]+h1) > -0.1) && ((center[i][j]+h1) < 1.1)) {
                 center[i][j] += h1;
			  }
          }
      }
  }
  free(force);
}


void make_fitRBF(void)
{
  double **mat,*hcen;
  double h = 0.00;
  int i,j,n,nst;

  check_alloc(mat=(double**)malloc(sizeof(double*)*(CENTER+1)));

  for (i=0;i<=CENTER;i++){
	  check_alloc(mat[i]=(double*)malloc(sizeof(double)*(CENTER+1)));
  }  

  check_alloc(hcen=(double*)malloc(sizeof(double)*CENTER));

  for (i=0;i<=CENTER;i++) {
      coefs[i]=0.0;
      for (j=0;j<=CENTER;j++){
          mat[i][j]=0.00;
	  }
  }

  for (n=(DIM-1)*DELAY;n<INSAMPLE-STEP;n++) {
      nst=n+STEP;
      for (i=0;i<CENTER;i++){
          hcen[i]=rbf(&series[n],center[i]);
	  }
      coefs[0] += series[nst];
      mat[0][0] += 1.0;
      for (i=1;i<=CENTER;i++) { 
          mat[i][0] += hcen[i-1];
	  }
      for (i=1;i<=CENTER;i++) {
          coefs[i] += series[nst]*(h=hcen[i-1]);
          for (j=1;j<=i;j++) {
	          mat[i][j] += h*hcen[j-1];
		  }
      }
  }
  
  h=(double)(INSAMPLE-STEP-(DIM-1)*DELAY);
  
  for (i=0;i<=CENTER;i++) {
      coefs[i] /= h;
      for (j=0;j<=i;j++) {
          mat[i][j] /= h;
          mat[j][i]=mat[i][j];
      }
  }

  solvele(mat,coefs,(unsigned int)(CENTER+1));

  for (i=0;i<=CENTER;i++){
      free(mat[i]);
  }
  free(mat);
  free(hcen);
}

double forecast_errorRBF(unsigned long i0,unsigned long i1)
{
  int i,n;
  double h,error=0.0;

  for (n=i0+(DIM-1)*DELAY;n<i1-STEP;n++) {
    h=coefs[0];
    for (i=1;i<=CENTER;i++)
      h += coefs[i]*rbf(&series[n],center[i-1]);
    error += (series[n+STEP]-h)*(series[n+STEP]-h);
  }
  
  return sqrt(error/(i1-i0-STEP-(DIM-1)*DELAY));
}

void make_castRBF()
{
  double *cast,new_el = 0.00;
  int i,n,dim = 0;
  
  dim=(DIM-1)*DELAY;

  check_alloc(cast=(double*)malloc(sizeof(double)*(dim+1)));

  for (i=0;i<=dim;i++) { 
      cast[i]=series[LENGTH-1-dim+i];
  }

  for (n=0;n<CLENGTH;n++) {
      new_el=coefs[0];
      for (i=1;i<=CENTER;i++) {
          new_el += coefs[i]*rbf(&cast[dim],center[i-1]);
	  }
	  
	  if (PRINTSCREEN) {
	     cout << "# " << new_el*interval+minimo << endl;
	  }
	 
	  PREDICTION.push_back(new_el*interval+minimo);
      
	  for (i=0;i<dim;i++) {
		  cast[i]=cast[i+1];
	  }
      cast[dim]=new_el;
 }

  free(cast);
 }


vector<double> runRBF(vector<double> vetorPredicao)
{
  char stdi=0;
  int i=0;
  int j=0;
  int cstep=0;
  double sigma = 0.00;
  double av = 0.00;

    // initializing global variables
  varianz = 0;
  interval = 0;
  minimo = 0;
  INSAMPLE = ULONG_MAX;

  //erase prediction vetor 
  PREDICTION.erase(PREDICTION.begin(),PREDICTION.end());

  series=(double*)malloc(vetorPredicao.size()*sizeof(double));

  for (int i=0; i < vetorPredicao.size(); i++) {
	  series[i] = vetorPredicao[i];
  } 

  LENGTH = vetorPredicao.size();
  
  rescale_data(series,LENGTH,&minimo,&interval);

  variance(series,LENGTH,&av,&varianz);

  if (INSAMPLE > LENGTH) {
     INSAMPLE=LENGTH;
  }

  if (CENTER > LENGTH) { 
     CENTER = LENGTH;
  }

  if (MAKECAST) {
     STEP=1;
  }

  check_alloc(coefs=(double*)malloc(sizeof(double)*(CENTER+1)));
  check_alloc(center=(double**)malloc(sizeof(double*)*CENTER));
  for (i=0;i<CENTER;i++){
	  check_alloc(center[i]=(double*)malloc(sizeof(double)*DIM));
  }
  
  cstep=LENGTH-1-(DIM-1)*DELAY;
  
  for (i=0;i<CENTER;i++) {
	  for (j=0;j<DIM;j++) {
          center[i][j] = series[(DIM-1)*DELAY-j*DELAY+(i*cstep)/(CENTER-1)];
	  }
  }
  if (setdrift){
	 driftRBF();
  }
  
  varianz=avdistanceRBF();
  
  make_fitRBF();


  if (PRINTSCREEN) {
	 cout << "#Center points used: " << endl;
   	 for (i=0; i<CENTER; i++) {
         cout << "# ";
         for (j=0;j<DIM;j++) {
	         cout << center[i][j]*interval+minimo << " ";
	     }
         cout << endl;
     }
 	 cout << endl;
     cout << "#variance= " << varianz*interval << endl;
     cout << endl;
	 cout << "#Coefficients: " << endl;
     cout <<  coefs[0]*interval+minimo << endl;
     for (i=1; i<=CENTER; i++) {
         cout << coefs[i]*interval << endl;
	 }
  }

  av = sigma = 0.0;
  
  for (i=0;i<INSAMPLE;i++) {
      av += series[i];
      sigma += series[i]*series[i];
  }
  
  av /= INSAMPLE;
  
  sigma=sqrt(fabs(sigma/INSAMPLE-av*av));
  
  if (PRINTSCREEN) {
     cout << "#insample error= " << forecast_errorRBF(0LU,INSAMPLE)/sigma << endl;
  }

  if (INSAMPLE < LENGTH) {
     av=sigma=0.0;
     for (i=INSAMPLE;i<LENGTH;i++) {
         av += series[i];
         sigma += series[i]*series[i];
     }
     av /= (LENGTH-INSAMPLE);
     sigma=sqrt(fabs(sigma/(LENGTH-INSAMPLE)-av*av));
	 if (PRINTSCREEN) {
        cout << "#out of sample error= " << forecast_errorRBF(INSAMPLE,LENGTH)/sigma << endl;
	 }
  }

  if (MAKECAST) {
     make_castRBF();
  }

  free(series);

  free(coefs);

  for (i=0;i<CENTER;i++){
	  free(center[i]);
  }

  free(center);

  return(PREDICTION);
}

/*   PREDICTION ALGORITHM POLYNOM
 * 
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
 *
 *Author: Rainer Hegger
 *Changes:
 *  6/30/2006: Norm of the errors was wrong
 *
 * Author of the modifications : Marcos Paulo Moro
 * Changes: 31/03/2015

*/


/*void show_options(char *progname)
{
  what_i_do(progname,WID_STR);
  fprintf(stderr," Usage: %s [options]\n",progname);
  fprintf(stderr," Options:\n");
  fprintf(stderr,"Everything not being a valid option will be interpreted"
          " as a possible"
          " datafile.\nIf no datafile is given stdin is read. Just - also"
          " means stdin\n");
  fprintf(stderr,"\t-l # of data to use [default: whole file]\n");
  fprintf(stderr,"\t-x # of lines to be ignored [default: 0]\n");
  fprintf(stderr,"\t-c column to read [default: 1]\n");
  fprintf(stderr,"\t-m embedding dimension [default: 2]\n");
  fprintf(stderr,"\t-d delay [default: 1]\n");
  fprintf(stderr,"\t-p order of the polynomial [default: 2]\n");
  fprintf(stderr,"\t-n # of points for insample [default: # of data]\n");
  fprintf(stderr,"\t-L steps to cast [default: none]\n");
  fprintf(stderr,"\t-o output file name [default: 'datafile'.pol]\n");
  fprintf(stderr,"\t-V verbosity level [default: 1]\n\t\t"
          "0='only panic messages'\n\t\t"
          "1='+ input/output messages'\n");
  fprintf(stderr,"\t-h show these options\n");
  exit(0);
}

void scan_options(int n,char **in)
{
  char *out;

  if ((out=check_option(in,n,'l','u')) != NULL)
    sscanf(out,"%lu",&LENGTH);
  if ((out=check_option(in,n,'x','u')) != NULL)
    sscanf(out,"%lu",&exclude);
  if ((out=check_option(in,n,'c','u')) != NULL)
    sscanf(out,"%u",&COLUMN);
  if ((out=check_option(in,n,'m','u')) != NULL)
    sscanf(out,"%u",&DIM);
  if ((out=check_option(in,n,'d','u')) != NULL)
    sscanf(out,"%u",&DELAY);
  if ((out=check_option(in,n,'p','u')) != NULL)
    sscanf(out,"%u",&N);
  if ((out=check_option(in,n,'V','u')) != NULL)
    sscanf(out,"%u",&verbosity);
  if ((out=check_option(in,n,'n','u')) != NULL) {
    sscanf(out,"%lu",&INSAMPLE);
    sinsample=1;
  }
  if ((out=check_option(in,n,'L','u')) != NULL) {
    CAST=1;
    sscanf(out,"%lu",&CLENGTH);
  }
  if ((out=check_option(in,n,'o','o')) != NULL)
    if (strlen(out) > 0)
      outfile=out;
}*/


double polynom(int act,int dim,long cur,long fac)
{
  int j=0,n=0,hi=0;
  double ret=1.0;

  n=cur/fac;
  hi=act-(dim-1)*DELAY;
  for (j=1;j<=n;j++) {
      ret *= series[hi];
  }
  if (dim > 1) { 
     ret *= polynom(act,dim-1,cur-n*fac,fac/(N+1));
  }
  return ret;
}

int number_parsPOLYNOM(int ord,int start)
{
  int i=0,ret=0;

  if (ord == 1) {
	 for (i=start; i<=DIM; i++) {
         ret += 1;
	 }
  } 
  else {
     for (i=start; i<=DIM; i++) {
         ret += number_parsPOLYNOM(ord-1,i);
     }
  }

  return ret;
}

void make_codingPOLYNOM(int ord,int d,int fac,int cur)
{
  int j=0;

  if ( d == -1) { 
    coding[hpar++]=cur;
  }
  else {
    for (j=0;j<=ord;j++) {
        make_codingPOLYNOM(ord-j,d-1,fac*(N+1),cur+j*fac);
    }
  }
}

void make_fitPOLYNOM(void)
{
  int i=0,j=0,k=0;
  double **mat,*b;
  
  check_alloc(b=(double*)malloc(sizeof(double)*pars));

  check_alloc(mat=(double**)malloc(sizeof(double*)*pars));

  for (i=0; i<pars; i++) {
      check_alloc(mat[i]=(double*)malloc(sizeof(double)*pars));
  } 

  for (i=0; i<pars; i++) {
      b[i]=0.0;
      for (j=0;j<pars;j++) {
          mat[i][j]=0.0;
      }  
  }
  for (i=0; i<pars; i++) {
      for (j=i; j<pars; j++) {
          for (k=(DIM-1)*DELAY; k<INSAMPLE-1; k++) {
	          mat[i][j] += polynom(k,DIM,coding[i],maxencode) * polynom(k,DIM,coding[j],maxencode);
		  }
	  }
  }            
  for (i=0; i<pars; i++) {
      for (j=i; j<pars; j++) {
          mat[j][i] = (mat[i][j] /= (INSAMPLE-1-(DIM-1)*DELAY));
	  }
  }

  for (i=0; i<pars; i++) {
      for (j=(DIM-1)*DELAY; j<INSAMPLE-1; j++) {
          b[i] += series[j+1] * polynom(j,DIM,coding[i],maxencode);
	  }  
      b[i] /= (INSAMPLE-1-(DIM-1)*DELAY);
  }
  
  solvele(mat,b,pars);

  for (i=0; i<pars; i++) {
      results[i]=b[i];  
  }

  free(b);

  for (i=0; i<pars; i++){
        free(mat[i]);
    }

  
  free(mat);
}

void decodePOLYNOM(int *out,int dimen,long cur,long fac)
{
  int n=0;
  n=cur/fac;
  out[dimen]=n;
  if (dimen > 0) { 
    decodePOLYNOM(out,(dimen-1),(cur-((long)n*fac)),(fac/(N+1)));
  }

}

double make_errorPOLYNOM(unsigned long i0,unsigned long i1)
{
  int j=0,k=0;
  double h=0.0,err=0.0;
  
  err=0.0;
  for (j=i0+(DIM-1)*DELAY;j<(long)i1-1;j++) {
      h=0.0;
      for (k=0;k<pars;k++) { 
          h += results[k]*polynom(j,DIM,coding[k],maxencode);
	  }
      err += (series[j+1]-h)*(series[j+1]-h);
  }

  return err /= ((long)i1-(long)i0-(DIM-1)*DELAY);

}

void make_castPOLYNOM()
{
  int i=0,j=0,k=0,hi=0;
  double casted=0.0;
  
  for (i=0;i<=(DIM-1)*DELAY;i++) {
      series[i]=series[LENGTH-(DIM-1)*DELAY-1+i];
  }

  hi=(DIM-1)*DELAY;
 
  for (i=1;i<=CLENGTH;i++) { 
      casted=0.0;
      for (k=0;k<pars;k++) {
		  casted += results[k] * polynom((DIM - 1)*DELAY,DIM,coding[k],maxencode);
	  }
	  if (PRINTSCREEN) {
         cout << casted*std_dev << endl;
	  }
	  PREDICTION.push_back(casted*std_dev);

      for (j=0;j<(DIM-1)*DELAY;j++) {
		  series[j]=series[j+1];
	  }
	  series[hi]=casted;
  }
}

vector<double> runPolynom(vector<double> vetorPredicao)
{
  char stdi=0;
  int i=0,j=0,k=0;
  int *opar,sumpar=0;
  double in_error=0.0,out_error=0.0,av=0.0;
  
  // initializing global variables
  pars = 1;
  varianz = 0;
  interval = 0;
  minimo = 0;
  sinsample=0;
  hpar = 0;
  maxencode = 0;
  exclude = 0;
  std_dev = 0;

  //erase prediction vetor 
  PREDICTION.erase(PREDICTION.begin(),PREDICTION.end());

  series=(double*)malloc(vetorPredicao.size()*sizeof(double));

  for(int i=0; i < vetorPredicao.size(); i++) {
	  series[i] = vetorPredicao[i];
  } 

  LENGTH = vetorPredicao.size();

  variance(series,LENGTH,&av,&std_dev);

  for (i=0;i<LENGTH;i++) {
    series[i] /= std_dev;
  }

  if (!sinsample || (INSAMPLE > LENGTH)) {
     INSAMPLE=LENGTH;
  }

  maxencode=1;

  for (i=1;i<DIM;i++) {
      maxencode *= (N+1);
  }

  for (i=1;i<=N;i++) {
      pars += number_parsPOLYNOM(i,1);
  }

  if (PRINTSCREEN) {
     cout << "#number of free parameters " << pars << endl << endl;
  }

  check_alloc(coding=(long*)malloc(sizeof(long)*pars));
  
  hpar=0;

  make_codingPOLYNOM(N,DIM-1,1,0);
  
  check_alloc(results=(double*)malloc(sizeof(double)*pars));
  
  make_fitPOLYNOM();
  
  check_alloc(opar=(int*)malloc(sizeof(int)*DIM));
  
  if (PRINTSCREEN) {
     cout << "#used norm for the fit " << std_dev << endl;
  }

  for (j=0; j<pars; j++) {
      decodePOLYNOM(opar,DIM-1,coding[j],maxencode);
	  if (PRINTSCREEN) {
         cout << "#";
	  }
      sumpar=0;
      for (k=0; k<DIM; k++) {
          sumpar += opar[k];
		  if (PRINTSCREEN) {
			 cout << opar[k] << " ";
		  }
      }
	  if (PRINTSCREEN) {
		 cout << results[j]/pow(std_dev,(double)(sumpar-1)) << endl;
	  }
  }
  
  in_error=make_errorPOLYNOM((unsigned long)0,INSAMPLE);
  
  if (PRINTSCREEN) {
     cout << endl;
     cout << "#average insample error " << sqrt(in_error) << endl; 
  }

  if (INSAMPLE < LENGTH) {
     out_error=make_errorPOLYNOM(INSAMPLE,LENGTH);
     cout << "#average out of sample error " << sqrt(out_error) << endl;
  }

  if (CAST) {
    make_castPOLYNOM();
  }

  free(opar);
  free(series);
  free(coefs);
  free(center);
  free(coding);
  free(results);

  return(PREDICTION);
}
