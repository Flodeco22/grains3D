/*
Copyright ESIEE (2009) 

m.couprie@esiee.fr

This software is an image processing library whose purpose is to be
used primarily for research and teaching.

This software is governed by the CeCILL  license under French law and
abiding by the rules of distribution of free software. You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/
/* 
Librairie mcsplines : 

fonctions pour les splines cubiques naturelles

Michel Couprie, S�bastien Couprie, juin 2003

scn_solvespline(
  double *x, double *y, int32_t n, 
  double *Z0, double *Z1, double *Z2, double *Z3);
scn_solvespline_noalloc(
  double *x, double *y, int32_t n, 
  double *Z0, double *Z1, double *Z2, double *Z3,
  double *M, double *P, double *z, double *A, double*B);
scn_solvespline1(
  double *y, int32_t n, 
  double *Z0, double *Z1, double *Z2, double *Z3);
scn_solveclosedspline(
  double *x, double *y, int32_t n, 
  double *Z0, double *Z1, double *Z2, double *Z3);
scn_samplespline(
  double *x, double *y, int32_t n, 
  int32_t m, double *X, double *Y);
scn_samplespline3d(
  double *x, double *y, double *z, int32_t n, 
  int32_t m, double *X, double *Y, double *Z);
scn_curvatures(
  double *x, double *y, int32_t n, 
  int32_t m, double *sk, double *rhok);
scn_curvatures3d(
  double *x, double *y, double *z, int32_t n, 
  int32_t m, double *sk, double *rhok);
scn_approxcurve(
  int32_t *X, int32_t *Y, int32_t N, 
  double deltamax, int32_t *Z, int32_t *n, 
  double *C0, double *C1, double *C2, double *C3,
  double *D0, double *D1, double *D2, double *D3);
scn_approxcurve3d(
  int32_t *X, int32_t *Y, int32_t *Z, int32_t N, double deltamax, 
  int32_t *C, int32_t *n, 
  double *C0, double *C1, double *C2, double *C3,
  double *D0, double *D1, double *D2, double *D3,
  double *E0, double *E1, double *E2, double *E3);
scn_lengthspline(
  double *X0, double *X1, double *X2, double *X3, 
  double *Y0, double *Y1, double *Y2, double *Y3, 
  int32_t nctrl);
scn_lengthspline3d(
  double *X0, double *X1, double *X2, double *X3, 
  double *Y0, double *Y1, double *Y2, double *Y3, 
  double *Z0, double *Z1, double *Z2, double *Z3,
  int32_t nctrl);

*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <mcutil.h>
#include <mclin.h>
#include <ldraw.h>
#include <mcsplines.h>

//#define TEST
//#define DEBUG
//#define DEBUG_scn_tangents3d
//#define PARANO

#define SCN_EPSILON 0.000001
#define SCN_EPSILON2 1E-30

#ifndef HUGE_VAL
#define HUGE_VAL 1E+100
#endif

//static double cube(double x) { return x * x * x; }
static double dist3(double x1, double y1, double z1, double x2, double y2, double z2)
{
  return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
}

// =================================================
// SPLINES CUBIQUES NATURELLES
// =================================================

/* ==================================== */
int32_t scn_constant(double *x, int32_t n)
/* ==================================== */
{
  int32_t i; 
  for (i = 1; i < n; i++) 
    if (mcabs((x[i]-x[0])) > SCN_EPSILON) return 0;
  return 1;
} // scn_constant()

/* ==================================== */
int32_t scn_solvespline(double *x, double *y, int32_t n, 
                         double *Z0, double *Z1, double *Z2, double *Z3)
/* ==================================== */
/*! \fn double * scn_solvespline(double *x, double *y, int32_t n, double *A, double *B)
    \param x (entr�e) : tableau des abcisses
    \param y (entr�e) : tableau des ordonn�es
    \param n (entr�e) : nombre de points
    \param Z0 (sortie) : tableau des coef. spline de degr� 0 (taille n-1) 
    \param Z1 (sortie) : tableau des coef. spline de degr� 1 (taille n-1) 
    \param Z2 (sortie) : tableau des coef. spline de degr� 2 (taille n-1) 
    \param Z3 (sortie) : tableau des coef. spline de degr� 3 (taille n-1) 
    \brief calcule la spline cubique naturelle passant par les n points de contr�le donn�s par x, y
    \warning la m�moire pour stocker les r�sultats Z0, Z1, Z2, Z3 doit avoir �t� allou�e
*/
#undef F_NAME
#define F_NAME "scn_solvespline"
{
  int32_t i, j;
  double *M, *P, *z, *A, *B;

  M = (double *)calloc(1,(n-2) * (n-2) * sizeof(double)); assert(M != NULL);
  P = (double *)calloc(1,(n-2) * sizeof(double)); assert(P != NULL);
  z = (double *)calloc(1,(n-2) * sizeof(double)); assert(z != NULL);
  A = (double *)calloc(1,(n-1) * sizeof(double)); assert(A != NULL);
  B = (double *)calloc(1,(n-1) * sizeof(double)); assert(B != NULL);

  for (i = 0; i < n-2; i++)
    for (j = 0; j < n-2; j++)
    {
      if (i == j)
        M[i*(n-2) + j] = (x[i+2] - x[i]) / 3;
      else if (j == (i+1))
        M[i*(n-2) + j] = (x[j+1] - x[j]) / 6;
      else if (j == (i-1))
        M[i*(n-2) + j] = (x[i+1] - x[i]) / 6;
      else
        M[i*(n-2) + j] = 0.0;
    }

  for (j = 0; j < n-2; j++)
    P[j] = ((y[j+2] - y[j+1]) / (x[j+2] - x[j+1])) - 
           ((y[j+1] - y[j]) / (x[j+1] - x[j]));
  if (!lin_solvetridiag(M, P, z, n-2))
  {
    printf("%s: lin_solvetridiag failed\n", F_NAME);
    return 0;
  }

  A[0] = y[0];
  B[0] = ((y[1] - y[0]) / (x[1] - x[0])) - (((x[1] - x[0]) * z[0]) / 6.0) ;
  for (j = 1; j < n-2; j++)
  {
    A[j] = y[j] - ((z[j-1] * (x[j+1]-x[j]) * (x[j+1]-x[j])) / 6.0);
    B[j] = ((y[j+1] - y[j]) / (x[j+1] - x[j])) - 
           (((x[j+1] - x[j]) * (z[j] - z[j-1])) / 6.0) ;
  }
  // j = n-2
  A[j] = y[j] - ((z[j-1] * (x[j+1]-x[j]) * (x[j+1]-x[j])) / 6.0);
  B[j] = ((y[j+1] - y[j]) / (x[j+1] - x[j])) - 
          (((x[j+1] - x[j]) * (0 - z[j-1])) / 6.0) ;

  Z3[0] = z[0] / (6 * (x[1] - x[0]));
  Z2[0] = (-z[0]*x[0]) / (2 * (x[1] - x[0]));
  Z1[0] = ((z[0]*x[0]*x[0]) / (2 * (x[1] - x[0]))) + B[0];
  Z0[0] = ((-z[0]*x[0]*x[0]*x[0]) / (6 * (x[1] - x[0]))) - B[0]*x[0] + A[0];
  for (j = 1; j < n-2; j++)
  {
    Z3[j] = (z[j] - z[j-1]) / (6 * (x[j+1] - x[j]));
    Z2[j] = (-z[j]*x[j] + z[j-1]*x[j+1]) / (2 * (x[j+1] - x[j]));
    Z1[j] = ((z[j]*x[j]*x[j] - z[j-1]*x[j+1]*x[j+1]) / (2 * (x[j+1] - x[j]))) + B[j];
    Z0[j] = ((-z[j]*x[j]*x[j]*x[j] + z[j-1]*x[j+1]*x[j+1]*x[j+1]) / 
             (6 * (x[j+1] - x[j]))) - B[j]*x[j] + A[j];
  }  
  // j = n-2
  Z3[j] = (0 - z[j-1]) / (6 * (x[j+1] - x[j]));
  Z2[j] = (z[j-1]*x[j+1]) / (2 * (x[j+1] - x[j]));
  Z1[j] = ((0 - z[j-1]*x[j+1]*x[j+1]) / (2 * (x[j+1] - x[j]))) + B[j];
  Z0[j] = ((z[j-1]*x[j+1]*x[j+1]*x[j+1]) / 
	   (6 * (x[j+1] - x[j]))) - B[j]*x[j] + A[j];

  free(M); 
  free(P); 
  free(z); 
  free(A); 
  free(B);
  return 1;
} // scn_solvespline()

/* ==================================== */
int32_t scn_solvespline_noalloc(double *x, double *y, int32_t n, 
			     double *Z0, double *Z1, double *Z2, double *Z3,
			     double *M, double *P, double *z, double *A, double*B)
/* ==================================== */
/*! \fn double * scn_solvespline_noalloc(double *x, double *y, int32_t n, double *A, double *B)
    \param x (entr�e) : tableau des abcisses
    \param y (entr�e) : tableau des ordonn�es
    \param n (entr�e) : nombre de points
    \param Z0 (sortie) : tableau des coef. spline de degr� 0 (taille n-1) 
    \param Z1 (sortie) : tableau des coef. spline de degr� 1 (taille n-1) 
    \param Z2 (sortie) : tableau des coef. spline de degr� 2 (taille n-1) 
    \param Z3 (sortie) : tableau des coef. spline de degr� 3 (taille n-1) 
    \param M (sortie) : tableau provisoire de taille (n-2)*(n-2)
    \param P (sortie) : tableau provisoire de taille (n-2)
    \param z (sortie) : tableau provisoire de taille (n-2)
    \param A (sortie) : tableau provisoire de taille (n-1)
    \param B (sortie) : tableau provisoire de taille (n-1)
    \brief calcule la spline cubique naturelle passant par les n points de contr�le donn�s par x, y
    \warning la m�moire pour stocker les r�sultats Z0, Z1, Z2, Z3 doit avoir �t� allou�e
*/
#undef F_NAME
#define F_NAME "scn_solvespline_noalloc"
{
  int32_t i, j;

  memset(M, 0, (n-2) * (n-2) * sizeof(double));
  memset(P, 0, (n-2) * sizeof(double));
  memset(z, 0, (n-2) * sizeof(double));
  memset(A, 0, (n-1) * sizeof(double));
  memset(B, 0, (n-1) * sizeof(double));

  for (i = 0; i < n-2; i++)
    for (j = 0; j < n-2; j++)
    {
      if (i == j)
        M[i*(n-2) + j] = (x[i+2] - x[i]) / 3;
      else if (j == (i+1))
        M[i*(n-2) + j] = (x[j+1] - x[j]) / 6;
      else if (j == (i-1))
        M[i*(n-2) + j] = (x[i+1] - x[i]) / 6;
      else
        M[i*(n-2) + j] = 0.0;
    }

  for (j = 0; j < n-2; j++)
    P[j] = ((y[j+2] - y[j+1]) / (x[j+2] - x[j+1])) - 
           ((y[j+1] - y[j]) / (x[j+1] - x[j]));
  if (!lin_solvetridiag(M, P, z, n-2))
  {
    printf("%s: lin_solvetridiag failed\n", F_NAME);
    return 0;
  }

  A[0] = y[0];
  B[0] = ((y[1] - y[0]) / (x[1] - x[0])) - (((x[1] - x[0]) * z[0]) / 6.0) ;
  for (j = 1; j < n-2; j++)
  {
    A[j] = y[j] - ((z[j-1] * (x[j+1]-x[j]) * (x[j+1]-x[j])) / 6.0);
    B[j] = ((y[j+1] - y[j]) / (x[j+1] - x[j])) - 
           (((x[j+1] - x[j]) * (z[j] - z[j-1])) / 6.0) ;
  }
  // j = n-2
  A[j] = y[j] - ((z[j-1] * (x[j+1]-x[j]) * (x[j+1]-x[j])) / 6.0);
  B[j] = ((y[j+1] - y[j]) / (x[j+1] - x[j])) - 
          (((x[j+1] - x[j]) * (0 - z[j-1])) / 6.0) ;

  Z3[0] = z[0] / (6 * (x[1] - x[0]));
  Z2[0] = (-z[0]*x[0]) / (2 * (x[1] - x[0]));
  Z1[0] = ((z[0]*x[0]*x[0]) / (2 * (x[1] - x[0]))) + B[0];
  Z0[0] = ((-z[0]*x[0]*x[0]*x[0]) / (6 * (x[1] - x[0]))) - B[0]*x[0] + A[0];
  for (j = 1; j < n-2; j++)
  {
    Z3[j] = (z[j] - z[j-1]) / (6 * (x[j+1] - x[j]));
    Z2[j] = (-z[j]*x[j] + z[j-1]*x[j+1]) / (2 * (x[j+1] - x[j]));
    Z1[j] = ((z[j]*x[j]*x[j] - z[j-1]*x[j+1]*x[j+1]) / (2 * (x[j+1] - x[j]))) + B[j];
    Z0[j] = ((-z[j]*x[j]*x[j]*x[j] + z[j-1]*x[j+1]*x[j+1]*x[j+1]) / 
             (6 * (x[j+1] - x[j]))) - B[j]*x[j] + A[j];
  }  
  Z3[j] = (0 - z[j-1]) / (6 * (x[j+1] - x[j]));
  Z2[j] = (z[j-1]*x[j+1]) / (2 * (x[j+1] - x[j]));
  Z1[j] = ((0 - z[j-1]*x[j+1]*x[j+1]) / (2 * (x[j+1] - x[j]))) + B[j];
  Z0[j] = ((z[j-1]*x[j+1]*x[j+1]*x[j+1]) / 
	   (6 * (x[j+1] - x[j]))) - B[j]*x[j] + A[j];
  return 1;
} // scn_solvespline_noalloc()

/* ==================================== */
int32_t scn_solvespline1(double *y, int32_t n, 
                      double *Z0, double *Z1, double *Z2, double *Z3)
/* ==================================== */
/*! \fn double * scn_solvespline1(double *y, int32_t n, double *A, double *B)
    \param y (entr�e) : tableau des ordonn�es
    \param n (entr�e) : nombre de points
    \param Z0 (sortie) : tableau des coef. spline de degr� 0 (taille n-1) 
    \param Z1 (sortie) : tableau des coef. spline de degr� 1 (taille n-1) 
    \param Z2 (sortie) : tableau des coef. spline de degr� 2 (taille n-1) 
    \param Z3 (sortie) : tableau des coef. spline de degr� 3 (taille n-1) 
    \brief calcule la spline cubique naturelle passant par les n points de contr�le donn�s par y[i], 0 <= i < n
    \warning la m�moire pour stocker les r�sultats Z0, Z1, Z2, Z3 doit avoir �t� allou�e
*/
#undef F_NAME
#define F_NAME "scn_solvespline1"
{
  int32_t i, j;
  double *M, *P, *z, *A, *B;
  M = (double *)calloc(1,(n-2) * (n-2) * sizeof(double)); assert(M != NULL);
  P = (double *)calloc(1,(n-2) * sizeof(double)); assert(P != NULL);
  z = (double *)calloc(1,(n-2) * sizeof(double)); assert(z != NULL);
  A = (double *)calloc(1,(n-1) * sizeof(double)); assert(A != NULL);
  B = (double *)calloc(1,(n-1) * sizeof(double)); assert(B != NULL);

  for (i = 0; i < n-2; i++)
    for (j = 0; j < n-2; j++)
    {
      if (i == j)
        M[i*(n-2) + j] = 2.0 / 3;
      else if (j == (i+1))
        M[i*(n-2) + j] = 1.0 / 6;
      else if (j == (i-1))
        M[i*(n-2) + j] = 1.0 / 6;
      else
        M[i*(n-2) + j] = 0.0;
    }

  for (j = 0; j < n-2; j++)
    P[j] = ((y[j+2] - y[j+1]) / 2) - ((y[j+1] - y[j]) / 2);

  if (!lin_solvetridiag(M, P, z, n-2))
  {
    printf("%s: lin_solvetridiag failed\n", F_NAME);
    return 0;
  }

  A[0] = y[0];
  B[0] = (y[1] - y[0]) - (z[0] / 6.0);
  for (j = 1; j < n-2; j++)
  {
    A[j] = y[j] - (z[j-1] / 6.0);
    B[j] = (y[j+1] - y[j]) - ((z[j] - z[j-1]) / 6.0);
  }
  // j = n-2
  A[j] = y[j] - (z[j-1] / 6.0);
  B[j] = (y[j+1] - y[j]) - ((0 - z[j-1]) / 6.0);

  Z3[0] = z[0] / 6;
  Z2[0] = 0.0;
  Z1[0] = B[0];
  Z0[0] = A[0];
  for (j = 1; j < n-2; j++)
  {
    Z3[j] = (z[j] - z[j-1]) / 6;
    Z2[j] = (-z[j]*j + z[j-1]*(j+1)) / 2;
    Z1[j] = ((z[j]*j*j - z[j-1]*(j+1)*(j+1)) / 2) + B[j];
    Z0[j] = ((-z[j]*j*j*j + z[j-1]*(j+1)*(j+1)*(j+1)) / 6) - B[j]*j + A[j];
  }  
  // j = n-2
  Z3[j] = (0 - z[j-1]) / 6;
  Z2[j] = (z[j-1]*(j+1)) / 2;
  Z1[j] = ((0 - z[j-1]*(j+1)*(j+1)) / 2) + B[j];
  Z0[j] = ((z[j-1]*(j+1)*(j+1)*(j+1)) / 6) - B[j]*j + A[j];

  free(M); 
  free(P); 
  free(z); 
  free(A); 
  free(B);
  return 1;
} // scn_solvespline1()

/* ==================================== */
int32_t scn_solveclosedspline(double *x, double *y, int32_t n, 
                           double *Z0, double *Z1, double *Z2, double *Z3)
/* ==================================== */
/*! \fn double * scn_solvespline(double *x, double *y, int32_t n, double *A, double *B)
    \param x (entr�e) : tableau des abcisses
    \param y (entr�e) : tableau des ordonn�es
    \param n (entr�e) : nombre de points
    \param Z0 (sortie) : tableau des coef. spline de degr� 0 (taille n-1) 
    \param Z1 (sortie) : tableau des coef. spline de degr� 1 (taille n-1) 
    \param Z2 (sortie) : tableau des coef. spline de degr� 2 (taille n-1) 
    \param Z3 (sortie) : tableau des coef. spline de degr� 3 (taille n-1) 
    \brief calcule la spline cubique naturelle passant par les n points de contr�le donn�s par x, y
    \warning la m�moire pour stocker les r�sultats A,B doit avoir �t� allou�e
*/
#undef F_NAME
#define F_NAME "scn_solveclosedspline"
{
  int32_t i, nn;
  double *xx, *yy, *ZZ0, *ZZ1, *ZZ2, *ZZ3;

  if (n < 3)
  {
    fprintf(stderr, "%s: not enough points\n", F_NAME);
    return 0;
  }

  if (y[0] != y[n-1])
  {
    fprintf(stderr, "%s: not a closed spline\n", F_NAME);
    return 0;
  }

  nn = n + 4;
  xx = (double *)calloc(nn, sizeof(double)); assert(xx != NULL);
  yy = (double *)calloc(nn, sizeof(double)); assert(yy != NULL);
  ZZ0 = (double *)calloc(1,(nn-1) * sizeof(double)); assert(ZZ0 != NULL);
  ZZ1 = (double *)calloc(1,(nn-1) * sizeof(double)); assert(ZZ1 != NULL);
  ZZ2 = (double *)calloc(1,(nn-1) * sizeof(double)); assert(ZZ2 != NULL);
  ZZ3 = (double *)calloc(1,(nn-1) * sizeof(double)); assert(ZZ3 != NULL);

  xx[0] = x[0] - x[n-1] + x[n-3];
  yy[0] = y[n-3];
  xx[1] = x[0] - x[n-1] + x[n-2];
  yy[1] = y[n-2];
  for (i = 0; i < n; i++) { xx[i+2] = x[i]; yy[i+2] = y[i]; }
  xx[n+2] = x[n-1] + x[1] - x[0];
  yy[n+2] = y[1];
  xx[n+3] = x[n-1] + x[2] - x[0];
  yy[n+3] = y[2];

  if (scn_solvespline(xx, yy, nn, ZZ0, ZZ1, ZZ2, ZZ3) == 0)
    return 0;

  for (i = 0; i < n-1; i++)
  {
    Z0[i] = ZZ0[i+2];
    Z1[i] = ZZ1[i+2];
    Z2[i] = ZZ2[i+2];
    Z3[i] = ZZ3[i+2];
  }

  free(xx); 
  free(yy); 
  free(ZZ0); 
  free(ZZ1); 
  free(ZZ2); 
  free(ZZ3);
  return 1;
} // scn_solveclosedspline()

// =================================================
// CALCUL D'ABCISSE CURVILIGNE
// =================================================

double evalpoly(int32_t n, double *f, double x)
{
     double sum = 0;
     int32_t i;    
     for(i=n-1;i>0;i--) sum=x*(f[i]+sum);
     return sum+f[0];
}

double integrale(double f[3], double a, double b, int32_t p)
{
     int32_t i;
     double fti,fti1;
     double sum=0.0;
     double pas=(b-a)/p;
     double ti=a, ti1=a+pas;
     double tmp;
     for(i=0;i<p;i++)
     {
       tmp = evalpoly(3, (double *)f,ti);
       fti=sqrt(1+tmp*tmp);
       tmp = evalpoly(3, (double *)f,ti1);
       fti1=sqrt(1+tmp*tmp);
       sum=sum+(fti1-fti)*(pas/2)+fti*pas;
       ti=ti1;
       ti1=ti1+pas;
     }
     return sum;
} // integrale()

double integrale2(double f[3], double g[3], double a, double b, int32_t p)
{
     int32_t i;
     double fti,fti1;
     double sum=0.0;
     double pas=(b-a)/p;
     double ti=a, ti1=a+pas;
     double tmp1, tmp2;
     for(i=0;i<p;i++)
     {
       tmp1 = evalpoly(3, (double *)f,ti);
       tmp2 = evalpoly(3, (double *)g,ti);
       fti=sqrt(tmp1*tmp1+tmp2*tmp2);
       tmp1 = evalpoly(3, (double *)f,ti1);
       tmp2 = evalpoly(3, (double *)g,ti1);
       fti1=sqrt(tmp1*tmp1+tmp2*tmp2);
       sum=sum+(fti1-fti)*(pas/2)+fti*pas;
       ti=ti1;
       ti1=ti1+pas;
     }
     return sum;
} // integrale2()

double integrale3(double f[3], double g[3], double h[3], double a, double b, int32_t p)
{
     int32_t i;
     double fti, fti1;
     double sum = 0.0;
     double pas = (b-a)/p;
     double ti = a, ti1 = a+pas;
     double tmp1, tmp2, tmp3;
     for(i = 0; i < p; i++)
     {
       tmp1 = evalpoly(3, (double *)f, ti);
       tmp2 = evalpoly(3, (double *)g, ti);
       tmp3 = evalpoly(3, (double *)h, ti);
       fti = sqrt(tmp1*tmp1 + tmp2*tmp2 + tmp3*tmp3);
       tmp1 = evalpoly(3, (double *)f, ti1);
       tmp2 = evalpoly(3, (double *)g, ti1);
       tmp3 = evalpoly(3, (double *)h, ti1);
       fti1 = sqrt(tmp1*tmp1 + tmp2*tmp2 + tmp3*tmp3);
       sum = sum + (fti1-fti) * (pas/2) + fti*pas;
       ti = ti1;
       ti1 = ti1 + pas;
     }
     return sum;
} // integrale3()

double dicho(double f[3], double a, double b, double r, int32_t p, double eps)
/*
recherche par dichotomie d'une valeur t dans [a,b] telle que 
integrale(f, a, t, p) = r avec une erreur max de eps   
retourne t
*/
{
  double tmp, u, asav = a;
  while (b-a>eps)
  {
    u=(a+b)/2;
    tmp = integrale(f, asav, u, p);
    if (tmp == r) return u;
    else if (tmp > r) b = u;
	 else a = u;
  }
  return u;                    
} // dicho()

double dicho2(double f[3], double g[3], double a, double b, double r, int32_t p, double eps)
/*
recherche par dichotomie d'une valeur t dans [a,b] telle que 
integrale2(f, g, a, t, p) = r avec une erreur max de eps   
retourne t
*/
{
  double tmp, u, asav = a;
  while (b-a > eps)
  {
    u = (a + b) / 2;
    tmp = integrale2(f, g, asav, u, p);
    if (tmp == r) return u;
    else if (tmp > r) b = u;
	 else a = u;
  }
  return u;                    
} // dicho2()

double dicho3(double f[3], double g[3], double h[3], double a, double b, double r, int32_t p, double eps)
/*
recherche par dichotomie d'une valeur t dans [a,b] telle que 
integrale3(f, g, h, a, t, p) = r avec une erreur max de eps
retourne t
*/
{
  double tmp, u, asav = a;
  while (b-a > eps)
  {
    u = (a + b) / 2;
    tmp = integrale3(f, g, h, asav, u, p);
    if (tmp == r) return u;
    else if (tmp > r) b = u;
	 else a = u;
  }
  return u;
} // dicho3()

/* ==================================== */
int32_t scn_samplespline(double *x, double *y, int32_t n, int32_t m, double *X, double *Y)
/* ==================================== */
/*! \fn double * scn_samplespline(double *x, double *y, int32_t n, int32_t m, double *X, double *Y)
    \param x (entr�e) : tableau des abcisses des points de contr�le (taille n)
    \param y (entr�e) : tableau des ordonn�es des points de contr�le (taille n)
    \param n (entr�e) : nombre de points de contr�le
    \param m (entr�e) : nombre des �chantillons
    \param X (sortie) : tableau des absisses des �chantillons (taille m)
    \param Y (sortie) : tableau des ordonn�es des �chantillons (taille m)
    \brief �chantilonne une spline selon son abcisse curviligne
    \warning la m�moire pour stocker les r�sultats X et Y doit avoir �t� allou�e
*/
#undef F_NAME
#define F_NAME "scn_samplespline"
{
  double *X0, *X1, *X2, *X3;
  double *Y0, *Y1, *Y2, *Y3;
  double *t;
  double tk, tmp, L = 0;
  double sum, sumsav;
  int32_t p = m*10;
  int32_t i, j, k;
  double f[4], g[4];

  if (n < 1)
  {
    fprintf(stderr, "%s: not enough points\n", F_NAME);
    return 0;
  }

  if (m < 2)
  {
    fprintf(stderr, "%s: not enough samples\n", F_NAME);
    return 0;
  }

  if (n == 1)
  {
    for(k = 0; k < m-1; k++) { X[k] = x[0]; Y[k] = y[0]; }
    return 1;
  }

  if (n == 2)
  {
    X[0] = x[0]; Y[0] = y[0];
    X[m-1] = x[n-1]; Y[m-1] = y[n-1];
    for(k = 1; k < m-1; k++)
    {
      X[k] = x[0] + k * (x[n-1] - x[0]) / (m-1); 
      Y[k] = y[0] + k * (y[n-1] - y[0]) / (m-1); 
    }
    return 1;
  }
  
  t = (double *)calloc(1,n * sizeof(double)); assert(t != NULL);
  X0 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X0 != NULL);
  X1 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X1 != NULL);
  X2 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X2 != NULL);
  X3 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X3 != NULL);
  Y0 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y0 != NULL);
  Y1 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y1 != NULL);
  Y2 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y2 != NULL);
  Y3 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y3 != NULL);
  for(i = 0; i < n; i++) t[i] = (double)i;


  if ((x[0] != x[n-1]) || (y[0] != y[n-1]))
  {
    if (scn_solvespline(t, x, n, X0, X1, X2, X3) == 0) return 0;
    if (scn_solvespline(t, y, n, Y0, Y1, Y2, Y3) == 0) return 0;
  }
  else
  {
    if (scn_solveclosedspline(t, x, n, X0, X1, X2, X3) == 0) return 0;
    if (scn_solveclosedspline(t, y, n, Y0, Y1, Y2, Y3) == 0) return 0;
  }

  for(i = 0; i < n-1; i++)
  {
    f[0] = X1[i]; f[1] = 2*X2[i]; f[2] = 3*X3[i];
    g[0] = Y1[i]; g[1] = 2*Y2[i]; g[2] = 3*Y3[i];
    L = L + integrale2(f, g, t[i], t[i+1], p);
  }

  if (L < SCN_EPSILON)
  {
    for (k = 0; k < m; k++) 
    { 
      X[k] = x[0]; Y[k] = y[0];
    }
    goto scn_samplespline_end;
  }

  X[0] = x[0]; Y[0] = y[0];
  X[m-1] = x[n-1]; Y[m-1] = y[n-1];
  for(k = 1; k < m-1; k++)
  {
    tmp = k * L/(m-1);
    j = 0;
    sumsav = sum = 0; 
    while (sum < tmp)
    {
      sumsav = sum;
      f[0] = X1[j]; f[1] = 2*X2[j]; f[2] = 3*X3[j];
      g[0] = Y1[j]; g[1] = 2*Y2[j]; g[2] = 3*Y3[j];
      sum = sum + integrale2(f, g, t[j], t[j+1], p);
      j++;
    }
    j--;
    f[0] = X1[j]; f[1] = 2*X2[j]; f[2] = 3*X3[j];
    g[0] = Y1[j]; g[1] = 2*Y2[j]; g[2] = 3*Y3[j];
    tk = dicho2(f, g, t[j], t[j+1], tmp-sumsav, p, SCN_EPSILON);    
    f[0] = X0[j]; f[1] = X1[j]; f[2] = X2[j]; f[3] = X3[j];
    g[0] = Y0[j]; g[1] = Y1[j]; g[2] = Y2[j]; g[3] = Y3[j];
    X[k] = evalpoly(4, (double *)f, tk);
    Y[k] = evalpoly(4, (double *)g, tk);
  }             
 scn_samplespline_end:
  free(t);
  free(X0); free(X1); free(X2); free(X3); 
  free(Y0); free(Y1); free(Y2); free(Y3); 
  return 1;
} // scn_samplespline()

/* ==================================== */
int32_t scn_samplespline3d(double *x, double *y, double *z, int32_t n, int32_t m, double *X, double *Y, double *Z)
/* ==================================== */
/*! \fn double * scn_samplespline3d(double *x, double *y, double *z, int32_t n, int32_t m, double *X, double *Y, double *Z)
    \param x (entr�e) : tableau des abcisses des points de contr�le (taille n)
    \param y (entr�e) : tableau des ordonn�es des points de contr�le (taille n)
    \param z (entr�e) : tableau des cotes des points de contr�le (taille n)
    \param n (entr�e) : nombre de points de contr�le
    \param m (entr�e) : nombre des �chantillons
    \param X (sortie) : tableau des absisses des �chantillons (taille m)
    \param Y (sortie) : tableau des ordonn�es des �chantillons (taille m)
    \param Z (sortie) : tableau des cotes des �chantillons (taille m)
    \brief �chantilonne une spline 3d selon son abcisse curviligne
    \warning la m�moire pour stocker les r�sultats X, Y et Z doit avoir �t� allou�e
*/
{
#undef F_NAME
#define F_NAME "scn_samplespline3d"
  double *X0, *X1, *X2, *X3;
  double *Y0, *Y1, *Y2, *Y3;
  double *Z0, *Z1, *Z2, *Z3;
  double *t;
  double tk, tmp, L = 0;
  double sum, sumsav;
  int32_t p = m*10;
  int32_t i, j, k;
  double f[4], g[4], h[4];

  if (n < 1)
  {
    fprintf(stderr, "%s: not enough points\n", F_NAME);
    return 0;
  }

  if (m < 2)
  {
    fprintf(stderr, "%s: not enough samples\n", F_NAME);
    return 0;
  }

  if (n == 1)
  {
    for (k = 0; k < m-1; k++) { X[k] = x[0]; Y[k] = y[0]; Z[k] = z[0]; }
    return 1;
  }

  if (n == 2)
  {
    X[0] = x[0]; Y[0] = y[0]; Z[0] = z[0];
    X[m-1] = x[n-1]; Y[m-1] = y[n-1]; Z[m-1] = z[n-1];
    for (k = 1; k < m-1; k++)
    {
      X[k] = x[0] + k * (x[n-1] - x[0]) / (m-1); 
      Y[k] = y[0] + k * (y[n-1] - y[0]) / (m-1); 
      Z[k] = z[0] + k * (z[n-1] - z[0]) / (m-1); 
    }
    return 1;
  }
  
  t = (double *)calloc(1,n * sizeof(double)); assert(t != NULL);
  X0 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X0 != NULL);
  X1 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X1 != NULL);
  X2 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X2 != NULL);
  X3 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X3 != NULL);
  Y0 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y0 != NULL);
  Y1 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y1 != NULL);
  Y2 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y2 != NULL);
  Y3 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y3 != NULL);
  Z0 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Z0 != NULL);
  Z1 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Z1 != NULL);
  Z2 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Z2 != NULL);
  Z3 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Z3 != NULL);
  for(i = 0; i < n; i++) t[i] = (double)i;

  if ((x[0] != x[n-1]) || (y[0] != y[n-1]) || (z[0] != z[n-1]))
  {
    if (scn_solvespline(t, x, n, X0, X1, X2, X3) == 0) return 0;
    if (scn_solvespline(t, y, n, Y0, Y1, Y2, Y3) == 0) return 0;
    if (scn_solvespline(t, z, n, Z0, Z1, Z2, Z3) == 0) return 0;
  }
  else
  {
    if (scn_solveclosedspline(t, x, n, X0, X1, X2, X3) == 0) return 0;
    if (scn_solveclosedspline(t, y, n, Y0, Y1, Y2, Y3) == 0) return 0;
    if (scn_solveclosedspline(t, z, n, Z0, Z1, Z2, Z3) == 0) return 0;
  }

  for (i = 0; i < n-1; i++)
  {
    f[0] = X1[i]; f[1] = 2*X2[i]; f[2] = 3*X3[i];
    g[0] = Y1[i]; g[1] = 2*Y2[i]; g[2] = 3*Y3[i];
    h[0] = Z1[i]; h[1] = 2*Z2[i]; h[2] = 3*Z3[i];
    L = L + integrale3(f, g, h, t[i], t[i+1], p);
  }

  if (L < SCN_EPSILON)
  {
    for (k = 0; k < m; k++) 
    { 
      X[k] = x[0]; Y[k] = y[0]; Z[k] = z[0];
    }
    goto scn_samplespline3d_end;
  }

  X[0] = x[0]; Y[0] = y[0]; Z[0] = z[0];
  X[m-1] = x[n-1]; Y[m-1] = y[n-1]; Z[m-1] = z[n-1];
  for (k = 1; k < m-1; k++)
  {
    tmp = k * L/(m-1);
    j = 0;
    sumsav = sum = 0; 
    while (sum < tmp)
    {
      sumsav = sum;
      f[0] = X1[j]; f[1] = 2*X2[j]; f[2] = 3*X3[j];
      g[0] = Y1[j]; g[1] = 2*Y2[j]; g[2] = 3*Y3[j];
      h[0] = Z1[j]; h[1] = 2*Z2[j]; h[2] = 3*Z3[j];
      sum = sum + integrale3(f, g, h, t[j], t[j+1], p);
      j++;
    }
    j--;
    f[0] = X1[j]; f[1] = 2*X2[j]; f[2] = 3*X3[j];
    g[0] = Y1[j]; g[1] = 2*Y2[j]; g[2] = 3*Y3[j];
    h[0] = Z1[j]; h[1] = 2*Z2[j]; h[2] = 3*Z3[j];
    tk = dicho3(f, g, h, t[j], t[j+1], tmp-sumsav, p, SCN_EPSILON);    
    f[0] = X0[j]; f[1] = X1[j]; f[2] = X2[j]; f[3] = X3[j];
    g[0] = Y0[j]; g[1] = Y1[j]; g[2] = Y2[j]; g[3] = Y3[j];
    h[0] = Z0[j]; h[1] = Z1[j]; h[2] = Z2[j]; h[3] = Z3[j];
    X[k] = evalpoly(4, (double *)f, tk);
    Y[k] = evalpoly(4, (double *)g, tk);
    Z[k] = evalpoly(4, (double *)h, tk);
  }             
 scn_samplespline3d_end:
  free(t);
  free(X0); free(X1); free(X2); free(X3); 
  free(Y0); free(Y1); free(Y2); free(Y3); 
  free(Z0); free(Z1); free(Z2); free(Z3); 
  return 1;
} // scn_samplespline3d()

/* ==================================== */
double scn_lengthspline(double *X0, double *X1, double *X2, double *X3, 
			double *Y0, double *Y1, double *Y2, double *Y3, 
			int32_t nctrl)
/* ==================================== */
/*! \fn double * scn_lengthspline(double *X0, double *X1, double *X2, double *X3, 
			double *Y0, double *Y1, double *Y2, double *Y3, 
			int32_t n)
    \param X0 (entr�e) : tableau des coef. spline de degr� 0 (taille nctrl-1) 
    \param X1 (entr�e) : tableau des coef. spline de degr� 1 (taille nctrl-1) 
    \param X2 (entr�e) : tableau des coef. spline de degr� 2 (taille nctrl-1) 
    \param X3 (entr�e) : tableau des coef. spline de degr� 3 (taille nctrl-1) 
    \param Y0 (entr�e) : tableau des coef. spline de degr� 0 (taille nctrl-1) 
    \param Y1 (entr�e) : tableau des coef. spline de degr� 1 (taille nctrl-1) 
    \param Y2 (entr�e) : tableau des coef. spline de degr� 2 (taille nctrl-1) 
    \param Y3 (entr�e) : tableau des coef. spline de degr� 3 (taille nctrl-1) 
    \param nctrl (entr�e) : nombre de points de contr�le
    \brief calcule la longueur d'une spline 2d
*/
{
#undef F_NAME
#define F_NAME "scn_lengthspline"
  double *t, L = 0, f[3], g[3];
  int32_t i, p = 10000;
  
  t = (double *)calloc(1, nctrl * sizeof(double)); assert(t != NULL);

  for(i = 0; i < nctrl; i++) t[i] = (double)i;

  for (i = 0; i < nctrl-1; i++)
  {
    f[0] = X1[i]; f[1] = 2*X2[i]; f[2] = 3*X3[i];
    g[0] = Y1[i]; g[1] = 2*Y2[i]; g[2] = 3*Y3[i];

    /*
printf("%s: \n", F_NAME);
printf("f %g %g %g \n", f[0], f[1], f[2]);
printf("g %g %g %g \n", g[0], g[1], g[2]);
printf("i = %d t[i] = %g t[i+1] = %g\n", i, t[i], t[i+1]);
    */

    L = L + integrale2(f, g, t[i], t[i+1], p);
  }
  free(t);
  return L;
} // scn_lengthspline()

/* ==================================== */
double scn_lengthspline3d(double *X0, double *X1, double *X2, double *X3, 
			double *Y0, double *Y1, double *Y2, double *Y3, 
			double *Z0, double *Z1, double *Z2, double *Z3,
			int32_t nctrl)
/* ==================================== */
/*! \fn double * scn_lengthspline3d(double *X0, double *X1, double *X2, double *X3, 
			double *Y0, double *Y1, double *Y2, double *Y3, 
			double *Z0, double *Z1, double *Z2, double *Z3,
			int32_t n)
    \param X0 (entr�e) : tableau des coef. spline de degr� 0 (taille nctrl-1) 
    \param X1 (entr�e) : tableau des coef. spline de degr� 1 (taille nctrl-1) 
    \param X2 (entr�e) : tableau des coef. spline de degr� 2 (taille nctrl-1) 
    \param X3 (entr�e) : tableau des coef. spline de degr� 3 (taille nctrl-1) 
    \param Y0 (entr�e) : tableau des coef. spline de degr� 0 (taille nctrl-1) 
    \param Y1 (entr�e) : tableau des coef. spline de degr� 1 (taille nctrl-1) 
    \param Y2 (entr�e) : tableau des coef. spline de degr� 2 (taille nctrl-1) 
    \param Y3 (entr�e) : tableau des coef. spline de degr� 3 (taille nctrl-1) 
    \param Z0 (entr�e) : tableau des coef. spline de degr� 0 (taille nctrl-1) 
    \param Z1 (entr�e) : tableau des coef. spline de degr� 1 (taille nctrl-1) 
    \param Z2 (entr�e) : tableau des coef. spline de degr� 2 (taille nctrl-1) 
    \param Z3 (entr�e) : tableau des coef. spline de degr� 3 (taille nctrl-1) 
    \param nctrl (entr�e) : nombre de points de contr�le
    \brief calcule la longueur d'une spline 3d
*/
{
#undef F_NAME
#define F_NAME "scn_lengthspline3d"
  double *t, L = 0, f[3], g[3], h[3];
  int32_t i, p = 10000;
  
  t = (double *)calloc(1, nctrl * sizeof(double)); assert(t != NULL);

  for(i = 0; i < nctrl; i++) t[i] = (double)i;

  for (i = 0; i < nctrl-1; i++)
  {
    f[0] = X1[i]; f[1] = 2*X2[i]; f[2] = 3*X3[i];
    g[0] = Y1[i]; g[1] = 2*Y2[i]; g[2] = 3*Y3[i];
    h[0] = Z1[i]; h[1] = 2*Z2[i]; h[2] = 3*Z3[i];

    /*
printf("%s: \n", F_NAME);
printf("f %g %g %g \n", f[0], f[1], f[2]);
printf("g %g %g %g \n", g[0], g[1], g[2]);
printf("h %g %g %g \n", h[0], h[1], h[2]);
printf("i = %d t[i] = %g t[i+1] = %g\n", i, t[i], t[i+1]);
    */

    L = L + integrale3(f, g, h, t[i], t[i+1], p);
  }
  free(t);
  return L;
} // scn_lengthspline3d()

// =================================================
// CALCUL DE COURBURES
// =================================================

/* ==================================== */
int32_t scn_curvatures(double *x, double *y, int32_t n, int32_t m, double *sk, double *rhok)
/* ==================================== */
/*! \fn double * scn_curvatures(double *x, double *y, int32_t n, int32_t m, double *sk, double *rhok)
    \param x (entr�e) : tableau des abcisses des points de contr�le (taille n)
    \param y (entr�e) : tableau des ordonn�es des points de contr�le (taille n)
    \param n (entr�e) : nombre de points de contr�le
    \param m (entr�e) : nombre des �chantillons
    \param sk (sortie) : tableau  des absisses curvilignes (taille m)
    \param rhok (sortie) : tableau des courbures (taille m)
    \brief calcule de la courbure le long d'une spline 2D param�tr�e : 
           K = (x'y'' - y'x'') / (x'^2 + y'^2)^(3/2)
	   source: http://en.wikipedia.org/wiki/Curvature
    \warning la m�moire pour stocker les r�sultats sk, rhok doit avoir �t� allou�e
*/
{
#undef F_NAME
#define F_NAME "scn_curvatures"
  double *X0, *X1, *X2, *X3;
  double *Y0, *Y1, *Y2, *Y3;
  double *t;
  double tk, tmp, eval, xp, yp, xs, ys, L = 0;
  double sum, sumsav;
  int32_t p = m * 10;
  int32_t i, j, k;
  double f[3], g[3];
  
  t = (double *)calloc(1,n * sizeof(double)); assert(t != NULL);
  X0 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X0 != NULL);
  X1 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X1 != NULL);
  X2 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X2 != NULL);
  X3 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X3 != NULL);
  Y0 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y0 != NULL);
  Y1 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y1 != NULL);
  Y2 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y2 != NULL);
  Y3 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y3 != NULL);

  for(i=0; i < n; i++) t[i] = (double)i;

  if (scn_solvespline(t, x, n, X0, X1, X2, X3) == 0) return 0;
  if (scn_solvespline(t, y, n, Y0, Y1, Y2, Y3) == 0) return 0;

  for(i=0; i < n-1; i++)
  {
    f[0] = X1[i]; f[1] = 2*X2[i]; f[2] = 3*X3[i];
    g[0] = Y1[i]; g[1] = 2*Y2[i]; g[2] = 3*Y3[i];
    L=L+integrale2(f, g, t[i], t[i+1], p);
  }

  sk[0] = rhok[0] = 0.0;
  for(k=1; k < m; k++)
  {
    tmp=k*L/m;
    j = 0;
    sumsav = sum = 0; 
    while (sum<tmp)
    {
      sumsav = sum;
      f[0] = X1[j]; f[1] = 2*X2[j]; f[2] = 3*X3[j];
      g[0] = Y1[j]; g[1] = 2*Y2[j]; g[2] = 3*Y3[j];
      sum=sum+integrale2(f, g, t[j], t[j+1], p);
      j++;
    }
    j--;
    f[0] = X1[j]; f[1] = 2*X2[j]; f[2] = 3*X3[j];
    g[0] = Y1[j]; g[1] = 2*Y2[j]; g[2] = 3*Y3[j];
    tk=dicho2(f, g, t[j], t[j+1], tmp-sumsav, p, SCN_EPSILON);    
    xp = evalpoly(3, (double *)f, tk);
    yp = evalpoly(3, (double *)g, tk);
    eval = sqrt(xp*xp + yp*yp);
    eval = eval * eval * eval;
    f[0] = 2*X2[j]; f[1] = 6*X3[j]; f[2] = 0;
    g[0] = 2*Y2[j]; g[1] = 6*Y3[j]; g[2] = 0;
    sk[k] = tmp;
    xs = evalpoly(3, (double *)f, tk);
    ys = evalpoly(3, (double *)g, tk);
    rhok[k] = (xp*ys - yp*xs) / eval;
  }             
  free(t);
  free(X0); free(X1); free(X2); free(X3); 
  free(Y0); free(Y1); free(Y2); free(Y3); 
  return 1;
} // scn_curvatures()

/* ==================================== */
int32_t scn_curvatures3d(double *x, double *y, double *z, int32_t n, int32_t m, double *sk, double *rhok)
/* ==================================== */
/*! \fn double * scn_curvatures(double *x, double *y, double *z, int32_t n, int32_t m, double *sk, double *rhok)
    \param x (entr�e) : tableau des abcisses des points de contr�le (taille n)
    \param y (entr�e) : tableau des ordonn�es des points de contr�le (taille n)
    \param z (entr�e) : tableau des cotes des points de contr�le (taille n)
    \param n (entr�e) : nombre de points de contr�le
    \param m (entr�e) : nombre des �chantillons
    \param sk (sortie) : tableau  des absisses curvilignes (taille m)
    \param rhok (sortie) : tableau des courbures (taille m)
    \brief calcule de la courbure le long d'une spline 3D param�tr�e : 
      K = ((z''y' - y''z')^2 + (x''z' - z''x')^2 + (y''x' - x''y')^2)^(1/2) / 
          (x'^2 + y'^2 + z'^2)^(3/2)
      source: http://en.wikipedia.org/wiki/Curvature
    \warning la m�moire pour stocker les r�sultats sk, rhok doit avoir �t� allou�e
*/
{
#undef F_NAME
#define F_NAME "scn_curvatures3d"
  double *X0, *X1, *X2, *X3;
  double *Y0, *Y1, *Y2, *Y3;
  double *Z0, *Z1, *Z2, *Z3;
  double *t;
  double tk, tmp, eval, xp, yp, zp, xs, ys, zs, L = 0;
  double sum, sumsav;
  int32_t p = m * 10;
  int32_t i, j, k;
  double f[3], g[3], h[3];
  
  t = (double *)calloc(1,n * sizeof(double)); assert(t != NULL);
  X0 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X0 != NULL);
  X1 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X1 != NULL);
  X2 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X2 != NULL);
  X3 = (double *)calloc(1,(n-1) * sizeof(double)); assert(X3 != NULL);
  Y0 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y0 != NULL);
  Y1 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y1 != NULL);
  Y2 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y2 != NULL);
  Y3 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Y3 != NULL);
  Z0 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Z0 != NULL);
  Z1 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Z1 != NULL);
  Z2 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Z2 != NULL);
  Z3 = (double *)calloc(1,(n-1) * sizeof(double)); assert(Z3 != NULL);

  for (i = 0; i < n; i++) t[i] = (double)i;

  if (scn_solvespline(t, x, n, X0, X1, X2, X3) == 0) return 0;
  if (scn_solvespline(t, y, n, Y0, Y1, Y2, Y3) == 0) return 0;
  if (scn_solvespline(t, z, n, Z0, Z1, Z2, Z3) == 0) return 0;

  for (i = 0; i < n-1; i++)
  {
    f[0] = X1[i]; f[1] = 2*X2[i]; f[2] = 3*X3[i];
    g[0] = Y1[i]; g[1] = 2*Y2[i]; g[2] = 3*Y3[i];
    h[0] = Z1[i]; h[1] = 2*Z2[i]; h[2] = 3*Z3[i];
    L = L + integrale3(f, g, h, t[i], t[i+1], p);
  }

  sk[0] = rhok[0] = 0.0;
  for (k = 1; k < m; k++)
  {
    tmp = (k*L) / m;
    j = 0;
    sumsav = sum = 0; 
    while (sum < tmp)
    {
      sumsav = sum;
      f[0] = X1[j]; f[1] = 2*X2[j]; f[2] = 3*X3[j];
      g[0] = Y1[j]; g[1] = 2*Y2[j]; g[2] = 3*Y3[j];
      h[0] = Z1[j]; h[1] = 2*Z2[j]; h[2] = 3*Z3[j];
      sum = sum + integrale3(f, g, h, t[j], t[j+1], p);
      j++;
    }
    j--;
    f[0] = X1[j]; f[1] = 2*X2[j]; f[2] = 3*X3[j];
    g[0] = Y1[j]; g[1] = 2*Y2[j]; g[2] = 3*Y3[j];
    h[0] = Z1[j]; h[1] = 2*Z2[j]; h[2] = 3*Z3[j];
    tk = dicho3(f, g, h, t[j], t[j+1], tmp-sumsav, p, SCN_EPSILON);    
    xp = evalpoly(3, (double *)f, tk);
    yp = evalpoly(3, (double *)g, tk);
    zp = evalpoly(3, (double *)h, tk);
    eval = sqrt(xp*xp + yp*yp + zp*zp);
    eval = eval * eval * eval;
    f[0] = 2*X2[j]; f[1] = 6*X3[j]; f[2] = 0;
    g[0] = 2*Y2[j]; g[1] = 6*Y3[j]; g[2] = 0;
    h[0] = 2*Z2[j]; h[1] = 6*Z3[j]; h[2] = 0;
    sk[k] = tmp;
    xs = evalpoly(3, (double *)f, tk);
    ys = evalpoly(3, (double *)g, tk);
    zs = evalpoly(3, (double *)h, tk);
    tk = (zs*yp - ys*zp) * (zs*yp - ys*zp);
    tk = tk + (xs*zp - zs*xp) * (xs*zp - zs*xp);
    tk = tk + (ys*xp - xs*yp) * (ys*xp - xs*yp);
    rhok[k] = sqrt(tk) / eval;
  }             
  free(t);
  free(X0); free(X1); free(X2); free(X3); 
  free(Y0); free(Y1); free(Y2); free(Y3); 
  free(Z0); free(Z1); free(Z2); free(Z3); 
  return 1;
} // scn_curvatures3d()

/* ==================================== */
int32_t scn_tangents3d(
		       double *X0, double *X1, double *X2, double *X3,
		       double *Y0, double *Y1, double *Y2, double *Y3,
		       double *Z0, double *Z1, double *Z2, double *Z3,
		       int32_t n, int32_t m, double *sk, 
		       double *xk, double *yk, double *zk, 
		       double *txk, double *tyk, double *tzk)
/* ==================================== */
/*! \fn double * scn_tangents(
		       double *X0, double *X1, double *X2, double *X3,
		       double *Y0, double *Y1, double *Y2, double *Y3,
		       double *Z0, double *Z1, double *Z2, double *Z3,
		       int32_t n, int32_t m, double *sk, 
		       double *xk, double *yk, double *zk, 
		       double *txk, double *tyk, double *tzk)
    \param X0, X1, X2, X3 (entr�e) : tableau des coefficients des polynomes X
    \param Y0, Y1, Y2, Y3 (entr�e) : tableau des coefficients des polynomes Y
    \param Z0, Z1, Z2, Z3 (entr�e) : tableau des coefficients des polynomes Z
    \param n (entr�e) : nombre de points de contr�le
    \param m (entr�e) : nombre des �chantillons
    \param sk (sortie) : tableau  des absisses curvilignes (taille m)
    \param xk (sortie) :
    \param yk (sortie) :
    \param zk (sortie) : tableaux des points de base des tangentes (taille m)
    \param txk (sortie) :
    \param tyk (sortie) :
    \param tzk (sortie) : tableaux des tangentes (taille m)
    \brief calcule les tangentes le long d'une spline 3D param�tr�e
    \warning la m�moire pour stocker les r�sultats sk, xk, yk, zk, txk, tyk, tzk doit avoir �t� allou�e
*/
{
#undef F_NAME
#define F_NAME "scn_tangents3d"
  double tk, tmp, L = 0;
  double sum, sumsav;
  int32_t p = m * 10;
  int32_t i, j, k;
  double f[4], g[4], h[4];
  
  // calcule la longueur de la courbe 
  for (i = 0; i < n-1; i++)
  {
    f[0] = X1[i]; f[1] = 2*X2[i]; f[2] = 3*X3[i];
    g[0] = Y1[i]; g[1] = 2*Y2[i]; g[2] = 3*Y3[i];
    h[0] = Z1[i]; h[1] = 2*Z2[i]; h[2] = 3*Z3[i];
    L = L + integrale3(f, g, h, (double)(i), (double)(i+1), p);
  }

#ifdef DEBUG_scn_tangents3d
  printf("%s: L=%g\n", F_NAME, L);
#endif

  sk[0] = 0.0;
  for (k = 0; k < m; k++)
  {
    tmp = (k*L) / m; // emplacement du k-ieme echantillon
    j = 0;
    sumsav = sum = 0; 
    while (sum < tmp) // recherche du segment de la spline o� il se trouve
    {
      sumsav = sum;
      f[0] = X1[j]; f[1] = 2*X2[j]; f[2] = 3*X3[j];
      g[0] = Y1[j]; g[1] = 2*Y2[j]; g[2] = 3*Y3[j];
      h[0] = Z1[j]; h[1] = 2*Z2[j]; h[2] = 3*Z3[j];
      sum = sum + integrale3(f, g, h, (double)(j), (double)(j+1), p);
      j++;
    }
    j--; // c'est le j-�me segment
    f[0] = X1[j]; f[1] = 2*X2[j]; f[2] = 3*X3[j];
    g[0] = Y1[j]; g[1] = 2*Y2[j]; g[2] = 3*Y3[j];
    h[0] = Z1[j]; h[1] = 2*Z2[j]; h[2] = 3*Z3[j];
    // f,g,h sont les coef des polynomes d�riv�e premi�re

    tk = dicho3(f, g, h, (double)(j), (double)(j+1), tmp-sumsav, p, SCN_EPSILON);    
    // tk est l'abcisse curviligne du k-ieme echantillon

    txk[k] = evalpoly(3, (double *)f, tk);
    tyk[k] = evalpoly(3, (double *)g, tk);
    tzk[k] = evalpoly(3, (double *)h, tk);

    f[0] = X0[j]; f[1] = X1[j]; f[2] = X2[j]; f[3] = X3[j];
    g[0] = Y0[j]; g[1] = Y1[j]; g[2] = Y2[j]; g[3] = Y3[j];
    h[0] = Z0[j]; h[1] = Z1[j]; h[2] = Z2[j]; h[3] = Z3[j];
    xk[k] = evalpoly(4, (double *)f, tk);
    yk[k] = evalpoly(4, (double *)g, tk);
    zk[k] = evalpoly(4, (double *)h, tk);    

#ifdef DEBUG_scn_tangents3d
    printf("%s: k=%d pk=%g %g %g tk=%g %g %g\n", F_NAME, k, xk[k], yk[k], zk[k], txk[k], tyk[k], tzk[k]);
#endif

  }             
  return 1;
} // scn_tangents3d()

// =================================================
// APPROXIMATION D'UNE COURBE DISCRETE
// =================================================

/* ==================================== */
void distancesegments1(int32_t *Y, int32_t *W, int32_t i1, int32_t i2, double *delta, int32_t *arg)
/* ==================================== */
{
  double d, dmin, del = -1;
  int32_t ar, i, j;
  for(i=i1; i<=i2; i++)
  {
    dmin = HUGE_VAL;
    for(j=i1; j<=i2; j++)
    {
      d = sqrt((i-j)*(i-j) + (Y[i]-W[j])*(Y[i]-W[j]));
      if (d < dmin) dmin = d;
    }
    if (dmin > del)
    {
      del = dmin;
      ar = i;
    }
  }
#ifdef DEBUG
  printf("distancesegments: i1=%d ; i2=%d ; delta=%g, arg=%d\n", i1, i2, del, ar);
#endif
  *delta = del;
  *arg = ar;
} // distancesegments1()

/* ==================================== */
void distancesegments(int32_t *X, int32_t *Y, int32_t *V, int32_t *W, int32_t i1, int32_t i2, double *delta, int32_t *arg)
/* ==================================== */
{
  double d, dmin, del = -1;
  int32_t ar, i, j;
  for(i=i1; i<=i2; i++)
  {
    dmin = HUGE_VAL;
    for(j=i1; j<=i2; j++)
    {
      d = sqrt((X[i]-V[j])*(X[i]-V[j]) + (Y[i]-W[j])*(Y[i]-W[j]));
      if (d < dmin) dmin = d;
    }
    if (dmin > del)
    {
      del = dmin;
      ar = i;
    }
  }
#ifdef DEBUG
  printf("distancesegments: i1=%d ; i2=%d ; delta=%g, arg=%d\n", i1, i2, del, ar);
#endif
  *delta = del;
  *arg = ar;
} // distancesegments()

/* ==================================== */
void distancesegments3d(int32_t *X, int32_t *Y, int32_t *Z, int32_t *U, int32_t *V, int32_t *W, int32_t i1, int32_t i2, double *delta, int32_t *arg)
/* ==================================== */
/*! \fn void distancesegments3d(int32_t *X, int32_t *Y, int32_t *Z, int32_t *U, int32_t *V, int32_t *W, int32_t i1, int32_t i2, double *delta, int32_t *arg)
    \param X,Y,Z (entr�e) : courbe initiale C
    \param U,V,W (entr�e) : courbe discr�tis�e D
    \param i1,i2 (entr�e) : intervalle dans lequel les points des 2 courbes sont consid�r�s
    \param delta (sortie) : distance maximale entre C et D
    \param arg (sortie) : indice dans C r�alisant le max
    \brief 
    \warning 
*/
{
  double d, dmin, del = -1;
  int32_t ar, i, j;
  for(i=i1; i<=i2; i++)
  {
    dmin = HUGE_VAL;
    for(j=i1; j<=i2; j++)
    {
      d = sqrt((X[i]-U[j])*(X[i]-U[j]) + (Y[i]-V[j])*(Y[i]-V[j]) + (Z[i]-W[j])*(Z[i]-W[j]));
      if (d < dmin) dmin = d;
    }
    if (dmin > del)
    {
      del = dmin;
      ar = i;
    }
  }
#ifdef DEBUG
  printf("distancesegments3d: i1=%d ; i2=%d ; delta=%g, arg=%d\n", i1, i2, del, ar);
#endif
  *delta = del;
  *arg = ar;
} // distancesegments3d()

/* ==================================== */
void scn_discretisespline1(int32_t nctrl, 
			  double *C0, double *C1, double *C2, double *C3, 
			  int32_t *C, int32_t *W)
/* ==================================== */
/*! \fn double *scn_discretisespline1(int32_t nctrl, double *C0, double *C1, double *C2, double *C3, int32_t *C, int32_t *W)
    \param nctrl (entr�e) : nombre de points de contr�le
    \param C0 (entr�e) : tableau des coef. spline de degr� 0 (taille nctrl-1) 
    \param C1 (entr�e) : tableau des coef. spline de degr� 1 (taille nctrl-1) 
    \param C2 (entr�e) : tableau des coef. spline de degr� 2 (taille nctrl-1) 
    \param C3 (entr�e) : tableau des coef. spline de degr� 3 (taille nctrl-1) 
    \param C (entr�e) : tableau des abscisses des pts de contr�le (taille nctrl) 
    \param W (sortie) : courbe discr�tis�e
    \brief discr�tise la spline
    \warning la m�moire pour stocker le r�sultat W doit avoir �t� allou�e
*/
{
  int32_t i, j;
  double f[4];
  for(i = 0; i < nctrl-1; i++)
  {
    f[0] = C0[i];
    f[1] = C1[i];
    f[2] = C2[i];
    f[3] = C3[i];
    for(j = C[i]; j <= C[i+1]; j++)
      W[j] = (int32_t)evalpoly(4, (double *)f, (double)j);
  }
} // scn_discretisespline1()

/* ==================================== */
void scn_discretisespline(int32_t nctrl, 
			  double *C0, double *C1, double *C2, double *C3, 
			  double *D0, double *D1, double *D2, double *D3, 
			  int32_t *C, int32_t *V, int32_t *W)
/* ==================================== */
/*! \fn double *scn_discretisespline(int32_t nctrl, double *C0, double *C1, double *C2, double *C3, double *D0, double *D1, double *D2, double *D3, int32_t *C, int32_t *V, int32_t *W)
    \param nctrl (entr�e) : nombre de points de contr�le
    \param C0 (entr�e) : tableau des coef. spline de degr� 0 (taille nctrl-1) 
    \param C1 (entr�e) : tableau des coef. spline de degr� 1 (taille nctrl-1) 
    \param C2 (entr�e) : tableau des coef. spline de degr� 2 (taille nctrl-1) 
    \param C3 (entr�e) : tableau des coef. spline de degr� 3 (taille nctrl-1) 
    \param D0 (entr�e) : tableau des coef. spline de degr� 0 (taille nctrl-1) 
    \param D1 (entr�e) : tableau des coef. spline de degr� 1 (taille nctrl-1) 
    \param D2 (entr�e) : tableau des coef. spline de degr� 2 (taille nctrl-1) 
    \param D3 (entr�e) : tableau des coef. spline de degr� 3 (taille nctrl-1) 
    \param C (entr�e) : tableau des index des pts de contr�le (taille nctrl) 
    \param V (sortie) : courbe discr�tis�e (abscisses)
    \param W (sortie) : courbe discr�tis�e (ordonn�es)
    \brief discr�tise la spline param�trique
    \warning la m�moire pour stocker le r�sultat V, W doit avoir �t� allou�e
*/
{
  int32_t i, j;
  double f[4];
  for(i = 0; i < nctrl-1; i++)
  {
    f[0] = C0[i]; f[1] = C1[i]; f[2] = C2[i]; f[3] = C3[i];
    for(j = C[i]; j <= C[i+1]; j++) V[j] = (int32_t)evalpoly(4, (double *)f, (double)j);
    f[0] = D0[i]; f[1] = D1[i]; f[2] = D2[i]; f[3] = D3[i];
    for(j = C[i]; j <= C[i+1]; j++) W[j] = (int32_t)evalpoly(4, (double *)f, (double)j);
  }
} // scn_discretisespline()

/* ==================================== */
void scn_discretisespline3d(int32_t nctrl, 
			  double *C0, double *C1, double *C2, double *C3, 
			  double *D0, double *D1, double *D2, double *D3, 
			  double *E0, double *E1, double *E2, double *E3, 
			  int32_t *C, int32_t *U, int32_t *V, int32_t *W)
/* ==================================== */
/*! \fn void scn_discretisespline3d(int32_t nctrl, 
			  double *C0, double *C1, double *C2, double *C3, 
			  double *D0, double *D1, double *D2, double *D3, 
			  double *E0, double *E1, double *E2, double *E3, 
			  int32_t *C, int32_t *U, int32_t *V, int32_t *W)
    \param nctrl (entr�e) : nombre de points de contr�le
    \param C0 (entr�e) : tableau des coef. spline de degr� 0 (taille nctrl-1) 
    \param C1 (entr�e) : tableau des coef. spline de degr� 1 (taille nctrl-1) 
    \param C2 (entr�e) : tableau des coef. spline de degr� 2 (taille nctrl-1) 
    \param C3 (entr�e) : tableau des coef. spline de degr� 3 (taille nctrl-1) 
    \param D0 (entr�e) : tableau des coef. spline de degr� 0 (taille nctrl-1) 
    \param D1 (entr�e) : tableau des coef. spline de degr� 1 (taille nctrl-1) 
    \param D2 (entr�e) : tableau des coef. spline de degr� 2 (taille nctrl-1) 
    \param D3 (entr�e) : tableau des coef. spline de degr� 3 (taille nctrl-1) 
    \param E0 (entr�e) : tableau des coef. spline de degr� 0 (taille nctrl-1) 
    \param E1 (entr�e) : tableau des coef. spline de degr� 1 (taille nctrl-1) 
    \param E2 (entr�e) : tableau des coef. spline de degr� 2 (taille nctrl-1) 
    \param E3 (entr�e) : tableau des coef. spline de degr� 3 (taille nctrl-1) 
    \param C (entr�e) : tableau des index des pts de contr�le (taille nctrl) 
    \param U (sortie) : courbe discr�tis�e (abscisses)
    \param V (sortie) : courbe discr�tis�e (ordonn�es)
    \param W (sortie) : courbe discr�tis�e (cotes)
    \brief discr�tise la spline param�trique
    \warning la m�moire pour stocker le r�sultat U, V, W doit avoir �t� allou�e
*/
{
  int32_t i, j;
  double f[4];
  for(i = 0; i < nctrl-1; i++)
  {
    f[0] = C0[i]; f[1] = C1[i]; f[2] = C2[i]; f[3] = C3[i];
    for(j = C[i]; j <= C[i+1]; j++) U[j] = (int32_t)evalpoly(4, (double *)f, (double)j);
    f[0] = D0[i]; f[1] = D1[i]; f[2] = D2[i]; f[3] = D3[i];
    for(j = C[i]; j <= C[i+1]; j++) V[j] = (int32_t)evalpoly(4, (double *)f, (double)j);
    f[0] = E0[i]; f[1] = E1[i]; f[2] = E2[i]; f[3] = E3[i];
    for(j = C[i]; j <= C[i+1]; j++) W[j] = (int32_t)evalpoly(4, (double *)f, (double)j);
  }
} // scn_discretisespline3d()

void printcurve1(int32_t *W, int32_t N, char *filename)
{
  FILE *fd = NULL;
  int32_t i; 
  fd = fopen(filename,"w");
  fprintf(fd, "%d\n", N);
  for (i = 0; i < N; i++)
    fprintf(fd, "%d %d\n", i, W[i]);
  fclose(fd);
}

void printctrlpoints1(int32_t *C, int32_t *Y, int32_t n, char *filename)
{
  FILE *fd = NULL;
  int32_t i; 
  fd = fopen(filename,"w");
  fprintf(fd, "b %d\n", n);
  for (i = 0; i < n; i++)
    fprintf(fd, "%d %d\n", C[i], Y[C[i]]);
  fclose(fd);
}

void printcurve(int32_t *V, int32_t *W, int32_t N, char *filename)
{
  FILE *fd = NULL;
  int32_t i; 
  fd = fopen(filename,"w");
  fprintf(fd, "c %d\n", N);
  for (i = 0; i < N; i++)
    fprintf(fd, "%d %d\n", V[i], W[i]);
  fclose(fd);
}

void printctrlpoints(int32_t *C, int32_t *X, int32_t *Y, int32_t n, char *filename)
{
  FILE *fd = NULL;
  int32_t i;
  fd = fopen(filename,"w");
  fprintf(fd, "b %d\n", n);
  for (i = 0; i < n; i++) fprintf(fd, "%d %d\n", X[C[i]], Y[C[i]]);
  fclose(fd);
}

void printspline2d(int32_t *C, int32_t *X, int32_t *Y, int32_t n, char *filename,
                     double *C0, double *C1, double *C2, double *C3,
                     double *D0, double *D1, double *D2, double *D3)
{
  FILE *fd = NULL;
  int32_t i;
  fd = fopen(filename,"w");
  fprintf(fd, "s %d\n", n);
  for (i = 0; i < n; i++) fprintf(fd, "%d %d\n", X[C[i]], Y[C[i]]);
  for (i = 0; i < n-1; i++) fprintf(fd, "%g %g %g %g %g %g %g %g\n", 
				    C0[i], D0[i], C1[i], D1[i], C2[i], D2[i], C3[i], D3[i]);
  fclose(fd);
}

void printcurve3d(int32_t *U, int32_t *V, int32_t *W, int32_t N, char *filename)
{
  FILE *fd = NULL;
  int32_t i; 
  fd = fopen(filename,"w");
  fprintf(fd, "C %d\n", N);
  for (i = 0; i < N; i++)
    fprintf(fd, "%d %d %d\n", U[i], V[i], W[i]);
  fclose(fd);
}

void printctrlpoints3d(int32_t *C, int32_t *X, int32_t *Y, int32_t *Z, int32_t n, char *filename)
{
  FILE *fd = NULL;
  int32_t i;
  fd = fopen(filename,"w");
  fprintf(fd, "b %d\n", n);
  for (i = 0; i < n; i++) fprintf(fd, "%d %d %d\n", X[C[i]], Y[C[i]], Z[C[i]]);
  fclose(fd);
}

/* ==================================== */
int32_t scn_approxcurve1(int32_t *Y, int32_t N, double deltamax, int32_t *Z, int32_t *n, double *C0, double *C1, double *C2, double *C3)
/* ==================================== */
/*! \fn int32_t scn_approxcurve1(int32_t *Y, int32_t N, double deltamax, int32_t *Z, int32_t *n, double *C0, double *C1, double *C2, double *C3)
    \param Y (entr�e) : tableau des ordonn�es des points du graphe de la fonction (taille N)
    \param N (entr�e) : nombre de points de la courbe
    \param deltamax (entr�e) : tol�rance sur l'approximation
    \param Z (sortie) : tableau des absisses des points de contr�le (taille N)
    \param n (sortie) : nombre de points de contr�le
    \param C0 (sortie) : tableau des coef. spline de degr� 0 (taille *n-1) 
    \param C1 (sortie) : tableau des coef. spline de degr� 1 (taille *n-1) 
    \param C2 (sortie) : tableau des coef. spline de degr� 2 (taille *n-1) 
    \param C3 (sortie) : tableau des coef. spline de degr� 3 (taille *n-1) 
    \brief trouve une approximation par une spline cubique de la fonction discrete 
           d�finie par Y
    \warning la m�moire pour stocker les r�sultats Z, C0..C3 doit avoir �t� allou�e
*/
#undef F_NAME
#define F_NAME "scn_approxcurve1"
{
  int32_t i, j, continuer;
  double *x = (double *)calloc(1,N * sizeof(double)); 
  double *y = (double *)calloc(1,N * sizeof(double)); 
  int32_t *W = (int32_t *)calloc(1,N * sizeof(int32_t)); 
  double delta; 
  int32_t arg;
  int32_t nctrl;
  int32_t niter = 0;
  
  assert(x != NULL); assert(y != NULL); assert(W != NULL);

  C0[0] = (double)Y[0];
  C1[0] = ((double)Y[N-1] - (double)Y[0]) / (double)N;
  C2[0] = C3[0] = 0.0;
  Z[0] = 0;
  Z[1] = N-1;
  scn_discretisespline1(2, C0, C1, C2, C3, Z, W);
  distancesegments1(Y, W, Z[0], Z[1], &delta, &arg);
#ifdef DEBUG
    { 
    char b[100]; 
    sprintf(b, "_C%02d", niter);
    printctrlpoints1(Z, Y, 2, b);
    sprintf(b, "_W%02d", niter);
    printcurve1(W, N, b);
    }
#endif

  nctrl = 3;
  Z[0] = 0;
  Z[1] = N/2;
  Z[2] = N-1;

  do
  {
    niter++;
    continuer = 0;
    for (i = 0; i < nctrl; i++)
    {
      x[i] = Z[i];
      y[i] = Y[Z[i]];
    }
    if (scn_solvespline(x, y, nctrl, C0, C1, C2, C3) == 0) return 0;
    scn_discretisespline1(nctrl, C0, C1, C2, C3, Z, W);
#ifdef DEBUG
    { 
    char b[100]; 
    sprintf(b, "_C%02d", niter);
    printctrlpoints1(Z, Y, nctrl, b);
    sprintf(b, "_W%02d", niter);
    printcurve1(W, N, b);
    }
#endif
    i = 0;
    while (i < nctrl-1)
    {
      distancesegments1(Y, W, Z[i], Z[i+1], &delta, &arg);
      if (delta > deltamax)
      { // insere le point arg dans Z � la position i+1 
        continuer = 1;
        nctrl++;
        for (j = nctrl-1; j > i+1; j--) Z[j] = Z[j-1];
        Z[i+1] = (Z[i]+Z[i+2])/2;
        i++;
      }
      i++;

#ifdef DEBUG
      printf("%s: nctrl=%d ; \n", F_NAME, nctrl);  
#endif
      
    }
  } while(continuer && (nctrl <= N));

  if (nctrl > N)
  {
    printf("%s: WARNING too many control points, tolerance may be too small\n", F_NAME);  
  }

  *n = nctrl;
  free(x);
  free(y);
  free(W);
  return 1;
} // scn_approxcurve1()

/* ==================================== */
int32_t scn_approxcurve(int32_t *X, int32_t *Y, int32_t N, double deltamax, int32_t *C, int32_t *n, double *C0, double *C1, double *C2, double *C3, double *D0, double *D1, double *D2, double *D3)
/* ==================================== */
/*! \fn int32_t scn_approxcurve(int32_t *X, int32_t *Y, int32_t N, double deltamax, int32_t *C, int32_t *n, double *C0, double *C1, double *C2, double *C3, double *D0, double *D1, double *D2, double *D3)
    \param X (entr�e) : tableau des abscisses des points de la courbe (taille N)
    \param Y (entr�e) : tableau des ordonn�es des points de la courbe (taille N)
    \param N (entr�e) : nombre de points de la courbe
    \param deltamax (entr�e) : tol�rance sur l'approximation
    \param C (sortie) : tableau des index des points de contr�le (taille N)
    \param n (sortie) : nombre de points de contr�le
    \param C0 (sortie) : tableau des coef. spline X de degr� 0 (taille *n-1) 
    \param C1 (sortie) : tableau des coef. spline X de degr� 1 (taille *n-1) 
    \param C2 (sortie) : tableau des coef. spline X de degr� 2 (taille *n-1) 
    \param C3 (sortie) : tableau des coef. spline X de degr� 3 (taille *n-1) 
    \param D0 (sortie) : tableau des coef. spline Y de degr� 0 (taille *n-1) 
    \param D1 (sortie) : tableau des coef. spline Y de degr� 1 (taille *n-1) 
    \param D2 (sortie) : tableau des coef. spline Y de degr� 2 (taille *n-1) 
    \param D3 (sortie) : tableau des coef. spline Y de degr� 3 (taille *n-1) 
    \brief trouve une approximation par deux splines cubiques de la courbe discrete 
           d�finie par X, Y
    \warning la m�moire pour stocker les r�sultats C, C0..C3, D0..D3 doit avoir �t� allou�e
*/
#undef F_NAME
#define F_NAME "scn_approxcurve"
{
  C[0] = 0;
  C[1] = N/2;
  C[2] = N-1;
  *n = 3;
  return scn_approxcurve_with_initial_control_points(X, Y, N, deltamax, C, n, 
						       C0, C1, C2, C3,
						       D0, D1, D2, D3);
} // scn_approxcurve()

/* ==================================== */
int32_t scn_approxcurve2(int32_t *X, int32_t *Y, int32_t *N, double deltamax)
/* ==================================== */
/*! \fn int32_t scn_approxcurve2(int32_t *X, int32_t *Y, int32_t *N, double deltamax)
    \param X (entr�e/sortie) : tableau des abscisses des points (taille N)
    \param Y (entr�e/sortie) : tableau des ordonn�es des points (taille N)
    \param N (entr�e/sortie) : nombre de points de la courbe
    \param deltamax (entr�e) : tol�rance sur l'approximation
    \brief trouve une approximation par deux splines cubiques de la courbe discrete d�finie par X, Y. Le r�sultat (liste de points de contr�le) est retourn� dans X, Y, N.
*/
#undef F_NAME
#define F_NAME "scn_approxcurve2"
{
  int32_t *C;
  int32_t ret, n, i, npoints = *N;
  double *C0, *C1, *C2, *C3, *D0, *D1, *D2, *D3;
  
  C = (int32_t *)calloc(1,npoints*sizeof(int32_t)); assert(C != NULL);
  C0 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(C0 != NULL);
  C1 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(C1 != NULL);
  C2 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(C2 != NULL);
  C3 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(C3 != NULL);
  D0 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(D0 != NULL);
  D1 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(D1 != NULL);
  D2 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(D2 != NULL);
  D3 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(D3 != NULL);
  
  C[0] = 0;
  C[1] = npoints/2;
  C[2] = npoints-1;
  n = 3;
  ret = scn_approxcurve_with_initial_control_points(
	   X, Y, npoints, deltamax, C, &n, 
           C0, C1, C2, C3, D0, D1, D2, D3);
  if (ret == 0)
  {
    fprintf(stderr, "%s: scn_approxcurve_with_initial_control_points failed\n", F_NAME);
    return 0;
  }

  for (i = 0; i < n; i++) 
  {
    X[i] = X[C[i]];
    Y[i] = Y[C[i]];
  }
  *N = n;
  free(C); 
  free(C0); free(C1); free(C2); free(C3); 
  free(D0); free(D1); free(D2); free(D3);
  return 1;
} // scn_approxcurve2()

/* ==================================== */
int32_t scn_approxcurve_with_initial_control_points(int32_t *X, int32_t *Y, int32_t N, double deltamax, int32_t *C, int32_t *n, double *C0, double *C1, double *C2, double *C3, double *D0, double *D1, double *D2, double *D3)
/* ==================================== */
/*! \fn int32_t scn_approxcurve_with_initial_control_points(int32_t *X, int32_t *Y, int32_t N, double deltamax, int32_t *C, int32_t *n, double *C0, double *C1, double *C2, double *C3, double *D0, double *D1, double *D2, double *D3)
    \param X (entr�e) : tableau des abscisses des points de la courbe (taille N)
    \param Y (entr�e) : tableau des ordonn�es des points de la courbe (taille N)
    \param N (entr�e) : nombre de points de la courbe
    \param deltamax (entr�e) : tol�rance sur l'approximation
    \param C (entr�e/sortie) : tableau des index des points de contr�le (taille N)
    \param n (entr�e/sortie) : nombre de points de contr�le
    \param C0 (sortie) : tableau des coef. spline X de degr� 0 (taille *n-1) 
    \param C1 (sortie) : tableau des coef. spline X de degr� 1 (taille *n-1) 
    \param C2 (sortie) : tableau des coef. spline X de degr� 2 (taille *n-1) 
    \param C3 (sortie) : tableau des coef. spline X de degr� 3 (taille *n-1) 
    \param D0 (sortie) : tableau des coef. spline Y de degr� 0 (taille *n-1) 
    \param D1 (sortie) : tableau des coef. spline Y de degr� 1 (taille *n-1) 
    \param D2 (sortie) : tableau des coef. spline Y de degr� 2 (taille *n-1) 
    \param D3 (sortie) : tableau des coef. spline Y de degr� 3 (taille *n-1) 
    \brief trouve une approximation par deux splines cubiques de la courbe discrete 
           d�finie par X, Y
    \warning la m�moire pour stocker les r�sultats C, C0..C3, D0..D3 doit avoir �t� allou�e
*/
#undef F_NAME
#define F_NAME "scn_approxcurve_with_initial_control_points"
{
  int32_t i, j, continuer;
  double *t, *x, *y;
  int32_t *V, *W;
  double delta; 
  int32_t arg;
  int32_t nctrl = *n;
  int32_t niter = 0;

  t = (double *)calloc(1,N * sizeof(double)); assert(t != NULL);
  x = (double *)calloc(1,N * sizeof(double)); assert(x != NULL);
  y = (double *)calloc(1,N * sizeof(double)); assert(y != NULL);
  V = (int32_t *)calloc(1,N * sizeof(int32_t)); assert(V != NULL);
  W = (int32_t *)calloc(1,N * sizeof(int32_t)); assert(W != NULL);

  if ((X[0] == X[N-1]) && (Y[0] == Y[N-1]))
  {
    fprintf(stderr, "%s: this operator does not allow closed curves\n", F_NAME);
    return 0;
  }

  for (i = 0; i < N; i++) t[i] = (double)i;

  do
  {
    niter++;
#ifdef DEBUG_AC
    printf("%s debut: niter = %d ; nctrl=%d ; \n", F_NAME, niter, nctrl);  
#endif
    continuer = 0;
    for (i = 0; i < nctrl; i++) x[i] = C[i];

    for (i = 0; i < nctrl; i++) y[i] = X[C[i]];
    if (scn_solvespline(x, y, nctrl, C0, C1, C2, C3) == 0) return 0;
    for (i = 0; i < nctrl; i++) y[i] = Y[C[i]];
    if (scn_solvespline(x, y, nctrl, D0, D1, D2, D3) == 0) return 0;
    scn_discretisespline(nctrl, C0, C1, C2, C3, D0, D1, D2, D3, C, V, W);
#ifdef DEBUG_AC
    { 
    char b[100]; 
    sprintf(b, "_C%02d", niter);
    printctrlpoints(C, X, Y, nctrl, b);
    sprintf(b, "_S%02d", niter);
    printspline2d(C, X, Y, nctrl, b, C0, C1, C2, C3, D0, D1, D2, D3);
    sprintf(b, "_W%02d", niter);
    printcurve(V, W, N, b);
    }
#endif
    i = 0;
    while (i < nctrl-1)
    {
      distancesegments(X, Y, V, W, C[i], C[i+1], &delta, &arg);
      if (delta > deltamax)
      { // insere le point arg dans C � la position i+1 
        continuer = 1;
        nctrl++;
        for (j = nctrl-1; j > i+1; j--) C[j] = C[j-1];
        C[i+1] = (C[i]+C[i+2])/2;
        i++;
      }
      i++;      
    }
#ifdef DEBUG_AC
    printf("%s fin: niter = %d ; nctrl=%d ; \n", F_NAME, niter, nctrl);  
#endif
  } while(continuer && (nctrl <= N));

  if (nctrl > N)
  {
    printf("%s: WARNING too many control points, tolerance may be too small\n", F_NAME);  
  }

#ifdef PARANO
  {
    double f[4]; double x1, x2, y1, y2;
    for(i = 0; i < nctrl-1; i++)
    {
      f[0] = C0[i]; f[1] = C1[i]; f[2] = C2[i]; f[3] = C3[i];
      x1 = evalpoly(4, (double *)f, (double)C[i]);
      x2 = evalpoly(4, (double *)f, (double)C[i+1]);
      f[0] = D0[i]; f[1] = D1[i]; f[2] = D2[i]; f[3] = D3[i];
      y1 = evalpoly(4, (double *)f, (double)C[i]);
      y2 = evalpoly(4, (double *)f, (double)C[i+1]);
      printf("normalement: %g = %d = X[Ci]; %g = %d = X[Ci+1]; %g = %d = Y[Ci]; %g = %d = Y[Ci+1]\n",
	     x1, X[C[i]], x2, X[C[i+1]], y1, Y[C[i]], y2, Y[C[i+1]]);
    }    
  }
#endif

  // renormalisation de l'abcisse curviligne
  // (d�part � 0, incr�ment de 1 pour chaque point de contr�le) 
  for (i = 0; i < nctrl; i++) x[i] = i;
  for (i = 0; i < nctrl; i++) y[i] = X[C[i]];
  if (scn_solvespline(x, y, nctrl, C0, C1, C2, C3) == 0) return 0;
  for (i = 0; i < nctrl; i++) y[i] = Y[C[i]];
  if (scn_solvespline(x, y, nctrl, D0, D1, D2, D3) == 0) return 0;

  *n = nctrl;
  free(t);
  free(x);
  free(y);
  free(W);
  free(V);
  return 1;
} // scn_approxcurve_with_initial_control_points()

/* ==================================== */
int32_t scn_approxcurve3d(int32_t *X, int32_t *Y, int32_t *Z, int32_t N, double deltamax, 
                       int32_t *C, int32_t *n, 
                       double *C0, double *C1, double *C2, double *C3,
                       double *D0, double *D1, double *D2, double *D3,
                       double *E0, double *E1, double *E2, double *E3)
/* ==================================== */
/*! \fn int32_t scn_approxcurve3d(int32_t *X, int32_t *Y, int32_t *Z, int32_t N, double deltamax, 
                       int32_t *C, int32_t *n, 
                       double *C0, double *C1, double *C2, double *C3,
                       double *D0, double *D1, double *D2, double *D3,
                       double *E0, double *E1, double *E2, double *E3)
    \param X (entr�e) : tableau des abscisses des points de la courbe (taille N)
    \param Y (entr�e) : tableau des ordonn�es des points de la courbe (taille N)
    \param Z (entr�e) : tableau des cotes des points de la courbe (taille N)
    \param N (entr�e) : nombre de points de la courbe
    \param deltamax (entr�e) : tol�rance sur l'approximation
    \param C (sortie) : tableau des index des points de contr�le (taille N)
    \param n (sortie) : nombre de points de contr�le
    \param C0 (sortie) : tableau des coef. spline X de degr� 0 (taille *n-1) 
    \param C1 (sortie) : tableau des coef. spline X de degr� 1 (taille *n-1) 
    \param C2 (sortie) : tableau des coef. spline X de degr� 2 (taille *n-1) 
    \param C3 (sortie) : tableau des coef. spline X de degr� 3 (taille *n-1) 
    \param D0 (sortie) : tableau des coef. spline Y de degr� 0 (taille *n-1) 
    \param D1 (sortie) : tableau des coef. spline Y de degr� 1 (taille *n-1) 
    \param D2 (sortie) : tableau des coef. spline Y de degr� 2 (taille *n-1) 
    \param D3 (sortie) : tableau des coef. spline Y de degr� 3 (taille *n-1) 
    \param E0 (sortie) : tableau des coef. spline Z de degr� 0 (taille *n-1) 
    \param E1 (sortie) : tableau des coef. spline Z de degr� 1 (taille *n-1) 
    \param E2 (sortie) : tableau des coef. spline Z de degr� 2 (taille *n-1) 
    \param E3 (sortie) : tableau des coef. spline Z de degr� 3 (taille *n-1) 
    \brief trouve une approximation par trois splines cubiques de la courbe discrete 
           d�finie par X, Y, Z
    \warning la m�moire pour stocker les r�sultats C, C0..C3, D0..D3, E0..E3 doit avoir �t� allou�e
*/
#undef F_NAME
#define F_NAME "scn_approxcurve3d"
{

  C[0] = 0;
  C[1] = N/2;
  C[2] = N-1;
  *n = 3;
  return scn_approxcurve3d_with_initial_control_points(X, Y, Z, N, deltamax, C, n, 
						       C0, C1, C2, C3,
						       D0, D1, D2, D3,
						       E0, E1, E2, E3);
} // scn_approxcurve3d()

/* ==================================== */
int32_t scn_approxcurve3d2(int32_t *X, int32_t *Y, int32_t *Z, int32_t *N, double deltamax)
/* ==================================== */
/*! \fn 
    \param X (entr�e/sortie) : tableau des abscisses des points (taille N)
    \param Y (entr�e/sortie) : tableau des ordonn�es des points (taille N)
    \param Z (entr�e/sortie) : tableau des cotes des points (taille N)
    \param N (entr�e/sortie) : nombre de points de la courbe
    \param deltamax (entr�e) : tol�rance sur l'approximation
    \brief trouve une approximation par deux splines cubiques de la courbe discrete d�finie par X, Y, Z. Le r�sultat (liste de points de contr�le) est retourn� dans X, Y, Z, N.
*/
#undef F_NAME
#define F_NAME "scn_approxcurve3d2"
{
  int32_t *C;
  int32_t ret, n, i, npoints = *N;
  double *C0, *C1, *C2, *C3, *D0, *D1, *D2, *D3, *E0, *E1, *E2, *E3;
  
  C = (int32_t *)calloc(1,npoints*sizeof(int32_t)); assert(C != NULL);
  C0 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(C0 != NULL);
  C1 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(C1 != NULL);
  C2 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(C2 != NULL);
  C3 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(C3 != NULL);
  D0 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(D0 != NULL);
  D1 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(D1 != NULL);
  D2 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(D2 != NULL);
  D3 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(D3 != NULL);
  E0 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(E0 != NULL);
  E1 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(E1 != NULL);
  E2 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(E2 != NULL);
  E3 = (double *)calloc(1,(npoints-1)*sizeof(double)); assert(E3 != NULL);
  
  C[0] = 0;
  C[1] = npoints/2;
  C[2] = npoints-1;
  n = 3;
  ret = scn_approxcurve3d_with_initial_control_points(
           X, Y, Z, npoints, deltamax, C, &n, 
           C0, C1, C2, C3, D0, D1, D2, D3, E0, E1, E2, E3);
  if (ret == 0)
  {
    fprintf(stderr, "%s: scn_approxcurve3d_with_initial_control_points failed\n", F_NAME);
    return 0;
  }

  for (i = 0; i < n; i++) 
  {
    X[i] = X[C[i]];
    Y[i] = Y[C[i]];
    Z[i] = Z[C[i]];
  }
  *N = n;
  free(C); 
  free(C0); free(C1); free(C2); free(C3); 
  free(D0); free(D1); free(D2); free(D3);
  free(E0); free(E1); free(E2); free(E3);
  return 1;
} // scn_approxcurve3d2()

/* ==================================== */ 
int32_t scn_approxcurve3d_with_initial_control_points(
  int32_t *X, int32_t *Y, int32_t *Z, int32_t N, double deltamax, 
  int32_t *C, int32_t *n, 
  double *C0, double *C1, double *C2, double *C3,
  double *D0, double *D1, double *D2, double *D3,
  double *E0, double *E1, double *E2, double *E3)
/* ==================================== */
/*! \fn int32_t scn_approxcurve3d_with_initial_control_points(
  int32_t *X, int32_t *Y, int32_t *Z, int32_t N, double deltamax, 
  int32_t *C, int32_t *n, 
  double *C0, double *C1, double *C2, double *C3,
  double *D0, double *D1, double *D2, double *D3,
  double *E0, double *E1, double *E2, double *E3)
    \param X (entr�e) : tableau des abscisses des points de la courbe (taille N)
    \param Y (entr�e) : tableau des ordonn�es des points de la courbe (taille N)
    \param Z (entr�e) : tableau des cotes des points de la courbe (taille N)
    \param N (entr�e) : nombre de points de la courbe
    \param deltamax (entr�e) : tol�rance sur l'approximation
    \param C (entr�e/sortie) : tableau des index des points de contr�le (taille N)
    \param n (entr�e/sortie) : nombre de points de contr�le
    \param C0 (sortie) : tableau des coef. spline X de degr� 0 (taille *n-1) 
    \param C1 (sortie) : tableau des coef. spline X de degr� 1 (taille *n-1) 
    \param C2 (sortie) : tableau des coef. spline X de degr� 2 (taille *n-1) 
    \param C3 (sortie) : tableau des coef. spline X de degr� 3 (taille *n-1) 
    \param D0 (sortie) : tableau des coef. spline Y de degr� 0 (taille *n-1) 
    \param D1 (sortie) : tableau des coef. spline Y de degr� 1 (taille *n-1) 
    \param D2 (sortie) : tableau des coef. spline Y de degr� 2 (taille *n-1) 
    \param D3 (sortie) : tableau des coef. spline Y de degr� 3 (taille *n-1) 
    \param E0 (sortie) : tableau des coef. spline Z de degr� 0 (taille *n-1) 
    \param E1 (sortie) : tableau des coef. spline Z de degr� 1 (taille *n-1) 
    \param E2 (sortie) : tableau des coef. spline Z de degr� 2 (taille *n-1) 
    \param E3 (sortie) : tableau des coef. spline Z de degr� 3 (taille *n-1) 
    \brief trouve une approximation par trois splines cubiques de la courbe discrete 
           d�finie par X, Y, Z
    \warning la m�moire pour stocker les r�sultats C, C0..C3, D0..D3, E0..E3 doit avoir �t� allou�e
*/
#undef F_NAME
#define F_NAME "scn_approxcurve3d_with_initial_control_points"
{
  int32_t i, j, continuer;
  double *t, *x, *y;
  int32_t *U, *V, *W;
  double delta; 
  int32_t arg;
  int32_t nctrl = *n;
  int32_t niter = 0;

  t = (double *)calloc(1,N * sizeof(double)); assert(t != NULL);
  x = (double *)calloc(1,N * sizeof(double)); assert(x != NULL);
  y = (double *)calloc(1,N * sizeof(double)); assert(y != NULL);
  U = (int32_t *)calloc(1,N * sizeof(int32_t)); assert(U != NULL);
  V = (int32_t *)calloc(1,N * sizeof(int32_t)); assert(V != NULL);
  W = (int32_t *)calloc(1,N * sizeof(int32_t)); assert(W != NULL);

  if ((X[0] == X[N-1]) && (Y[0] == Y[N-1]) && (Z[0] == Z[N-1]))
  {
    fprintf(stderr, "%s: this operator does not allow closed curves\n", F_NAME);
    return 0;
  }

  for (i = 0; i < N; i++) t[i] = (double)i;

  do
  {
    niter++;
    continuer = 0;
    for (i = 0; i < nctrl; i++) x[i] = C[i];
    for (i = 0; i < nctrl; i++) y[i] = X[C[i]];
    if (scn_solvespline(x, y, nctrl, C0, C1, C2, C3) == 0) return 0;
    for (i = 0; i < nctrl; i++) y[i] = Y[C[i]];
    if (scn_solvespline(x, y, nctrl, D0, D1, D2, D3) == 0) return 0;
    for (i = 0; i < nctrl; i++) y[i] = Z[C[i]];
    if (scn_solvespline(x, y, nctrl, E0, E1, E2, E3) == 0) return 0;
    scn_discretisespline3d(nctrl, C0, C1, C2, C3, D0, D1, D2, D3, E0, E1, E2, E3, C, U, V, W);
#ifdef DEBUG_approxcurve3d
    { 
    char b[100]; 
    sprintf(b, "_C%02d", niter);
    printctrlpoints3d(C, X, Y, Z, nctrl, b);
    sprintf(b, "_W%02d", niter);
    printcurve3d(U, V, W, N, b);
    }
#endif
    i = 0;
    while (i < nctrl-1)
    {
      distancesegments3d(X, Y, Z, U, V, W, C[i], C[i+1], &delta, &arg);
      if (delta > deltamax)
      { // insere le point arg dans C � la position i+1 
        continuer = 1;
        nctrl++;
        for (j = nctrl-1; j > i+1; j--) C[j] = C[j-1];
        C[i+1] = (C[i]+C[i+2])/2;
        i++;
      }
      i++;

#ifdef DEBUG_approxcurve3d
      printf("%s: nctrl=%d ; \n", F_NAME, nctrl);  
#endif
      
    }
  } while(continuer && (nctrl <= N));

  if (nctrl > N)
  {
    printf("%s: too many control points, tolerance may be too small\n", F_NAME);  
    return 0;
  }

  // renormalisation de l'abcisse curviligne
  // (d�part � 0, incr�ment de 1 pour chaque point de contr�le) 
  for (i = 0; i < nctrl; i++) x[i] = i;
  for (i = 0; i < nctrl; i++) y[i] = X[C[i]];
  if (scn_solvespline(x, y, nctrl, C0, C1, C2, C3) == 0) return 0;
  for (i = 0; i < nctrl; i++) y[i] = Y[C[i]];
  if (scn_solvespline(x, y, nctrl, D0, D1, D2, D3) == 0) return 0;
  for (i = 0; i < nctrl; i++) y[i] = Z[C[i]];
  if (scn_solvespline(x, y, nctrl, E0, E1, E2, E3) == 0) return 0;

  *n = nctrl;
  free(t);
  free(x);
  free(y);
  free(W);
  free(V);
  free(U);
  return 1;
} // scn_approxcurve3d_with_initial_control_points()

/* ==================================== */
double scn_splinequerypoint3d(double x, double y, double z, double p, int32_t n, 
			       double *C0, double *C1, double *C2, double *C3,
			       double *D0, double *D1, double *D2, double *D3,
			       double *E0, double *E1, double *E2, double *E3)
/* ==================================== */
/*! \fn double scn_splinequerypoint3d(double x, double y, double z, double p, int32_t n, 
			       double *C0, double *C1, double *C2, double *C3,
			       double *D0, double *D1, double *D2, double *D3,
			       double *E0, double *E1, double *E2, double *E3)
    \param x,y,z (entr�e) : un point
    \param p (entr�e) : pr�cision (valeur dont on incr�mente l'abs. curv. � chaque �tape) 
    \param n (entr�e) : nombre de points de contr�le de la spline
    \param C0 (entr�e) : tableau des coef. spline X de degr� 0 (taille n-1) 
    \param C1 (entr�e) : tableau des coef. spline X de degr� 1 (taille n-1) 
    \param C2 (entr�e) : tableau des coef. spline X de degr� 2 (taille n-1) 
    \param C3 (entr�e) : tableau des coef. spline X de degr� 3 (taille n-1) 
    \param D0 (entr�e) : tableau des coef. spline Y de degr� 0 (taille n-1) 
    \param D1 (entr�e) : tableau des coef. spline Y de degr� 1 (taille n-1) 
    \param D2 (entr�e) : tableau des coef. spline Y de degr� 2 (taille n-1) 
    \param D3 (entr�e) : tableau des coef. spline Y de degr� 3 (taille n-1) 
    \param E0 (entr�e) : tableau des coef. spline Z de degr� 0 (taille n-1) 
    \param E1 (entr�e) : tableau des coef. spline Z de degr� 1 (taille n-1) 
    \param E2 (entr�e) : tableau des coef. spline Z de degr� 2 (taille n-1) 
    \param E3 (entr�e) : tableau des coef. spline Z de degr� 3 (taille n-1) 
    \brief retourne l'abcisse curviligne du point de la spline le plus proche du point (x,y,z) donn�
*/
#undef F_NAME
#define F_NAME "scn_splinequerypoint3d"
{
  double s, smin, d, dmin, X, Y, Z;
  double f[4];
  int32_t i;
  
  dmin = dist3(x, y, z, C0[0], D0[0], E0[0]);
  for (smin = s = 0.0, i = 0; s <= (double)(n-1); s += p)
  {
    if (s > (double)(i+1)) i++;
    f[0] = C0[i]; f[1] = C1[i]; f[2] = C2[i]; f[3] = C3[i];
    X = evalpoly(4, (double *)f, s);
    f[0] = D0[i]; f[1] = D1[i]; f[2] = D2[i]; f[3] = D3[i];
    Y = evalpoly(4, (double *)f, s);
    f[0] = E0[i]; f[1] = E1[i]; f[2] = E2[i]; f[3] = E3[i];
    Z = evalpoly(4, (double *)f, s);
    d = dist3(x, y, z, X, Y, Z);
    if (d < dmin) { dmin = d; smin = s; }
  }
  return smin;
} // scn_splinequerypoint3d()

/* ==================================== */
double scn_splinequerycurvature3d(double s, int32_t n,
			       double *C0, double *C1, double *C2, double *C3,
			       double *D0, double *D1, double *D2, double *D3,
			       double *E0, double *E1, double *E2, double *E3)
/* ==================================== */
/*! \fn double scn_splinequerycurvature3d(double s, int32_t n,
			       double *C0, double *C1, double *C2, double *C3,
			       double *D0, double *D1, double *D2, double *D3,
			       double *E0, double *E1, double *E2, double *E3)
    \param s (entr�e) : abcisse curviligne d'un point de la spline
    \param n (entr�e) : nombre de points de contr�le de la spline
    \param C0 (entr�e) : tableau des coef. spline X de degr� 0 (taille n-1) 
    \param C1 (entr�e) : tableau des coef. spline X de degr� 1 (taille n-1) 
    \param C2 (entr�e) : tableau des coef. spline X de degr� 2 (taille n-1) 
    \param C3 (entr�e) : tableau des coef. spline X de degr� 3 (taille n-1) 
    \param D0 (entr�e) : tableau des coef. spline Y de degr� 0 (taille n-1) 
    \param D1 (entr�e) : tableau des coef. spline Y de degr� 1 (taille n-1) 
    \param D2 (entr�e) : tableau des coef. spline Y de degr� 2 (taille n-1) 
    \param D3 (entr�e) : tableau des coef. spline Y de degr� 3 (taille n-1) 
    \param E0 (entr�e) : tableau des coef. spline Z de degr� 0 (taille n-1) 
    \param E1 (entr�e) : tableau des coef. spline Z de degr� 1 (taille n-1) 
    \param E2 (entr�e) : tableau des coef. spline Z de degr� 2 (taille n-1) 
    \param E3 (entr�e) : tableau des coef. spline Z de degr� 3 (taille n-1) 
    \brief retourne la courbure de la spline au point d'abcisse s
*/
#undef F_NAME
#define F_NAME "scn_splinequerycurvature3d"
{
  double xp, yp, zp; // d�riv�es premi�res
  double xs, ys, zs; // d�riv�es secondes
  double f[3], tmp1, tmp2;
  int32_t j;
  
  if (s < n-1) j = (int32_t)floor(s); else j = n-2;
  
  f[0] = C1[j]; f[1] = 2*C2[j]; f[2] = 3*C3[j];
  xp = evalpoly(3, (double *)f, s);
  f[0] = D1[j]; f[1] = 2*D2[j]; f[2] = 3*D3[j];
  yp = evalpoly(3, (double *)f, s);
  f[0] = E1[j]; f[1] = 2*E2[j]; f[2] = 3*E3[j];
  zp = evalpoly(3, (double *)f, s);

  f[0] = 2*C2[j]; f[1] = 6*C3[j]; f[2] = 0;
  xs = evalpoly(3, (double *)f, s);
  f[0] = 2*D2[j]; f[1] = 6*D3[j]; f[2] = 0;
  ys = evalpoly(3, (double *)f, s);
  f[0] = 2*E2[j]; f[1] = 6*E3[j]; f[2] = 0;
  zs = evalpoly(3, (double *)f, s);

  tmp1 = sqrt(xp*xp + yp*yp + zp*zp);
  tmp1 = tmp1 * tmp1 * tmp1;
  tmp2 = (zs*yp - ys*zp) * (zs*yp - ys*zp);
  tmp2 = tmp2 + (xs*zp - zs*xp) * (xs*zp - zs*xp);
  tmp2 = tmp2 + (ys*xp - xs*yp) * (ys*xp - xs*yp);
  tmp2 = sqrt(tmp2);

  //  printf("%s : %g / %g\n", F_NAME, tmp2, tmp1);
  
  if (tmp2 < SCN_EPSILON2) return 0.0;
  return tmp2 / tmp1;
} // scn_splinequerycurvature3d()

// =================================================
// CONVERT SPLINES INTO DISCRETE 'CURVES'
// =================================================

/* ==================================== */
void scn_drawspline(struct xvimage * image, double *x, double *y, int32_t nctrl)
/* ==================================== */
/*! \fn void scn_drawspline(struct xvimage * image, double *x, double *y, int32_t nctrl)
    \param image (entr�e/sortie)
    \param x (entr�e) : abcisses des points de contr�le de la spline
    \param y (entr�e) : ordonn�es des points de contr�le de la spline
    \param nctrl (entr�e) : nombre de points de contr�le de la spline
    \brief dessine la spline de points de contr�le donn�s dans 'image'
    \warning le r�sultat n'est pas forc�ment une courbe discr�te
*/
#undef F_NAME
#define F_NAME "scn_drawspline"
{
  double Px[4], Py[4];
  double *X0, *X1, *X2, *X3, *Y0, *Y1, *Y2, *Y3, *t;
  int32_t j;

  t = (double *)calloc(1,nctrl*sizeof(double)); assert(t != NULL);
  X0 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X0 != NULL);
  X1 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X1 != NULL);
  X2 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X2 != NULL);
  X3 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X3 != NULL);
  Y0 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y0 != NULL);
  Y1 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y1 != NULL);
  Y2 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y2 != NULL);
  Y3 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y3 != NULL);

  for (j = 0; j < nctrl; j++) t[j] = (double)j;

  if (nctrl == 2)
  {
    X0[0] = x[0]; X1[0] = x[1] - x[0]; X2[0] = X3[0] = 0.0; 
    Y0[0] = y[0]; Y1[0] = y[1] - y[0]; Y2[0] = Y3[0] = 0.0; 
  }
  else
  {
    if ((x[0] != x[nctrl-1]) || (y[0] != y[nctrl-1]))
    {
      scn_solvespline(t, x, nctrl, X0, X1, X2, X3);
      scn_solvespline(t, y, nctrl, Y0, Y1, Y2, Y3);
    }
    else
    {
      scn_solveclosedspline(t, x, nctrl, X0, X1, X2, X3);
      scn_solveclosedspline(t, y, nctrl, Y0, Y1, Y2, Y3);
    }
  }    
    
  for (j = 0; j < nctrl-1; j++)
  {
    Px[0] = X0[j]; Px[1] = X1[j]; Px[2] = X2[j]; Px[3] = X3[j];
    Py[0] = Y0[j]; Py[1] = Y1[j]; Py[2] = Y2[j]; Py[3] = Y3[j];
    ldrawcubic2(image, (double *)Px, (double *)Py, 10, t[j], t[j+1]);
  }
  free(t);
  free(X0); free(X1); free(X2); free(X3);
  free(Y0); free(Y1); free(Y2); free(Y3);
} // scn_drawspline()

/* ==================================== */
void scn_drawsplinelist(int32_t *lx, int32_t *ly, int32_t *npoints, double *x, double *y, int32_t nctrl)
/* ==================================== */
/*! \fn void scn_drawsplinelist(int32_t *lx, int32_t *ly, int32_t *npoints, double *x, double *y, int32_t nctrl)
    \param lx (sortie) : liste des abcisses des points dessin�s
    \param ly (sortie) : liste des ordonn�es des points dessin�s
    \param npoints (entr�e) : taille des tableaux lx, ly (sortie) : nombre de points dessin�s
    \param x (entr�e) : abcisses des points de contr�le de la spline
    \param y (entr�e) : ordonn�es des points de contr�le de la spline
    \param nctrl (entr�e) : nombre de points de contr�le de la spline
    \brief dessine la spline de points de contr�le donn�s dans 'image'
    \warning les tableaux lx, ly doivent avoir �t� allou�s
    \warning le r�sultat n'est pas forc�ment une courbe discr�te
*/
#undef F_NAME
#define F_NAME "scn_drawsplinelist"
{
  double Px[4], Py[4];
  double *X0, *X1, *X2, *X3, *Y0, *Y1, *Y2, *Y3, *t;
  int32_t j, nmaxpoints = *npoints, np, npp;

  t = (double *)calloc(1,nctrl*sizeof(double)); assert(t != NULL);
  X0 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X0 != NULL);
  X1 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X1 != NULL);
  X2 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X2 != NULL);
  X3 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X3 != NULL);
  Y0 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y0 != NULL);
  Y1 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y1 != NULL);
  Y2 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y2 != NULL);
  Y3 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y3 != NULL);

  for (j = 0; j < nctrl; j++) t[j] = (double)j;

  if (nctrl == 2)
  {
    X0[0] = x[0]; X1[0] = x[1] - x[0]; X2[0] = X3[0] = 0.0; 
    Y0[0] = y[0]; Y1[0] = y[1] - y[0]; Y2[0] = Y3[0] = 0.0; 
  }
  else
  {
    if ((x[0] != x[nctrl-1]) || (y[0] != y[nctrl-1]))
    {
      scn_solvespline(t, x, nctrl, X0, X1, X2, X3);
      scn_solvespline(t, y, nctrl, Y0, Y1, Y2, Y3);
    }
    else
    {
      scn_solveclosedspline(t, x, nctrl, X0, X1, X2, X3);
      scn_solveclosedspline(t, y, nctrl, Y0, Y1, Y2, Y3);
    }
  }    

  np = 0;    
#ifdef DEBUG
printf("nctrl=%d\n", nctrl);
#endif
  for (j = 0; j < nctrl-1; j++)
  {
    Px[0] = X0[j]; Px[1] = X1[j]; Px[2] = X2[j]; Px[3] = X3[j];
    Py[0] = Y0[j]; Py[1] = Y1[j]; Py[2] = Y2[j]; Py[3] = Y3[j];
    npp = nmaxpoints - np;
#ifdef DEBUG
printf("ldrawcubic2list np=%d ;  npp=%d\n", np, npp);
#endif
    ldrawcubic2list(lx+np, ly+np, &npp, (double *)Px, (double *)Py, 10, t[j], t[j+1]);
    np += npp;
#ifdef DEBUG
printf("j=%d ; np=%d ; npp=%d\n", j, np, npp);
#endif
  }
  *npoints = np;

  free(t);
  free(X0); free(X1); free(X2); free(X3);
  free(Y0); free(Y1); free(Y2); free(Y3);

} // scn_drawsplinelist()

/* ==================================== */
void scn_drawspline3d(struct xvimage * image, double *x, double *y, double *z, int32_t nctrl)
/* ==================================== */
/*! \fn void scn_drawspline3d(struct xvimage * image, double *x, double *y, double *z, int32_t nctrl)
    \param image (entr�e/sortie)
    \param x (entr�e) : abcisses des points de contr�le de la spline
    \param y (entr�e) : ordonn�es des points de contr�le de la spline
    \param z (entr�e) : cotes des points de contr�le de la spline
    \param nctrl (entr�e) : nombre de points de contr�le de la spline
    \brief dessine la spline de points de contr�le donn�s dans 'image'
*/
#undef F_NAME
#define F_NAME "scn_drawspline3d"
{
  double Px[4], Py[4], Pz[4];
  double *X0, *X1, *X2, *X3, *Y0, *Y1, *Y2, *Y3, *Z0, *Z1, *Z2, *Z3, *t;
  int32_t j;

  t = (double *)calloc(1,nctrl*sizeof(double)); assert(t != NULL);
  X0 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X0 != NULL);
  X1 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X1 != NULL);
  X2 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X2 != NULL);
  X3 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X3 != NULL);
  Y0 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y0 != NULL);
  Y1 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y1 != NULL);
  Y2 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y2 != NULL);
  Y3 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y3 != NULL);
  Z0 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Z0 != NULL);
  Z1 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Z1 != NULL);
  Z2 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Z2 != NULL);
  Z3 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Z3 != NULL);

  for (j = 0; j < nctrl; j++) t[j] = (double)j;

  if (nctrl == 2)
  {
    X0[0] = x[0]; X1[0] = x[1] - x[0]; X2[0] = X3[0] = 0.0; 
    Y0[0] = y[0]; Y1[0] = y[1] - y[0]; Y2[0] = Y3[0] = 0.0; 
    Z0[0] = z[0]; Z1[0] = z[1] - z[0]; Z2[0] = Z3[0] = 0.0; 
  }
  else
  {
    if ((x[0] != x[nctrl-1]) || (y[0] != y[nctrl-1]) || (z[0] != z[nctrl-1]))
    {
      scn_solvespline(t, x, nctrl, X0, X1, X2, X3);
      scn_solvespline(t, y, nctrl, Y0, Y1, Y2, Y3);
      scn_solvespline(t, z, nctrl, Z0, Z1, Z2, Z3);
    }
    else
    {
      scn_solveclosedspline(t, x, nctrl, X0, X1, X2, X3);
      scn_solveclosedspline(t, y, nctrl, Y0, Y1, Y2, Y3);
      scn_solveclosedspline(t, z, nctrl, Z0, Z1, Z2, Z3);
    }
  }    
    
  for (j = 0; j < nctrl-1; j++)
  {
    Px[0] = X0[j]; Px[1] = X1[j]; Px[2] = X2[j]; Px[3] = X3[j];
    Py[0] = Y0[j]; Py[1] = Y1[j]; Py[2] = Y2[j]; Py[3] = Y3[j];
    Pz[0] = Z0[j]; Pz[1] = Z1[j]; Pz[2] = Z2[j]; Pz[3] = Z3[j];
    ldrawcubic3d(image, (double *)Px, (double *)Py, (double *)Pz, 10, t[j], t[j+1]);
  }

  free(t);
  free(X0); free(X1); free(X2); free(X3);
  free(Y0); free(Y1); free(Y2); free(Y3);
  free(Z0); free(Z1); free(Z2); free(Z3);
} // scn_drawspline3d()

/* ==================================== */
void scn_drawspline3dlist(int32_t *lx, int32_t *ly, int32_t *lz, int32_t *npoints, double *x, double *y, double *z, int32_t nctrl)
/* ==================================== */
/*! \fn void scn_drawspline3dlist(int32_t *lx, int32_t *ly, int32_t *lz, int32_t *npoints, double *x, double *y, double *z, int32_t nctrl)
    \param lx (sortie) : liste des abcisses des points dessin�s
    \param ly (sortie) : liste des ordonn�es des points dessin�s
    \param lz (sortie) : liste des cotes des points dessin�s
    \param npoints (entr�e) : taille des tableaux lx, ly, lz (sortie) : nombre de points dessin�s
    \param x (entr�e) : abcisses des points de contr�le de la spline
    \param y (entr�e) : ordonn�es des points de contr�le de la spline
    \param z (entr�e) : cotes des points de contr�le de la spline
    \param nctrl (entr�e) : nombre de points de contr�le de la spline
    \brief dessine la spline de points de contr�le donn�s dans 'image'
    \warning les tableaux lx, ly, lz doivent avoir �t� allou�s
    \warning le r�sultat n'est pas forc�ment une courbe discr�te
*/
#undef F_NAME
#define F_NAME "scn_drawspline3dlist"
{
  double Px[4], Py[4], Pz[4];
  double *X0, *X1, *X2, *X3, *Y0, *Y1, *Y2, *Y3, *Z0, *Z1, *Z2, *Z3, *t;
  int32_t j, nmaxpoints = *npoints, np = 0, npp;

  t = (double *)calloc(1,nctrl*sizeof(double)); assert(t != NULL);
  X0 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X0 != NULL);
  X1 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X1 != NULL);
  X2 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X2 != NULL);
  X3 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(X3 != NULL);
  Y0 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y0 != NULL);
  Y1 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y1 != NULL);
  Y2 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y2 != NULL);
  Y3 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Y3 != NULL);
  Z0 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Z0 != NULL);
  Z1 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Z1 != NULL);
  Z2 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Z2 != NULL);
  Z3 = (double *)calloc(1,(nctrl-1)*sizeof(double)); assert(Z3 != NULL);

  for (j = 0; j < nctrl; j++) t[j] = (double)j;

  if (nctrl == 2)
  {
    X0[0] = x[0]; X1[0] = x[1] - x[0]; X2[0] = X3[0] = 0.0; 
    Y0[0] = y[0]; Y1[0] = y[1] - y[0]; Y2[0] = Y3[0] = 0.0; 
    Z0[0] = z[0]; Z1[0] = z[1] - z[0]; Z2[0] = Z3[0] = 0.0; 
  }
  else
  {
    if ((x[0] != x[nctrl-1]) || (y[0] != y[nctrl-1]) || (z[0] != z[nctrl-1]))
    {
      scn_solvespline(t, x, nctrl, X0, X1, X2, X3);
      scn_solvespline(t, y, nctrl, Y0, Y1, Y2, Y3);
      scn_solvespline(t, z, nctrl, Z0, Z1, Z2, Z3);
    }
    else
    {
      scn_solveclosedspline(t, x, nctrl, X0, X1, X2, X3);
      scn_solveclosedspline(t, y, nctrl, Y0, Y1, Y2, Y3);
      scn_solveclosedspline(t, z, nctrl, Z0, Z1, Z2, Z3);
    }
  }    
    
  for (j = 0; j < nctrl-1; j++)
  {
    Px[0] = X0[j]; Px[1] = X1[j]; Px[2] = X2[j]; Px[3] = X3[j];
    Py[0] = Y0[j]; Py[1] = Y1[j]; Py[2] = Y2[j]; Py[3] = Y3[j];
    Pz[0] = Z0[j]; Pz[1] = Z1[j]; Pz[2] = Z2[j]; Pz[3] = Z3[j];
    npp = nmaxpoints - np;
    ldrawcubic3dlist(lx+np, ly+np, lz+np, &npp, (double *)Px, (double *)Py, (double *)Pz, 10, t[j], t[j+1]);
    np += npp;
  }
  *npoints = np;
  free(t);
  free(X0); free(X1); free(X2); free(X3);
  free(Y0); free(Y1); free(Y2); free(Y3);
  free(Z0); free(Z1); free(Z2); free(Z3);
} // scn_drawspline3dlist()

// =================================================
// TESTS
// =================================================

#ifdef TEST1
int32_t main()
{
  const int32_t n = 6;
  const int32_t m = 100;
  int32_t k;
  double x[6] = {1, 3, 4, 7, 11, 13};
  double y[6] = {4, 10, 8, 5, 5, 7};
  double sk[m];
  double rhok[m];

  scn_curvatures((double *)x, (double *)y, n, m, (double *)sk, (double *)rhok);

  for (k=0; k < m; k++)
    printf("k=%d ; sk= %lf ; rhok = %lf\n", k, sk[k], rhok[k]);

  return 0;
}
#endif

#ifdef TEST2
int32_t main()
{
  const int32_t n = 6;
  int32_t i;
  double X[6] = {1, 3, 4, 7, 11, 13};
  double Y[6] = {4, 10, 8, 5, 5, 7};
  double Z[6] = {1, 2, 3, 2, 1, 0};
  double *t;
  double *C0, *C1, *C2, *C3;
  double *D0, *D1, *D2, *D3;
  double *E0, *E1, *E2, *E3;
  double s;

  t = (double *)calloc(1,n*sizeof(double));
  C0 = (double *)calloc(1,(n-1)*sizeof(double));
  C1 = (double *)calloc(1,(n-1)*sizeof(double));
  C2 = (double *)calloc(1,(n-1)*sizeof(double));
  C3 = (double *)calloc(1,(n-1)*sizeof(double));
  D0 = (double *)calloc(1,(n-1)*sizeof(double));
  D1 = (double *)calloc(1,(n-1)*sizeof(double));
  D2 = (double *)calloc(1,(n-1)*sizeof(double));
  D3 = (double *)calloc(1,(n-1)*sizeof(double));
  E0 = (double *)calloc(1,(n-1)*sizeof(double));
  E1 = (double *)calloc(1,(n-1)*sizeof(double));
  E2 = (double *)calloc(1,(n-1)*sizeof(double));
  E3 = (double *)calloc(1,(n-1)*sizeof(double));

  for (i = 0; i < n; i++) t[i] = (double)i; 

  scn_solvespline(t, X, n, C0, C1, C2, C3);
  scn_solvespline(t, Y, n, D0, D1, D2, D3);
  scn_solvespline(t, Z, n, E0, E1, E2, E3);

  for (i = 0; i < n; i++) 
  {
    s = scn_splinequerypoint3d(X[i]+0.5, Y[i]+1.5, Z[i]+0.5, 0.1, n, 
			       C0, C1, C2, C3,
			       D0, D1, D2, D3,
			       E0, E1, E2, E3);
    printf("%d %g\n", i, s);
  }

  return 0;
}
#endif

#ifdef TEST3
int32_t main()
{
  const int32_t n = 7;
  int32_t i;
  double X[7] = {1, 1, 1, 10, 2, 2, 2};
  double Y[7] = {1, 1, 1, 10, 2, 2, 2};
  double Z[7] = {1, 2, 3, 4, 3, 2, 1};
  double *t;
  double *C0, *C1, *C2, *C3;
  double *D0, *D1, *D2, *D3;
  double *E0, *E1, *E2, *E3;
  double c;

  t = (double *)malloc(n * sizeof(double));
  C0 = (double *)malloc(12 * (n-1) * sizeof(double));
  C1 = C0 + n - 1; C2 = C1 + n - 1; C3 = C2 + n - 1;
  D0 = C3 + n - 1; D1 = D0 + n - 1; D2 = D1 + n - 1; D3 = D2 + n - 1;
  E0 = D3 + n - 1; E1 = E0 + n - 1; E2 = E1 + n - 1; E3 = E2 + n - 1;

  for (i = 0; i < n; i++) t[i] = (double)i; 

  scn_solvespline(t, X, n, C0, C1, C2, C3);
  scn_solvespline(t, Y, n, D0, D1, D2, D3);
  scn_solvespline(t, Z, n, E0, E1, E2, E3);

  for (i = 0; i < n; i++) 
  {
    c = scn_splinequerycurvature3d((double)i, n, 
			       C0, C1, C2, C3,
			       D0, D1, D2, D3,
			       E0, E1, E2, E3);
    printf("%d %g\n", i, c);
  }

  free(C0);
  return 0;
}
#endif




