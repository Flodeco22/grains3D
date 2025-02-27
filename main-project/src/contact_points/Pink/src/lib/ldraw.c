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
/* graphic primitives */
/* Michel Couprie - octobre 2002 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <mcutil.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <mclin.h>
#include <lbresen.h>
#include <ldraw.h>

//#define DEBUG_DL3
//#define DEBUG_DC3

/* ==================================== */
static double dist3(double x1, double y1, double z1, double x2, double y2, double z2)
/* ==================================== */
{
  return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
}

/* ==================================== */
int32_t ldrawline(struct xvimage * image1, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
/* ==================================== */
/* draws a straight line segment between two points */
#undef F_NAME
#define F_NAME "ldrawline"
{
  int32_t rs, cs;
  uint8_t *F;

  rs = rowsize(image1);
  cs = colsize(image1);
  F = UCHARDATA(image1);

  if ((x1 < 0) || (x1 >= rs) ||  (y1 < 0) || (y1 >= cs) ||
      (x2 < 0) || (x2 >= rs) ||  (y2 < 0) || (y2 >= cs)) 
    return 1; // do nothing

  lbresen(F, rs, x1, y1, x2, y2);

  return 1;
} // ldrawline()

static void swap(int32_t *a, int32_t *b)
{
  int32_t t = *a;
  *a = *b;
  *b = t;
}

/* ==================================== */
int32_t ldrawhorline(struct xvimage * image1, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
/* ==================================== */
/* draws a horizontal straight line segment between two points */
#undef F_NAME
#define F_NAME "ldrawhorline"
{
  int32_t rs, cs;
  uint8_t *F;
  int x;

  rs = rowsize(image1);
  cs = colsize(image1);
  F = UCHARDATA(image1);

  assert(y1 == y2);
  if (x1 > x2) swap(&x1, &x2);

  if ((y1 < 0) || (y1 >= cs) || (x1 >= rs) || (x2 < 0))
    return 1; // do nothing

  if (x1 < 0) x1 = 0;
  if (x2 >= rs) x2 = rs-1;

  for (x = x1; x <= x2; x++)
    F[y1*rs + x] = NDG_MAX;

  return 1;
} // ldrawline()

static fillBottomFlatTriangle(struct xvimage * image1, int32_t v1x, int32_t v1y, int32_t v2x, int32_t v2y, int32_t v3x, int32_t v3y)
{
  double invslope1 = (double)(v2x - v1x) / (double)(v2y - v1y);
  double invslope2 = (double)(v3x - v1x) / (double)(v3y - v1y);
  int y;

  double curx1 = v1x;
  double curx2 = v1x;

  for (y = v1y; y <= v2y; y++)
  {
    ldrawhorline(image1, (int32_t)floor(curx1), y, (int32_t)ceil(curx2), y);
    curx1 += invslope1;
    curx2 += invslope2;
  }
}

fillTopFlatTriangle(struct xvimage * image1, int32_t v1x, int32_t v1y, int32_t v2x, int32_t v2y, int32_t v3x, int32_t v3y)
{
  double invslope1 = (double)(v3x - v1x) / (double)(v3y - v1y);
  double invslope2 = (double)(v3x - v2x) / (double)(v3y - v2y);
  int y;

  double curx1 = v3x;
  double curx2 = v3x;

  for (y = v3y; y > v1y; y--)
  {
    ldrawhorline(image1, (int32_t)floor(curx1), y, (int32_t)ceil(curx2), y);
    curx1 -= invslope1;
    curx2 -= invslope2;
  }
}

/* ==================================== */
int32_t ldrawfilledtriangle(struct xvimage * image1, int32_t v1x, int32_t v1y, int32_t v2x, int32_t v2y, int32_t v3x, int32_t v3y)
/* ==================================== */
/* draws a filled triangle */
#undef F_NAME
#define F_NAME "ldrawfilledtriangle"
{
  int32_t v4x, v4y;

  /* at first sort the three vertices by y-coordinate ascending so v1 is the topmost vertice */
  if (v1y > v2y) { swap(&v1y, &v2y); swap(&v1x, &v2x); }
  if (v2y > v3y) { swap(&v2y, &v3y); swap(&v2x, &v3x); }
  if (v1y > v2y) { swap(&v1y, &v2y); swap(&v1x, &v2x); }

  /* here we know that v1y <= v2y <= v3y */
  /* check for trivial case of bottom-flat triangle */
  if (v2y == v3y)
  {
    fillBottomFlatTriangle(image1, v1x, v1y, v2x, v2y, v3x, v3y);
  }
  /* check for trivial case of top-flat triangle */
  else if (v1y == v2y)
  {
    fillTopFlatTriangle(image1, v1x, v1y, v2x, v2y, v3x, v3y);
  } 
  else
  {
    /* general case - split the triangle in a topflat and bottom-flat one */
    v4x = (int32_t)(v1x + ((double)(v2y - v1y) / (double)(v3y - v1y)) * (v3x - v1x));
    v4y = v2y;
    fillBottomFlatTriangle(image1, v1x, v1y, v2x, v2y, v4x, v4y);
    fillTopFlatTriangle(image1, v2x, v2y, v4x, v4y, v3x, v3y);
  }
} // ldrawfilledtriangle()

/* ==================================== */
int32_t ldrawfilledquadrangle(struct xvimage * image1, int32_t v1x, int32_t v1y, int32_t v2x, int32_t v2y, int32_t v3x, int32_t v3y, int32_t v4x, int32_t v4y)
/* ==================================== */
/* draws a filled quadrangle */
#undef F_NAME
#define F_NAME "ldrawfilledquadrangle"
{
  ldrawfilledtriangle(image1, v1x, v1y, v2x, v2y, v3x, v3y);
  ldrawfilledtriangle(image1, v1x, v1y, v4x, v4y, v3x, v3y);
} // ldrawfilledquadrangle()

/* ==================================== */
int32_t ldrawlinelist(int32_t *lx, int32_t *ly, int32_t *npoints, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
/* ==================================== */
// draws a straight line segment between two points
// mere call to lbresenlist
// the result is put in the list represented by (lx, ly, npoints)
// the arrays lx, ly must be allocated with a size that is given initially by npoints
#undef F_NAME
#define F_NAME "ldrawlinelist"
{
  lbresenlist(x1, y1, x2, y2, lx, ly, npoints);
  return 1;
} // ldrawlinelist()

/* ==================================== */
int32_t ldrawline3d(struct xvimage * image1, int32_t x1, int32_t y1, int32_t z1, int32_t x2, int32_t y2, int32_t z2)
/* ==================================== */
/* draws a 3D straight line segment between two points */
/* NAIVE ALGORITHM - TO IMPROVE !!!! */
#undef F_NAME
#define F_NAME "ldrawline3d"
{
  int32_t i, rs, cs, ds, ps, x, y, z, NBSAMPLES;
  uint8_t *F;
  double len;

#ifdef DEBUG_DL3
  printf("%s: %d %d %d   %d %d %d\n", F_NAME, x1, y1, z1, x2, y2, z2);
#endif

  len =  dist3(x1, y1, z1, x2, y2, z2);
  NBSAMPLES = (int32_t)(10 * len);

  rs = rowsize(image1);
  cs = colsize(image1);
  ds = depth(image1);
  ps = rs * cs;
  F = UCHARDATA(image1);
    
  x = x1; y = y1; z = z1; 
  if (!((x<0) || (x>=rs) || (y<0) || (y>=cs) || (z<0) || (z>=ds)))
  {
    F[z*ps + y*rs + x] = NDG_MAX;
  }
  for (i = 1; i <= NBSAMPLES; i++)
  {
    x = x1 + (i * (x2 - x1)) / NBSAMPLES;
    y = y1 + (i * (y2 - y1)) / NBSAMPLES;
    z = z1 + (i * (z2 - z1)) / NBSAMPLES;
    if (!((x<0) || (x>=rs) || (y<0) || (y>=cs) || (z<0) || (z>=ds)))
    {
      F[z*ps + y*rs + x] = NDG_MAX;
    }
  }

  return 1;
} // ldrawline3d()

/* ==================================== */
int32_t ldrawline3d_val(struct xvimage * image1, int32_t x1, int32_t y1, int32_t z1, int32_t x2, int32_t y2, int32_t z2, uint8_t v)
/* ==================================== */
/* draws a 3D straight line segment between two points */
/* NAIVE ALGORITHM - TO IMPROVE !!!! */
#undef F_NAME
#define F_NAME "ldrawline3d"
{
  int32_t i, rs, cs, ds, ps, x, y, z, NBSAMPLES;
  uint8_t *F;
  double len;

  len =  dist3(x1, y1, z1, x2, y2, z2);
  NBSAMPLES = (int32_t)(10 * len);

  rs = rowsize(image1);
  cs = colsize(image1);
  ds = depth(image1);
  ps = rs * cs;
  F = UCHARDATA(image1);
    
  x = x1; y = y1; z = z1; 
  if (!((x<0) || (x>=rs) || (y<0) || (y>=cs) || (z<0) || (z>=ds)))
  {
    F[z*ps + y*rs + x] = v;
  }
  for (i = 1; i <= NBSAMPLES; i++)
  {
    x = x1 + (i * (x2 - x1)) / NBSAMPLES;
    y = y1 + (i * (y2 - y1)) / NBSAMPLES;
    z = z1 + (i * (z2 - z1)) / NBSAMPLES;
    if (!((x<0) || (x>=rs) || (y<0) || (y>=cs) || (z<0) || (z>=ds)))
    {
      F[z*ps + y*rs + x] = v;
    }
  }

  return 1;
} // ldrawline3d_val()

/* ==================================== */
int32_t ldrawline3dlist(int32_t *lx, int32_t *ly, int32_t *lz, int32_t *npoints, int32_t x1, int32_t y1, int32_t z1, int32_t x2, int32_t y2, int32_t z2)
/* ==================================== */
// draws a 3D straight line segment between two points
// the result is put in the list represented by (lx, ly, lz, npoints)
// the arrays lx, ly, lz must be allocated with a size that is given initially by npoints
// NAIVE ALGORITHM - TO IMPROVE !!!!
#undef F_NAME
#define F_NAME "ldrawline3dlist"
{
  int32_t i, nmaxpoints = *npoints, np = 0, x, y, z;
  double len =  dist3(x1, y1, z1, x2, y2, z2);
  int32_t NBSAMPLES = (int32_t)(10 * len);
  if (NBSAMPLES > nmaxpoints-1) NBSAMPLES = nmaxpoints-1;

#ifdef DEBUG_DL3
  printf("%s: %d %d %d   %d %d %d\n", F_NAME, x1, y1, z1, x2, y2, z2);
#endif
    
  lx[0] = x1;
  ly[0] = y1;
  lz[0] = z1;
  for (np = i = 1; i <= NBSAMPLES; i++)
  {
    x = x1 + (i * (x2 - x1)) / NBSAMPLES;
    y = y1 + (i * (y2 - y1)) / NBSAMPLES;
    z = z1 + (i * (z2 - z1)) / NBSAMPLES;
    if ((x != lx[np-1]) || (y != ly[np-1]) || (z != lz[np-1]))
    { lx[np] = x; ly[np] = y; lz[np] = z; np++; }
  }
  *npoints = np;
  return 1;
} // ldrawline3dlist()

/* ==================================== */
int32_t ldrawline2(struct xvimage * image1)
/* ==================================== */
/* draws a straight line segment between the two first points found in image */
#undef F_NAME
#define F_NAME "ldrawline2"
{
  int32_t i, first = -1, last;
  uint8_t *F;
  int32_t rs, cs, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  N = rs * cs;
  F = UCHARDATA(image1);
  
  for (i = 0; i < N; i++)
  {
    if (F[i])
    {
      if (first == -1) first = i;
      else {last = i; break;}
    }
  }
  lbresen(F, rs, first%rs, first/rs, last%rs, last/rs);

  return 1;
} // ldrawline2()

/* =============================================================== */
void ldrawfilledellipse(struct xvimage * image, double R, double S, double T, double U, double V, double Z)
/* =============================================================== */
/*
    \param image (entr�e/sortie) : l'image o� dessiner l'ellipse 
    \param R,S,T,U,V,Z (entr�e) : parametres de l'�quation de l'ellipse (Rxx + Syy + 2Txy + 2Ux + 2Vy + Z = 0)
*/
#undef F_NAME
#define F_NAME "ldrawfilledellipse"
{
  int32_t rs, cs, N, i, j;
  uint8_t *F;
  rs = rowsize(image);
  cs = colsize(image);
  N = rs * cs;
  F = UCHARDATA(image);
  
  memset(F, 0, N);
  for (j = 0; j < cs; j++)
  for (i = 0; i < rs; i++)
  {
    if (R*i*i + S*j*j + 2*T*i*j + 2*U*i + 2*V*j + Z <= 0)
      F[j * rs + i] = NDG_MAX;
  }

} // ldrawfilledellipse()

/* ==================================== */
void ldrawcubic1(struct xvimage * image1, double *x, double *y, int32_t nseg, double sx, double sy)
/* ==================================== */
/* draws a cubic line segment */
#undef F_NAME
#define F_NAME "ldrawcubic1"
{
  int32_t i, rs, cs, x1, y1, x2, y2;
  uint8_t *F;
  double X, Y, r = 1.0 / nseg, t = 0.0, t2, t3;

  rs = rowsize(image1);
  cs = colsize(image1);
  F = UCHARDATA(image1);
  
  X = x[0]*sx;
  Y = y[0]*sy;
  x1 = arrondi(X);
  y1 = arrondi(Y);
  for (i = 0; i < nseg; i++)
  {
    t += r; t2 = t*t; t3 = t2 * t;
    X = sx * (x[0] + t*x[1] +  t2*x[2] +  t3*x[3]);
    Y = sy * (y[0] + t*y[1] +  t2*y[2] +  t3*y[3]);
    x2 = arrondi(X);
    y2 = arrondi(Y);
    ldrawline(image1, x1, y1, x2, y2);
    x1 = x2; y1 = y2;
  }
} // ldrawcubic1()

/* ==================================== */
void ldrawcubic2(struct xvimage * image1, double *x, double *y, int32_t nseg, double tmin, double tmax)
/* ==================================== */
/*! \fn void ldrawcubic2(struct xvimage * image1, double *x, double *y, int32_t nseg, double tmin, double tmax)
    \param image1 (entr�e/sortie) : image o� dessiner le r�sutat
    \param x (entr�e) : coefficients 0, 1, 2, 3 du polynome des abcisses
    \param y (entr�e) : coefficients 0, 1, 2, 3 du polynome des ordonn�es
    \param nseg (entr�e) : nombre de pas de discr�tisation
    \param tmin, tmax (entr�e) : valeurs min et max du param�tre des polynomes
    \brief draws a cubic line segment 
    \warning le r�sultat n'est pas forc�ment une courbe discr�te
*/
#undef F_NAME
#define F_NAME "ldrawcubic2"
{
  int32_t i, x1, y1, x2, y2;
  double X, Y, r = (tmax - tmin) / nseg, t = tmin, t2, t3;

  t2 = t*t; t3 = t2 * t;
  X = x[0] + t*x[1] +  t2*x[2] +  t3*x[3];
  Y = y[0] + t*y[1] +  t2*y[2] +  t3*y[3];
  x1 = arrondi(X);
  y1 = arrondi(Y);
  for (i = 0; i < nseg; i++)
  {
    t += r; t2 = t*t; t3 = t2 * t;
    X = x[0] + t*x[1] +  t2*x[2] +  t3*x[3];
    Y = y[0] + t*y[1] +  t2*y[2] +  t3*y[3];
    x2 = arrondi(X);
    y2 = arrondi(Y);
printf("ldrawline %d %d ; %d %d\n", x1, y1, x2, y2);
    ldrawline(image1, x1, y1, x2, y2);
    x1 = x2; y1 = y2;
  }
} // ldrawcubic2()

/* ==================================== */
void ldrawcubic2list(int32_t *lx, int32_t *ly, int32_t *npoints, double *x, double *y, int32_t nseg, double tmin, double tmax)
/* ==================================== */
/*! \fn void ldrawcubic2list(int32_t *lx, int32_t *ly, int32_t *npoints, double *x, double *y, int32_t nseg, double tmin, double tmax)
    \param lx (sortie) : liste des abcisses des points dessin�s
    \param ly (sortie) : liste des ordonn�es des points dessin�s
    \param npoints (entr�e) : taille des tableaux lx, ly ; (sortie) : nombre de points dessin�s
    \param x (entr�e) : coefficients 0, 1, 2, 3 du polynome des abcisses
    \param y (entr�e) : coefficients 0, 1, 2, 3 du polynome des ordonn�es
    \param nseg (entr�e) : nombre de pas de discr�tisation
    \param tmin, tmax (entr�e) : valeurs min et max du param�tre des polynomes
    \brief draws a cubic line segment 
    \warning les tableaux lx, ly doivent avoir �t� allou�s
    \warning le r�sultat n'est pas forc�ment une courbe discr�te
*/
#undef F_NAME
#define F_NAME "ldrawcubic2list"
{
  int32_t i, x1, y1, x2, y2, np, npp, nmaxpoints = *npoints;
  double X, Y, r = (tmax - tmin) / nseg, t = tmin, t2, t3;
  
  t2 = t*t; t3 = t2 * t;
  X = x[0] + t*x[1] +  t2*x[2] +  t3*x[3];
  Y = y[0] + t*y[1] +  t2*y[2] +  t3*y[3];
  x1 = arrondi(X);
  y1 = arrondi(Y);
  np = 0;
  for (i = 0; i < nseg; i++)
  {
    t += r; t2 = t*t; t3 = t2 * t;
    X = x[0] + t*x[1] +  t2*x[2] +  t3*x[3];
    Y = y[0] + t*y[1] +  t2*y[2] +  t3*y[3];
    x2 = arrondi(X);
    y2 = arrondi(Y);
    npp = nmaxpoints - np;
#ifdef DEBUG
printf("ldrawlinelist np=%d ;  npp=%d\n", np, npp);
#endif
    ldrawlinelist(lx+np, ly+np, &npp, x1, y1, x2, y2);
    np += npp;
#ifdef DEBUG
printf("ldrawlinelist %d %d ; %d %d ; np=%d\n", x1, y1, x2, y2, np);
#endif
    x1 = x2; y1 = y2;
  }
  *npoints = np;
} // ldrawcubic2list()

/* ==================================== */
void ldrawcubic3d(struct xvimage * image1, double *x, double *y, double *z, int32_t nseg, double tmin, double tmax)
/* ==================================== */
/*! \fn void ldrawcubic3d(struct xvimage * image1, double *x, double *y, double *z, int32_t nseg, double tmin, double tmax)
    \param image1 (entr�e/sortie) : image o� dessiner le r�sutat
    \param x (entr�e) : coefficients 0, 1, 2, 3 du polynome des abcisses
    \param y (entr�e) : coefficients 0, 1, 2, 3 du polynome des ordonn�es
    \param z (entr�e) : coefficients 0, 1, 2, 3 du polynome des cotes
    \param nseg (entr�e) : nombre de pas de discr�tisation
    \param tmin, tmax (entr�e) : valeurs min et max du param�tre des polynomes
    \brief draws a cubic line segment 
    \warning le r�sultat n'est pas forc�ment une courbe discr�te
*/
#undef F_NAME
#define F_NAME "ldrawcubic3d"
{
  int32_t i, x1, y1, z1, x2, y2, z2;
  double X, Y, Z, r = (tmax - tmin) / nseg, t = tmin, t2, t3;

#ifdef DEBUG_DC3
  printf("%s: %g %g\n", F_NAME, tmin, tmax);
#endif

  t2 = t*t; t3 = t2 * t;
  X = x[0] + t*x[1] +  t2*x[2] +  t3*x[3];
  Y = y[0] + t*y[1] +  t2*y[2] +  t3*y[3];
  Z = z[0] + t*z[1] +  t2*z[2] +  t3*z[3];
  x1 = arrondi(X);
  y1 = arrondi(Y);
  z1 = arrondi(Z);
  for (i = 0; i < nseg; i++)
  {
    t += r; t2 = t*t; t3 = t2 * t;
    X = x[0] + t*x[1] +  t2*x[2] +  t3*x[3];
    Y = y[0] + t*y[1] +  t2*y[2] +  t3*y[3];
    Z = z[0] + t*z[1] +  t2*z[2] +  t3*z[3];
    x2 = arrondi(X);
    y2 = arrondi(Y);
    z2 = arrondi(Z);
#ifdef DEBUG_DC3
    printf("%s: ldrawline3d %d,%d,%d -> %d,%d,%d\n", F_NAME, x1, y1, z1, x2, y2, z2);
#endif
    ldrawline3d(image1, x1, y1, z1, x2, y2, z2);
    x1 = x2; y1 = y2; z1 = z2;
  }
} // ldrawcubic3d()

/* ==================================== */
void ldrawcubic3d_val(struct xvimage * image1, double *x, double *y, double *z, int32_t nseg, double tmin, double tmax, uint8_t v)
/* ==================================== */
/*! \fn void ldrawcubic3d_val(struct xvimage * image1, double *x, double *y, double *z, int32_t nseg, double tmin, double tmax, uint8_t v)
    \param image1 (entr�e/sortie) : image o� dessiner le r�sutat
    \param x (entr�e) : coefficients 0, 1, 2, 3 du polynome des abcisses
    \param y (entr�e) : coefficients 0, 1, 2, 3 du polynome des ordonn�es
    \param z (entr�e) : coefficients 0, 1, 2, 3 du polynome des cotes
    \param nseg (entr�e) : nombre de pas de discr�tisation
    \param tmin, tmax (entr�e) : valeurs min et max du param�tre des polynomes
    \brief draws a cubic line segment 
    \warning le r�sultat n'est pas forc�ment une courbe discr�te
*/
#undef F_NAME
#define F_NAME "ldrawcubic3d_val"
{
  int32_t i, x1, y1, z1, x2, y2, z2;
  double X, Y, Z, r = (tmax - tmin) / nseg, t = tmin, t2, t3;

  t2 = t*t; t3 = t2 * t;
  X = x[0] + t*x[1] +  t2*x[2] +  t3*x[3];
  Y = y[0] + t*y[1] +  t2*y[2] +  t3*y[3];
  Z = z[0] + t*z[1] +  t2*z[2] +  t3*z[3];
  x1 = arrondi(X);
  y1 = arrondi(Y);
  z1 = arrondi(Z);
  for (i = 0; i < nseg; i++)
  {
    t += r; t2 = t*t; t3 = t2 * t;
    X = x[0] + t*x[1] +  t2*x[2] +  t3*x[3];
    Y = y[0] + t*y[1] +  t2*y[2] +  t3*y[3];
    Z = z[0] + t*z[1] +  t2*z[2] +  t3*z[3];
    x2 = arrondi(X);
    y2 = arrondi(Y);
    z2 = arrondi(Z);
    ldrawline3d_val(image1, x1, y1, z1, x2, y2, z2, v);
    x1 = x2; y1 = y2; z1 = z2;
  }
} // ldrawcubic3d_val()

/* ==================================== */
void ldrawcubic3dlist(int32_t *lx, int32_t *ly, int32_t *lz, int32_t *npoints, double *x, double *y, double *z, int32_t nseg, double tmin, double tmax)
/* ==================================== */
/*! \fn void ldrawcubic3dlist(int32_t *lx, int32_t *ly, int32_t *lz, int32_t *npoints, double *x, double *y, double *z, int32_t nseg, double tmin, double tmax)
    \param lx (sortie) : liste des abcisses des points dessin�s
    \param ly (sortie) : liste des ordonn�es des points dessin�s
    \param lz (sortie) : liste des ordonn�es des points dessin�s
    \param npoints (entr�e) : taille des tableaux lx, ly, lz ; (sortie) : nombre de points dessin�s
    \param x (entr�e) : coefficients 0, 1, 2, 3 du polynome des abcisses
    \param y (entr�e) : coefficients 0, 1, 2, 3 du polynome des ordonn�es
    \param z (entr�e) : coefficients 0, 1, 2, 3 du polynome des cotes
    \param nseg (entr�e) : nombre de pas de discr�tisation
    \param tmin, tmax (entr�e) : valeurs min et max du param�tre des polynomes
    \brief draws a cubic line segment 
    \warning les tableaux lx, ly, lz doivent avoir �t� allou�s
    \warning le r�sultat n'est pas forc�ment une courbe discr�te
    \warning certains points peuvent �tre dupliqu�s
*/
#undef F_NAME
#define F_NAME "ldrawcubic3dlist"
{
  int32_t i, x1, y1, z1, x2, y2, z2, np = 0, npp, nmaxpoints = *npoints;
  double X, Y, Z, r = (tmax - tmin) / nseg, t = tmin, t2, t3;
  
  t2 = t*t; t3 = t2 * t;
  X = x[0] + t*x[1] +  t2*x[2] +  t3*x[3];
  Y = y[0] + t*y[1] +  t2*y[2] +  t3*y[3];
  Z = z[0] + t*z[1] +  t2*z[2] +  t3*z[3];
  x1 = arrondi(X);
  y1 = arrondi(Y);
  z1 = arrondi(Z);
  for (i = 0; i < nseg; i++)
  {
    t += r; t2 = t*t; t3 = t2 * t;
    X = x[0] + t*x[1] +  t2*x[2] +  t3*x[3];
    Y = y[0] + t*y[1] +  t2*y[2] +  t3*y[3];
    Z = z[0] + t*z[1] +  t2*z[2] +  t3*z[3];
    x2 = arrondi(X);
    y2 = arrondi(Y);
    z2 = arrondi(Z);
    npp = nmaxpoints - np;
    ldrawline3dlist(lx+np, ly+np, lz+np, &npp, x1, y1, z1, x2, y2, z2);
    np += npp;
    x1 = x2; y1 = y2; z1 = z2;
  }
  *npoints = np;
} // ldrawcubic3dlist()

/* ==================================== */
void ldrawtangents2d(
		     struct xvimage *field,    // image de sortie (le champs de vecteurs)
		     double *x, double *y,     // deux polynomes de degre 3 
		     int32_t nseg,             // nombre de segments pour la discretisation 
		     double tmin, double tmax) // bornes pour le coeff. des polynomes
/* ==================================== */
/* saves, in a vector field structure, the tangent vectors of a cubic curve */
#undef F_NAME
#define F_NAME "ldrawtangents2d"
{
  int32_t N, rs, cs, x1, y1;
  double X, Y, r, t, t2, t3;
  float * F;

  assert(nseg > 0); assert(tmax >= tmin);
  assert(depth(field) == 1);
  assert(datatype(field) == VFF_TYP_FLOAT);
  assert(nbands(field) == 2);

  r = (tmax - tmin) / nseg;
  rs = rowsize(field);
  cs = colsize(field);
  N = rs * cs;
  F = FLOATDATA(field);

  for (t = tmin; t <= tmax; t += r)
  {
    t2 = t * t; t3 = t2 * t;
    X = x[0] + t*x[1] + t2*x[2] + t3*x[3];
    Y = y[0] + t*y[1] + t2*y[2] + t3*y[3];

    x1 = arrondi(X);
    y1 = arrondi(Y);
		  
    F[y1*rs+x1] = x[1] + 2*t*x[2] + 3*t2*x[3];
    F[y1*rs+x1+N] = y[1] + 2*t*y[2] + 3*t2*y[3];
  }
} // ldrawtangents2d()

/* ==================================== */
void ldrawtangents3d(
		     struct xvimage *field,           // image de sortie (le champs de vecteurs)
		     double *x, double *y, double *z, // trois polynomes de degre 3 
		     int32_t nseg,                    // nombre de segments pour la discretisation 
		     double tmin, double tmax)        // bornes pour le coeff. des polynomes
/* ==================================== */
/* saves, in a vector field structure, the tangent vectors of a cubic curve */
#undef F_NAME
#define F_NAME "ldrawtangents3d"
{
  int32_t N, rs, cs, ds, ps, x1, y1, z1;
  double X, Y, Z, r, t, t2, t3;
  float * F;

  assert(nseg > 0); assert(tmax >= tmin);
  assert(datatype(field) == VFF_TYP_FLOAT);
  assert(nbands(field) == 3);

  r = (tmax - tmin) / nseg;
  rs = rowsize(field);
  cs = colsize(field);
  ds = depth(field);
  ps = rs * cs;
  N = ps * ds;
  F = FLOATDATA(field);

  for (t = tmin; t <= tmax; t += r)
  {
    t2 = t * t; t3 = t2 * t;
    X = x[0] + t*x[1] + t2*x[2] + t3*x[3];
    Y = y[0] + t*y[1] + t2*y[2] + t3*y[3];
    Z = z[0] + t*z[1] + t2*z[2] + t3*z[3];

    x1 = arrondi(X);
    y1 = arrondi(Y);
    z1 = arrondi(Z);
		  
    F[z1*ps+y1*rs+x1] = x[1] + 2*t*x[2] + 3*t2*x[3];
    F[z1*ps+y1*rs+x1+N] = y[1] + 2*t*y[2] + 3*t2*y[3];
    F[z1*ps+y1*rs+x1+N+N] = z[1] + 2*t*z[2] + 3*t2*z[3];
  }
} // ldrawtangents3d()

/* ==================================== */
void ldrawdirtangents3dlist(
			    double *Vx, double *Vy, double *Vz, // liste de nseg+1 vecteurs (r�sultat)
			    double *x, double *y, double *z, // trois polynomes de degre 3 
			    int32_t nseg,                    // nombre de segments pour la discretisation 
			    double tmin, double tmax)        // bornes pour le coeff. des polynomes
/* ==================================== */
// saves, in a vector list, the tangent direction vectors of a cubic curve
// the vector list Vx Vy Vz must be allocated before calling
#undef F_NAME
#define F_NAME "ldrawdirtangents3dlist"
{
  int32_t i;
  double r, t, t2;

  assert(nseg > 0); assert(tmax >= tmin); assert(Vx != NULL); assert(Vy != NULL); assert(Vz != NULL);
  r = (tmax - tmin) / nseg;
  for (i = 0, t = tmin; i <= nseg; t += r, i++)
  {
    t2 = t * t;
    Vx[i] = x[1] + 2*t*x[2] + 3*t2*x[3];
    Vy[i] = y[1] + 2*t*y[2] + 3*t2*y[3];
    Vz[i] = z[1] + 2*t*z[2] + 3*t2*z[3];
  }
} // ldrawdirtangents3dlist()

#define EPS_DRAWVECT 1e-5

/* ==================================== */
struct xvimage *ldrawfield2d(struct xvimage *field, double len)
/* ==================================== */
/* draws lines in output image that represent vectors in input field */
#undef F_NAME
#define F_NAME "ldrawfield2d"
{
  struct xvimage *image;
  int32_t N, rs, cs, x1, y1;
  double X, Y, t;
  float * F;

  assert(datatype(field) == VFF_TYP_FLOAT);
  assert(nbands(field) == 2);
  assert(depth(field) == 1);

  rs = rowsize(field);
  cs = colsize(field);
  N = rs * cs;
  F = FLOATDATA(field);

  image = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  assert(image != NULL);

  for (y1 = 0; y1 < cs; y1++)
  for (x1 = 0; x1 < rs; x1++)
  {
    X = F[y1*rs+x1];
    Y = F[y1*rs+x1+N];
    t = sqrt(X*X + Y*Y);
    if (t > EPS_DRAWVECT)
      ldrawline(image, x1, y1, arrondi((x1+(len*X))), arrondi((y1+(len*Y))));
  }
  return image;
} // ldrawfield2d()

/* ==================================== */
void ldrawfield2dlist(int32_t npoints, int32_t *X, int32_t *Y, double *tx, double *ty, struct xvimage *image, double len)
/* ==================================== */
/* draws lines in output image that represent vectors in input field */
#undef F_NAME
#define F_NAME "ldrawfield2dlist"
{
  int32_t i;
  double t, x, y;

  for (i = 0; i < npoints; i++)
  {
    x = tx[i];
    y = ty[i];
    t = sqrt(x*x + y*y);
    if (t > EPS_DRAWVECT)
      ldrawline(image, X[i], Y[i], arrondi((X[i]+(len*x))), arrondi((Y[i]+(len*y))));
  }
} // ldrawfield2dlist()

/* ==================================== */
struct xvimage *ldrawfield3d(struct xvimage *field, double len)
/* ==================================== */
/* draws lines in output image that represent vectors in input field */
#undef F_NAME
#define F_NAME "ldrawfield3d"
{
  struct xvimage *image;
  int32_t N, rs, cs, ds, ps, x1, y1, z1;
  double X, Y, Z, t;
  float * F;

  assert(datatype(field) == VFF_TYP_FLOAT);
  assert(nbands(field) == 3);

  rs = rowsize(field);
  cs = colsize(field);
  ds = depth(field);
  ps = rs * cs;
  N = ps * ds;
  F = FLOATDATA(field);

  image = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  assert(image != NULL);

  for (z1 = 0; z1 < ds; z1++)
  for (y1 = 0; y1 < cs; y1++)
  for (x1 = 0; x1 < rs; x1++)
  {
    X = F[z1*ps+y1*rs+x1];
    Y = F[z1*ps+y1*rs+x1+N];
    Z = F[z1*ps+y1*rs+x1+N+N];
    t = sqrt(X*X + Y*Y + Z*Z);
    if (t > EPS_DRAWVECT)
      ldrawline3d(image, x1, y1, z1, arrondi((x1+(len*X))), arrondi((y1+(len*Y))), arrondi((z1+(len*Z))));
  }
  return image;
} // ldrawfield3d()

/* ==================================== */
void ldrawfield3dlist(int32_t npoints, int32_t *X, int32_t *Y, int32_t *Z, double *tx, double *ty, double *tz, struct xvimage *image, double len)
/* ==================================== */
/* draws lines in output image that represent vectors in input field */
#undef F_NAME
#define F_NAME "ldrawfield3dlist"
{
  int32_t i;
  double t, x, y, z;

  for (i = 0; i < npoints; i++)
  {
    x = tx[i];
    y = ty[i];
    z = tz[i];
    t = sqrt(x*x + y*y + z*z);
    //printf("i=%d, t=%g\n", i, t);
    if (t > EPS_DRAWVECT)
    {
      //printf("drawing line %d %d %d - %d %d %d\n", X[i], Y[i], Z[i], arrondi((X[i]+(len*x))), arrondi((Y[i]+(len*y))), arrondi((Z[i]+(len*z))));
      ldrawline3d(image, X[i], Y[i], Z[i], arrondi((X[i]+(len*x))), arrondi((Y[i]+(len*y))), arrondi((Z[i]+(len*z))));
    }
  }
} // ldrawfield3dlist()

/* ==================================== */
int32_t ldrawball(struct xvimage * image1, double r, double xc, double yc, double zc)
/* ==================================== */
/* draws a euclidean ball */
#undef F_NAME
#define F_NAME "ldrawball"
{
  int32_t i, j, k, rs, cs, ds, ps;
  uint8_t *F;
  double r2, x, y, z;
  
  rs = rowsize(image1);
  cs = colsize(image1);
  ds = depth(image1);
  ps = rs*cs;
  F = UCHARDATA(image1);
  
  r2 = r * r;
  for (k = 0; k < ds; k++)
  for (j = 0; j < cs; j++)
  for (i = 0; i < rs; i++)
  {
    x = xc - i; 
    y = yc - j; 
    z = zc - k; 
    if (x * x + y * y + z * z <= r2)
      F[k * ps + j * rs + i] = NDG_MAX;
  }

  return 1;
} // ldrawball()

/* ==================================== */
void ldrawdisc(struct xvimage * image1, double r, double xc, double yc)
/* ==================================== */
/* draws a euclidean disc */
#undef F_NAME
#define F_NAME "ldrawdisc"
{
  int32_t i, j, rs, cs;
  uint8_t *F;
  double r2, x, y;
  
  rs = rowsize(image1);
  cs = colsize(image1);
  F = UCHARDATA(image1);
  
  r2 = r * r;
  for (j = 0; j < cs; j++)
  for (i = 0; i < rs; i++)
  {
    x = xc - i; 
    y = yc - j; 
    if (x * x + y * y <= r2)
      F[j * rs + i] = NDG_MAX;
  }
} // ldrawdisc()

/* ==================================== */
int32_t ldrawtorus(struct xvimage * image1, double c, double a, double xc, double yc, double zc)
/* ==================================== */
/* draws a torus of equation (c-sqrt((xc-x)^2+(yc-y)^2))^2+(zc-z)^2 <= a^2 */
#undef F_NAME
#define F_NAME "ldrawtorus"
{
  int32_t i, j, k, rs, cs, ds, ps;
  uint8_t *F;
  double t, a2, x, y, z;
  
  rs = rowsize(image1);
  cs = colsize(image1);
  ds = depth(image1);
  ps = rs*cs;
  F = UCHARDATA(image1);
  
  a2 = a * a;
  for (k = 0; k < ds; k++)
  for (j = 0; j < cs; j++)
  for (i = 0; i < rs; i++)
  {
    x = xc - i; 
    y = yc - j; 
    z = zc - k;
    t = c - sqrt(x*x + y*y);
    if (t*t + z*z <= a2)
      F[k * ps + j * rs + i] = NDG_MAX;
  }

  return 1;
} // ldrawtorus()

/* ==================================== */
int isincylinder(
		 int32_t xM, int32_t yM, int32_t zM,
		 int32_t xA, int32_t yA, int32_t zA, 
		 double e, double r, double * P )
/* ==================================== */
#undef F_NAME
#define F_NAME  "isincylinder"
//retourne 1 si le point M (exprim� dans la base d'origine) est bien contenu dans le cylindre de parametres e (hauteur) et r (rayon) et de centre A (coordonnees de A exprim�s dans la base d'origine)
//et 0 sinon
//Note: P matrice de passage telle que: (point exprim� dans la base d'origine) = (point exprim� dans la nouvelle base)* P
{
  double u[3], v[3], w[3]; //les vecteurs de la nouvelle base
  double ub[3], vb[3], wb[3]; //les vecteurs exprim�s dans la base d'origine
  u[0]=1; u[1]=0; u[2]=0;
  v[0]=0; v[1]=1; v[2]=0;
  w[0]=0; w[1]=0; w[2]=1;
	
  lin_mult(P, u ,ub, 3, 3, 1);
  lin_mult(P, v, vb, 3, 3, 1);
  lin_mult(P, w, wb, 3, 3, 1);
  
  //les vars qui suivent sont les coefficients obtenus en faisant les produits scalaires sur les diff�rents axes

  double cz,cy,cx;	
  cz = (xM-xA)*wb[0]+(yM-yA)*wb[1]+(zM-zA)*wb[2];
  cx = (xM-xA)*ub[0]+(yM-yA)*ub[1]+(zM-zA)*ub[2];
  cy = (xM-xA)*vb[0]+(yM-yA)*vb[1]+(zM-zA)*vb[2];
	
  //on doit avoir AM.z<=e/2 et sqrt(cx*cx + cy*cy))<=r 
  if ((cz <= (e/2)) && (cz >= -(e/2)) && ((sqrt(cx*cx + cy*cy))<=r)) return 1;
  
  return 0;
} // isincylinder()

/* ==================================== */
int32_t ldrawcylinder (
		       struct xvimage* image, 
		       double xAd, double yAd, double zAd, 
		       double xD, double yD, double zD, 		       
		       double thickness, double radius)
/* ==================================== */
/* 
   draws a cylinder centered in A with direction of axis (xD, yD, zD)
*/
#undef F_NAME
#define F_NAME "ldrawcylinder"
{
  uint8_t *img = UCHARDATA(image);
  double A[3], v[3], n1[3], n2[3], P[9], d;
  int32_t i, j, k, l, rs, cs, ds, ps, N;
  int32_t xA = (int32_t)xAd, yA = (int32_t)yAd, zA = (int32_t)zAd;
  rs=rowsize(image);
  cs=colsize(image);
  ds=depth(image);
  ps=rs*cs;
  N=ps*ds;
  A[0] = xA; A[1] = yA; A[2] = zA; 
  v[0] = xD; v[1] = yD; v[2] = zD; 
  
  d = lin_norme2(v, 3, 1);
  lin_multscal(v, 1/d, 3, 1);
  lin_createbase(v, n1, n2);
			
  //matrice de passage:
  P[0]=n1[0]; 	P[1]=n2[0]; 	P[2]=v[0]; 
  P[3]=n1[1]; 	P[4]=n2[1]; 	P[5]=v[1]; 
  P[6]=n1[2]; 	P[7]=n2[2]; 	P[8]=v[2]; 
      
  //on parcourt un cube dont le centre est notre point A a la recherche des points contenus dans le cylindre
  for (j=-(radius+thickness);j<=radius+thickness;j++)
  {
    for (k=-(radius+thickness);k<=radius+thickness;k++)
    {
      for (l=-(radius+thickness);l<=radius+thickness;l++)
      {
	if ( (xA+l)>=0 && (xA+l)<rs && (yA+k)>=0 && (yA+k)<cs && (zA+j)>=0 && (zA+j)<ds && ((zA+j)*ps+(yA+k)*rs+xA+l)>=0 && ((zA+j)*ps+(yA+k)*rs+xA+l)<N)
	{
	  if (isincylinder(xA+l, yA+k, zA+j , xA, yA, zA, thickness, radius, P)) 
	  {
	    img[(zA+j)*ps+(yA+k)*rs+xA+l] = NDG_MAX;
	  }
	}
      } //for l	  
    } //for k
  } // for j
  return 1;
} // ldrawcylinder()

