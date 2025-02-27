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

   l3dcollapse: collapse guid�� et contraint (OBSOLETE)
     
   Michel Couprie - avril 2007

   l3dpardircollapse: collapse parall��le par sous-��tapes directionnelles
   l3dpardircollapse_l: collapse guid�� et contraint - priorit�� ULONG
   l3dpardircollapse_f: collapse guid�� et contraint - priorit�� FLOAT
   l3dsurfacecollapse: collapse avec condition d'arr��t surfacique
   l3dtopoflow_l: topological flow - priorit�� ULONG
   l3dtopoflow_f: topological flow - priorit�� FLOAT

   Michel Couprie - juillet 2015

   l3dcurvecollapse: collapse avec condition d'arr��t curviligne

*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcrlifo.h>
#include <mcrbt.h>
#include <mcindic.h>
#include <mcutil.h>
#include <mcgraphe.h>
#include <mckhalimsky3d.h>
#include <mcgeo.h>
#include <ldist.h>
#include <lmedialaxis.h>
#include <l3dkhalimsky.h>
#include <l3dcollapse.h>

#define EN_RBT        0
#define EN_RLIFO       1
#define BORDER        2
//#define VERBOSE

/* =============================================================== */
int32_t l3dcollapse(struct xvimage * k, struct xvimage * prio, struct xvimage * inhibit)
/* =============================================================== */
/* 
  collapse s��quentiel, guid�� et contraint
  OBSOLETE - utiliser l3dpardircollapse_l
*/
#undef F_NAME
#define F_NAME "l3dcollapse"
{
  int32_t n;
  index_t u, v, x, y, z, xv, yv, zv;
  index_t i, rs, cs, ps, ds, N;
  uint8_t * K;
  int32_t * P;
  uint8_t * I = NULL;
  Rbt * RBT;
  index_t taillemaxrbt;
  index_t tab[GRS3D*GCS3D*GDS3D];

  rs = rowsize(k);
  cs = colsize(k);
  ds = depth(k);
  ps = rs * cs;
  N = ps * ds;
  K = UCHARDATA(k);

  IndicsInit(N);

  if (prio == NULL)
  {
    fprintf(stderr, "%s : prio is needed\n", F_NAME);
    return(0);
  }

  if ((rowsize(prio) != rs) || (colsize(prio) != cs) || (depth(prio) != ds))
  {
    fprintf(stderr, "%s : bad size for prio\n", F_NAME);
    return(0);
  }
  if (datatype(prio) == VFF_TYP_4_BYTE) 
    P = SLONGDATA(prio); 
  else 
  {
    fprintf(stderr, "%s : datatype(prio) must be int32_t\n", F_NAME);
    return(0);
  }

  if (inhibit != NULL)
  {
    if ((rowsize(inhibit) != rs) || (colsize(inhibit) != cs) || (depth(inhibit) != ds))
    {
      fprintf(stderr, "%s : bad size for inhibit\n", F_NAME);
      return(0);
    }
    if (datatype(inhibit) == VFF_TYP_1_BYTE) 
      I = UCHARDATA(inhibit); 
    else 
    {
      fprintf(stderr, "%s : datatype(inhibit) must be uint8_t\n", F_NAME);
      return(0);
    }
  }

  taillemaxrbt = 2 * (rs + cs + ds);
  /* cette taille est indicative, le RBT est realloue en cas de depassement */
  RBT = mcrbt_CreeRbtVide(taillemaxrbt);
  if (RBT == NULL)
  {
    fprintf(stderr, "%s : mcrbt_CreeRbtVide failed\n", F_NAME);
    return(0);
  }

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  /* ========================================================= */
  /*   INITIALISATION DU RBT */
  /* ========================================================= */

  for (z = 0; z < ds; z++)
  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    i = z*ps + y*rs + x;
    if (K[i] && ((I == NULL) || (!I[i])) && FaceLibre3d(k, x, y, z))
    {
      mcrbt_RbtInsert(&RBT, P[i], i);
      Set(i, EN_RBT);
    }
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  while (!mcrbt_RbtVide(RBT))
  {
    i = RbtPopMin(RBT);
    UnSet(i, EN_RBT);
    x = i % rs; y = (i % ps) / rs; z = i / ps;
    u = Collapse3d(k, x, y, z);
    if (u != -1)
    {
      x = u % rs; y = (u % ps) / rs; z = u / ps;
      Alphacarre3d(rs, cs, ds, x, y, z, tab, &n);
      for (u = 0; u < n; u += 1)
      {
        v = tab[u];
	xv = v % rs; yv = (v % ps) / rs; zv = v / ps;
	if (K[v] && !IsSet(v, EN_RBT) && ((I == NULL) || (!I[v])) && FaceLibre3d(k, xv, yv, zv))
	{
	  mcrbt_RbtInsert(&RBT, P[v], v);
	  Set(v, EN_RBT);
	}
      }
    }
  } /* while (!mcrbt_RbtVide(RBT)) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  mcrbt_RbtTermine(RBT);
  return 1;

} /* l3dcollapse() */

/* =============================================================== */
int32_t l3dpardircollapse_l(struct xvimage * k, struct xvimage * prio, struct xvimage * inhibit, int32_t priomax)
/* =============================================================== */
/* 
  collapse parall��le directionnel
  fonction de priorit�� en entiers longs
  les ��l��ments �� pr��server sont ceux de l'image "inhibit" ou, si celle-ci est "NULL", ceux sup��rieurs �� "priomax" 
*/
#undef F_NAME
#define F_NAME "l3dpardircollapse_l"
{
  int32_t u, n;
  index_t g, f, xf, yf, zf, xg, yg, zg;
  index_t i, rs, cs, ps, ds, N;
  int32_t dim, ori, dir, direc, orien;
  uint8_t * K;
  int32_t * P;
  uint8_t * I = NULL;
  Rbt * RBT;
  Rlifo * RLIFO;
  Rlifo * RLIFOb;
  index_t taillemaxrbt;
  index_t tab[GRS3D*GCS3D*GDS3D];
  TypRbtKey p, pp;

  rs = rowsize(k);
  cs = colsize(k);
  ds = depth(k);
  ps = rs * cs;
  N = ps * ds;
  K = UCHARDATA(k);

  IndicsInit(N);

  if (prio == NULL)
  {
    fprintf(stderr, "%s : prio is needed\n", F_NAME);
    return(0);
  }

  if ((rowsize(prio) != rs) || (colsize(prio) != cs) || (depth(prio) != ds))
  {
    fprintf(stderr, "%s : bad size for prio\n", F_NAME);
    return(0);
  }
  if (datatype(prio) == VFF_TYP_4_BYTE) 
    P = SLONGDATA(prio); 
  else 
  {
    fprintf(stderr, "%s : datatype(prio) must be int32_t\n", F_NAME);
    return(0);
  }

  if (inhibit != NULL)
  {
    if ((rowsize(inhibit) != rs) || (colsize(inhibit) != cs) || (depth(inhibit) != ds))
    {
      fprintf(stderr, "%s : bad size for inhibit\n", F_NAME);
      return(0);
    }
    if (datatype(inhibit) == VFF_TYP_1_BYTE) 
      I = UCHARDATA(inhibit); 
    else 
    {
      fprintf(stderr, "%s : datatype(inhibit) must be uint8_t\n", F_NAME);
      return(0);
    }
  }

  taillemaxrbt = 4 * (rs*cs + cs*ds + ds*rs);
  /* cette taille est indicative, le RBT est realloue en cas de depassement */
  RBT = mcrbt_CreeRbtVide(taillemaxrbt);
  if (RBT == NULL)
  {
    fprintf(stderr, "%s : mcrbt_CreeRbtVide failed\n", F_NAME);
    return(0);
  }

  RLIFO = CreeRlifoVide(taillemaxrbt);
  RLIFOb = CreeRlifoVide(taillemaxrbt);
  if ((RLIFO == NULL) || (RLIFOb == NULL))
  {
    fprintf(stderr, "%s : CreeRlifoVide failed\n", F_NAME);
    return(0);
  }

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  /* ========================================================= */
  /*   INITIALISATION DU RBT */
  /* ========================================================= */

  for (zg = 0; zg < ds; zg++)
  for (yg = 0; yg < cs; yg++)
  for (xg = 0; xg < rs; xg++)
  {
    g = zg*ps + yg*rs + xg;
    if (K[g])
    {
      f = PaireLibre3d(k, xg, yg, zg);
      if ((f != -1) && 
	  (((I != NULL) && (!I[g] && !I[f])) || 
	   ((I == NULL) && (P[g] < priomax) && (P[f] < priomax)) ) )
      {
	pp = (TypRbtKey)(mcmax(P[g],P[f]));
	mcrbt_RbtInsert(&RBT, pp, g);
	Set(g, EN_RBT);
      }
    }
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  while (!mcrbt_RbtVide(RBT))
  {
    // construit la liste de toutes les paires libres ayant la priorit�� courante
    p = RbtMinLevel(RBT); 
    while (!mcrbt_RbtVide(RBT) && (RbtMinLevel(RBT) == p))
    {
      g = RbtPopMin(RBT);
      UnSet(g, EN_RBT);
      xg = g % rs; yg = (g % ps) / rs; zg = g / ps;
      f = PaireLibre3d(k, xg, yg, zg);
      if (f != -1)
      {
	RlifoPush(&RLIFO, f);
	RlifoPush(&RLIFO, g);
	Set(g, EN_RLIFO);
      }
    } // while (!mcrbt_RbtVide(RBT) && (RbtMinLevel(RBT) == p))

    for (dir = 0; dir <= 2; dir++) // For all face directions
      for (ori = 0; ori <= 1; ori++) // For both orientations
      {
	for (dim = 3; dim >= 1; dim--) // For dimensions in decreasing order
	{
	  for (i = 0; i < RLIFO->Sp; i += 2) // Scan the free faces list (dim 3)
	  {
	    f = RLIFO->Pts[i];
	    g = RLIFO->Pts[i+1];
	    xf = f % rs; yf = (f % ps) / rs; zf = f / ps;
	    if (K[f] && K[g] && 
		(((I != NULL) && (!I[g] && !I[f])) || 
		 ((I == NULL) && (P[g] < priomax) && (P[f] < priomax)) ) )
	    {
	      xg = g % rs; yg = (g % ps) / rs; zg = g / ps;  
	      if (xf - xg)      { direc = 0; if (xf > xg) orien = 0; else orien = 1; }
	      else if (yf - yg) { direc = 1; if (yf > yg) orien = 0; else orien = 1; }
	      else              { direc = 2; if (zf > zg) orien = 0; else orien = 1; }
	      if ((DIM3D(xf,yf,zf) == dim) && (direc == dir) && (orien == ori))
	      {
		K[g] = K[f] = VAL_NULLE; // COLLAPSE

		Alphacarre3d(rs, cs, ds, xf, yf, zf, tab, &n);
		for (u = 0; u < n; u += 1)
		{
		  g = tab[u];
		  xg = g % rs; yg = (g % ps) / rs; zg = g / ps;
		  if (K[g] &&
		      (((I != NULL) && (!I[g])) || ((I == NULL) && (P[g] < priomax))) )
		  {
		    f = PaireLibre3d(k, xg, yg, zg);
		    if ((f != -1) &&
			(((I != NULL) && (!I[f])) || ((I == NULL) && (P[f] < priomax))) )
		    {
		      if ((P[g] <= p) && (P[f] <= p) && !IsSet(g, EN_RLIFO))
		      { // Pr��paration sous-��tapes suivantes
			RlifoPush(&RLIFOb, f);
			RlifoPush(&RLIFOb, g);
		      }
		      if (!IsSet(g, EN_RBT))
		      { // Pr��paration ��tape suivante
			pp = (TypRbtKey)(mcmax(P[g],P[f]));
			mcrbt_RbtInsert(&RBT, pp, g);
			Set(g, EN_RBT);
		      }
		    } // if (f != -1)
		  } // if K[g] && ...
		} // for u
	      } // if ((DIM3D(xf,yf,zf) == dim) &&...
	    } // if (K[f] && K[g])
	  } // for (i = 0; i < RLIFO->Sp; i += 2)
	  while (!RlifoVide(RLIFOb))
	  { 
	    g = RlifoPop(RLIFOb);
	    f = RlifoPop(RLIFOb);
	    RlifoPush(&RLIFO, f); 
	    RlifoPush(&RLIFO, g); 
	  }
      } // for (dim = 3; dim >= 1; dim--)
    } // for for

    RlifoFlush(RLIFO);
  } /* while (!mcrbt_RbtVide(RBT)) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  mcrbt_RbtTermine(RBT);
  RlifoTermine(RLIFO);
  RlifoTermine(RLIFOb);
  return 1;

} /* l3dpardircollapse_l() */
 
/* =============================================================== */
int32_t l3dpardircollapse_f(struct xvimage * k, struct xvimage * prio, struct xvimage * inhibit, float priomax)
/* =============================================================== */
/* 
  collapse parall��le directionnel
  fonction de priorit�� en flottants
  les ��l��ments �� pr��server sont ceux de l'image "inhibit" ou, si celle-ci est "NULL", ceux sup��rieurs �� "priomax" 
*/
#undef F_NAME
#define F_NAME "l3dpardircollapse_f"
{
  int32_t u, n;
  index_t g, f, xf, yf, zf, xg, yg, zg;
  index_t i, rs, cs, ps, ds, N;
  int32_t dim, ori, dir, direc, orien;
  uint8_t * K;
  float * P;
  uint8_t * I = NULL;
  Rbt * RBT;
  Rlifo * RLIFO;
  Rlifo * RLIFOb;
  index_t taillemaxrbt;
  index_t tab[GRS3D*GCS3D*GDS3D];
  TypRbtKey p, pp;

  rs = rowsize(k);
  cs = colsize(k);
  ds = depth(k);
  ps = rs * cs;
  N = ps * ds;
  K = UCHARDATA(k);

  IndicsInit(N);

  if (prio == NULL)
  {
    fprintf(stderr, "%s : prio is needed\n", F_NAME);
    return(0);
  }

  if ((rowsize(prio) != rs) || (colsize(prio) != cs) || (depth(prio) != ds))
  {
    fprintf(stderr, "%s : bad size for prio\n", F_NAME);
    return(0);
  }
  if (datatype(prio) == VFF_TYP_FLOAT) 
    P = FLOATDATA(prio); 
  else 
  {
    fprintf(stderr, "%s : datatype(prio) must be float\n", F_NAME);
    return(0);
  }

  if (inhibit != NULL)
  {
    if ((rowsize(inhibit) != rs) || (colsize(inhibit) != cs) || (depth(inhibit) != ds))
    {
      fprintf(stderr, "%s : bad size for inhibit\n", F_NAME);
      return(0);
    }
    if (datatype(inhibit) == VFF_TYP_1_BYTE) 
      I = UCHARDATA(inhibit); 
    else 
    {
      fprintf(stderr, "%s : datatype(inhibit) must be uint8_t\n", F_NAME);
      return(0);
    }
  }

  taillemaxrbt = 4 * (rs*cs + cs*ds + ds*rs);
  /* cette taille est indicative, le RBT est realloue en cas de depassement */
  RBT = mcrbt_CreeRbtVide(taillemaxrbt);
  if (RBT == NULL)
  {
    fprintf(stderr, "%s : mcrbt_CreeRbtVide failed\n", F_NAME);
    return(0);
  }

  RLIFO = CreeRlifoVide(taillemaxrbt);
  RLIFOb = CreeRlifoVide(taillemaxrbt);
  if ((RLIFO == NULL) || (RLIFOb == NULL))
  {
    fprintf(stderr, "%s : CreeRlifoVide failed\n", F_NAME);
    return(0);
  }

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  /* ========================================================= */
  /*   INITIALISATION DU RBT */
  /* ========================================================= */

  for (zg = 0; zg < ds; zg++)
  for (yg = 0; yg < cs; yg++)
  for (xg = 0; xg < rs; xg++)
  {
    g = zg*ps + yg*rs + xg;
    if (K[g])
    {
      f = PaireLibre3d(k, xg, yg, zg);
      if ((f != -1) && 
	  (((I != NULL) && (!I[g] && !I[f])) || 
	   ((I == NULL) && (P[g] < priomax) && (P[f] < priomax)) ) )
      {
	pp = (TypRbtKey)(mcmax(P[g],P[f]));
	mcrbt_RbtInsert(&RBT, pp, g);
	Set(g, EN_RBT);
      }
    }
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  while (!mcrbt_RbtVide(RBT))
  {
    // construit la liste de toutes les paires libres ayant la priorit�� courante
    p = RbtMinLevel(RBT); 
    while (!mcrbt_RbtVide(RBT) && (RbtMinLevel(RBT) == p))
    {
      g = RbtPopMin(RBT);
      UnSet(g, EN_RBT);
      xg = g % rs; yg = (g % ps) / rs; zg = g / ps;
      f = PaireLibre3d(k, xg, yg, zg);
      if (f != -1)
      {
	RlifoPush(&RLIFO, f);
	RlifoPush(&RLIFO, g);
	Set(g, EN_RLIFO);
      }
    } // while (!mcrbt_RbtVide(RBT) && (RbtMinLevel(RBT) == p))

    for (dir = 0; dir <= 2; dir++) // For all face directions
      for (ori = 0; ori <= 1; ori++) // For both orientations
      {
	for (dim = 3; dim >= 1; dim--) // For dimensions in decreasing order
	{
	  for (i = 0; i < RLIFO->Sp; i += 2) // Scan the free faces list (dim 3)
	  {
	    f = RLIFO->Pts[i];
	    g = RLIFO->Pts[i+1];
	    xf = f % rs; yf = (f % ps) / rs; zf = f / ps;
	    if (K[f] && K[g] && 
		(((I != NULL) && (!I[g] && !I[f])) || 
		 ((I == NULL) && (P[g] < priomax) && (P[f] < priomax)) ) )
	    {
	      xg = g % rs; yg = (g % ps) / rs; zg = g / ps;  
	      if (xf - xg)      { direc = 0; if (xf > xg) orien = 0; else orien = 1; }
	      else if (yf - yg) { direc = 1; if (yf > yg) orien = 0; else orien = 1; }
	      else              { direc = 2; if (zf > zg) orien = 0; else orien = 1; }
	      if ((DIM3D(xf,yf,zf) == dim) && (direc == dir) && (orien == ori))
	      {
		K[g] = K[f] = VAL_NULLE; // COLLAPSE

		Alphacarre3d(rs, cs, ds, xf, yf, zf, tab, &n);
		for (u = 0; u < n; u += 1)
		{
		  g = tab[u];
		  xg = g % rs; yg = (g % ps) / rs; zg = g / ps;
		  if (K[g] &&
		      (((I != NULL) && (!I[g])) || ((I == NULL) && (P[g] < priomax))) )
		  {
		    f = PaireLibre3d(k, xg, yg, zg);
		    if ((f != -1) &&
			(((I != NULL) && (!I[f])) || ((I == NULL) && (P[f] < priomax))) )
		    {
		      if ((P[g] <= p) && (P[f] <= p) && !IsSet(g, EN_RLIFO))
		      { // Pr��paration sous-��tapes suivantes
			RlifoPush(&RLIFOb, f);
			RlifoPush(&RLIFOb, g);
		      }
		      if (!IsSet(g, EN_RBT))
		      { // Pr��paration ��tape suivante
			pp = (TypRbtKey)(mcmax(P[g],P[f]));
			mcrbt_RbtInsert(&RBT, pp, g);
			Set(g, EN_RBT);
		      }
		    } // if (f != -1)
		  } // if K[g] && ...
		} // for u
	      } // if ((DIM3D(xf,yf,zf) == dim) &&...
	    } // if (K[f] && K[g])
	  } // for (i = 0; i < RLIFO->Sp; i += 2)
	  while (!RlifoVide(RLIFOb))
	  { 
	    g = RlifoPop(RLIFOb);
	    f = RlifoPop(RLIFOb);
	    RlifoPush(&RLIFO, f); 
	    RlifoPush(&RLIFO, g); 
	  }
      } // for (dim = 3; dim >= 1; dim--)
    } // for for

    RlifoFlush(RLIFO);
  } /* while (!mcrbt_RbtVide(RBT)) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  mcrbt_RbtTermine(RBT);
  RlifoTermine(RLIFO);
  RlifoTermine(RLIFOb);
  return 1;

} /* l3dpardircollapse_f() */

/* =============================================================== */
int32_t l3dpardircollapse_short(struct xvimage * k, int32_t nsteps)
/* =============================================================== */
/* 
  collapse parall��le directionnel
  sans fonction de priorit��
*/
#undef F_NAME
#define F_NAME "l3dpardircollapse_short"
{
  int result;
  result = l3dpardircollapse(k, nsteps, NULL);
  return result;
} /* l3dpardircollapse_short */


/* =============================================================== */
int32_t l3dpardircollapse(struct xvimage * k, int32_t nsteps, struct xvimage * inhibit)
/* =============================================================== */
/* 
  collapse parall��le directionnel
  sans fonction de priorit��
*/
#undef F_NAME
#define F_NAME "l3dpardircollapse"
{
  int32_t u, n;
  index_t g, f, xf, yf, zf, xg, yg, zg;
  index_t i, rs, cs, ps, ds, N;
  int32_t dim, ori, dir, direc, orien, ncol;
  uint8_t * K;
  uint8_t * I = NULL;
  Rlifo * RLIFO;
  Rlifo * RLIFOb;
  Rlifo * RLIFOt;
  index_t taillemax;
  index_t tab[GRS3D*GCS3D*GDS3D];

  rs = rowsize(k);
  cs = colsize(k);
  ds = depth(k);
  ps = rs * cs;
  N = ps * ds;
  K = UCHARDATA(k);

  IndicsInit(N);

  if (inhibit != NULL)
  {
    if ((rowsize(inhibit) != rs) || (colsize(inhibit) != cs) || (depth(inhibit) != ds))
    {
      fprintf(stderr, "%s : bad size for inhibit\n", F_NAME);
      return(0);
    }
    if (datatype(inhibit) == VFF_TYP_1_BYTE) 
      I = UCHARDATA(inhibit); 
    else 
    {
      fprintf(stderr, "%s : datatype(inhibit) must be uint8_t\n", F_NAME);
      return(0);
    }
  }

  taillemax = 4 * (rs*cs + cs*ds + ds*rs);
  RLIFO = CreeRlifoVide(taillemax);
  RLIFOb = CreeRlifoVide(taillemax);
  if ((RLIFO == NULL) || (RLIFOb == NULL))
  {
    fprintf(stderr, "%s : CreeRlifoVide failed\n", F_NAME);
    return(0);
  }

  if (nsteps == -1) nsteps = 1000000000;

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  /* ========================================================= */
  /* INITIALISATION DE LA RLIFO ET DE LA "BORDER" */
  /* ========================================================= */

  for (zg = 0; zg < ds; zg++)
  for (yg = 0; yg < cs; yg++)
  for (xg = 0; xg < rs; xg++)
  {
    g = zg*ps + yg*rs + xg;
    if (K[g] && ((I == NULL) || (!I[g])))
    {
      f = PaireLibre3d(k, xg, yg, zg);
      if (f != -1) 
      { 
	RlifoPush(&RLIFO, f); 
	RlifoPush(&RLIFO, g);
	Set(g, EN_RLIFO);
	xf = f % rs; yf = (f % ps) / rs; zf = f / ps;
	Alphacarre3d(rs, cs, ds, xf, yf, zf, tab, &n);
	for (u = 0; u < n; u += 1)
	{
	  g = tab[u];
	  Set(g, BORDER);
	} // for u
      }
    }
  }

  /* ================================================ */
  /*              DEBUT BOUCLE PRINCIPALE             */
  /* ================================================ */

  ncol = 1;
  while (!RlifoVide(RLIFO) && (nsteps > 0) && (ncol > 0))
  {

#ifdef VERBOSE
    fprintf(stderr, "%s: steps remaining: %d\n", F_NAME, nsteps);
#endif

    nsteps --;
    ncol = 0;
    for (dir = 0; dir <= 2; dir++) // For all face directions
      for (ori = 0; ori <= 1; ori++) // For both orientations
      {
	for (dim = 3; dim >= 1; dim--) // For dimensions in decreasing order
	{
	  for (i = 0; i < RLIFO->Sp; i += 2) // Scan the free faces list (dim 3)
	  {
	    f = RLIFO->Pts[i];
	    g = RLIFO->Pts[i+1];
	    xf = f % rs; yf = (f % ps) / rs; zf = f / ps;
	    if (K[f] && K[g] && ((I == NULL) || (!I[g] && !I[f])))
	    {
	      xg = g % rs; yg = (g % ps) / rs; zg = g / ps;  
	      if (xf - xg)      { direc = 0; if (xf > xg) orien = 0; else orien = 1; }
	      else if (yf - yg) { direc = 1; if (yf > yg) orien = 0; else orien = 1; }
	      else              { direc = 2; if (zf > zg) orien = 0; else orien = 1; }
	      if ((DIM3D(xf,yf,zf) == dim) && (direc == dir) && (orien == ori))
	      {
		K[g] = K[f] = VAL_NULLE;
		ncol += 1;
		// Pr��paration sous-��tapes suivantes
		Alphacarre3d(rs, cs, ds, xf, yf, zf, tab, &n);
		for (u = 0; u < n; u += 1)
		{
		  g = tab[u];
		  xg = g % rs; yg = (g % ps) / rs; zg = g / ps;
		  if (K[g] && ((I == NULL) || (!I[g])))
		  {
		    f = PaireLibre3d(k, xg, yg, zg);
		    if ((f != -1) && IsSet(f, BORDER) && !IsSet(g, EN_RLIFO))
		    { 
		      RlifoPush(&RLIFOb, f); 
		      RlifoPush(&RLIFOb, g);
		    }
		  }
		} // for u
	      } // if ((DIM3D(xf,yf,zf) == dim) &&...
	    } // if (K[f] && K[g])
	  } // for (i = 0; i < RLIFO->Sp; i += 2)
	  while (!RlifoVide(RLIFOb))
	  { 
	    g = RlifoPop(RLIFOb);
	    f = RlifoPop(RLIFOb);
	    RlifoPush(&RLIFO, f); 
	    RlifoPush(&RLIFO, g); 
	  }
      } // for (dim = 3; dim >= 1; dim--)
    } // for for

    // PREPARATION ETAPE SUIVANTE
    for (i = 0; i < RLIFO->Sp; i++) UnSet(RLIFO->Pts[i], EN_RLIFO);
    for (i = 0; i < RLIFO->Sp; i += 2)
    {
      f = RLIFO->Pts[i];
      UnSet(f, BORDER);
      xf = f % rs; yf = (f % ps) / rs; zf = f / ps;
      Alphacarre3d(rs, cs, ds, xf, yf, zf, tab, &n);
      for (u = 0; u < n; u += 1)
      {
	g = tab[u];
	UnSet(g, BORDER);
	if (K[g] && !IsSet(g, EN_RLIFO) && ((I == NULL) || (!I[g])))
	{
	  xg = g % rs; yg = (g % ps) / rs; zg = g / ps;
	  f = PaireLibre3d(k, xg, yg, zg);
	  if (f != -1) 
	  { 
	    RlifoPush(&RLIFOb, f); 
	    RlifoPush(&RLIFOb, g);
	    Set(g, EN_RLIFO);
	  }
	}
      } // for u
    }

    for (i = 0; i < RLIFOb->Sp; i += 2)
    {
      f = RLIFOb->Pts[i];
      xf = f % rs; yf = (f % ps) / rs; zf = f / ps;
      Alphacarre3d(rs, cs, ds, xf, yf, zf, tab, &n);
      for (u = 0; u < n; u += 1)
      {
	g = tab[u];
	Set(g, BORDER);
      } // for u
    }

    RlifoFlush(RLIFO);

    RLIFOt = RLIFOb;
    RLIFOb = RLIFO;
    RLIFO = RLIFOt;

#ifdef VERBOSE
    fprintf(stderr, "%s: %d collapses\n", F_NAME, ncol);
#endif

  } // while (!RlifoVide(RLIFO) && nsteps > 0)

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  RlifoTermine(RLIFO);
  RlifoTermine(RLIFOb);
  return 1;

} /* l3dpardircollapse() */

/* =============================================================== */
int32_t l3ddetectdyncollapse(struct xvimage * k, int32_t nsteps, struct xvimage * inhibit, int32_t dddim)
/* =============================================================== */
/* 
  collapse parall��le directionnel
  sans fonction de priorit��
  detection et pr��servation d'��l��ments de dimension dddim
*/
#undef F_NAME
#define F_NAME "l3ddetectdyncollapse"
{
  int32_t u, n;
  index_t g, f, xf, yf, zf, xg, yg, zg;
  index_t i, rs, cs, ps, ds, N;
  int32_t dim, ori, dir, direc, orien;
  uint8_t * K;
  uint8_t * I = NULL;
  Rlifo * RLIFO;
  Rlifo * RLIFOb;
  Rlifo * RLIFOt;
  index_t taillemax;
  index_t tab[GRS3D*GCS3D*GDS3D];
  uint32_t nbcol;

  assert((dddim>=1) && (dddim<=3));

  rs = rowsize(k);
  cs = colsize(k);
  ds = depth(k);
  ps = rs * cs;
  N = ps * ds;
  K = UCHARDATA(k);

  IndicsInit(N);

  if (inhibit != NULL)
  {
    if ((rowsize(inhibit) != rs) || (colsize(inhibit) != cs) || (depth(inhibit) != ds))
    {
      fprintf(stderr, "%s : bad size for inhibit\n", F_NAME);
      return(0);
    }
    if (datatype(inhibit) == VFF_TYP_1_BYTE) 
      I = UCHARDATA(inhibit); 
    else 
    {
      fprintf(stderr, "%s : datatype(inhibit) must be uint8_t\n", F_NAME);
      return(0);
    }
  }
  else
  {
    fprintf(stderr, "%s : inhibit image must be allocated\n", F_NAME);
    return(0);
  }

  taillemax = 4 * (rs*cs + cs*ds + ds*rs);
  RLIFO = CreeRlifoVide(taillemax);
  RLIFOb = CreeRlifoVide(taillemax);
  if ((RLIFO == NULL) || (RLIFOb == NULL))
  {
    fprintf(stderr, "%s : CreeRlifoVide failed\n", F_NAME);
    return(0);
  }

  if (nsteps == -1) nsteps = 1000000000;

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  /* ========================================================= */
  /* INITIALISATION DE LA RLIFO ET DE LA "BORDER" */
  /* ========================================================= */

  for (zg = 0; zg < ds; zg++)
  for (yg = 0; yg < cs; yg++)
  for (xg = 0; xg < rs; xg++)
  {
    g = zg*ps + yg*rs + xg;
    if (K[g] && ((I == NULL) || (!I[g])))
    {
      f = PaireLibre3d(k, xg, yg, zg);
      if (f != -1) 
      { 
	RlifoPush(&RLIFO, f); 
	RlifoPush(&RLIFO, g);
	Set(g, EN_RLIFO);
	xf = f % rs; yf = (f % ps) / rs; zf = f / ps;
	Alphacarre3d(rs, cs, ds, xf, yf, zf, tab, &n);
	for (u = 0; u < n; u += 1)
	{
	  g = tab[u];
	  Set(g, BORDER);
	} // for u
      }
    }
  }

  /* ================================================ */
  /*              DEBUT BOUCLE PRINCIPALE             */
  /* ================================================ */

  nbcol = 1;
  while (!RlifoVide(RLIFO) && nsteps > 0 && nbcol)
  {
    nsteps --;
    nbcol = 0;

    for (dir = 0; dir <= 2; dir++) // For all face directions
      for (ori = 0; ori <= 1; ori++) // For both orientations
      {
	for (dim = 3; dim >= 1; dim--) // For dimensions in decreasing order
	{
	  for (i = 0; i < RLIFO->Sp; i += 2) // Scan the free faces list (dim 3)
	  {
	    f = RLIFO->Pts[i];
	    g = RLIFO->Pts[i+1];
	    xf = f % rs; yf = (f % ps) / rs; zf = f / ps;
	    if (K[f] && K[g] && ((I == NULL) || (!I[g] && !I[f])))
	    {
	      xg = g % rs; yg = (g % ps) / rs; zg = g / ps;  
	      if (xf - xg)      { direc = 0; if (xf > xg) orien = 0; else orien = 1; }
	      else if (yf - yg) { direc = 1; if (yf > yg) orien = 0; else orien = 1; }
	      else              { direc = 2; if (zf > zg) orien = 0; else orien = 1; }
	      if ((DIM3D(xf,yf,zf) == dim) && (direc == dir) && (orien == ori))
	      {
		K[g] = K[f] = VAL_NULLE;
		nbcol++;
		// Pr��paration sous-��tapes suivantes
		Alphacarre3d(rs, cs, ds, xf, yf, zf, tab, &n);
		for (u = 0; u < n; u += 1)
		{
		  g = tab[u];
		  xg = g % rs; yg = (g % ps) / rs; zg = g / ps;
		  if (K[g] && ((I == NULL) || (!I[g])))
		  {
		    f = PaireLibre3d(k, xg, yg, zg);
		    if ((f != -1) && IsSet(f, BORDER) && !IsSet(g, EN_RLIFO))
		    { 
		      RlifoPush(&RLIFOb, f); 
		      RlifoPush(&RLIFOb, g);
		    }
		  }
		} // for u
	      } // if ((DIM3D(xf,yf,zf) == dim) &&...
	    } // if (K[f] && K[g])
	  } // for (i = 0; i < RLIFO->Sp; i += 2)
	  while (!RlifoVide(RLIFOb))
	  { 
	    g = RlifoPop(RLIFOb);
	    f = RlifoPop(RLIFOb);
	    RlifoPush(&RLIFO, f); 
	    RlifoPush(&RLIFO, g); 
	  }
      } // for (dim = 3; dim >= 1; dim--)
    } // for for

    // PREPARATION ETAPE SUIVANTE
    for (i = 0; i < RLIFO->Sp; i++) UnSet(RLIFO->Pts[i], EN_RLIFO);
    if (dddim < 3)
    for (i = 0; i < RLIFO->Sp; i += 2)
    {
      f = RLIFO->Pts[i];
      UnSet(f, BORDER);
      xf = f % rs; yf = (f % ps) / rs; zf = f / ps;
      Alphacarre3d(rs, cs, ds, xf, yf, zf, tab, &n);
      for (u = 0; u < n; u += 1)
      {
	g = tab[u];
	UnSet(g, BORDER);
	xg = g % rs; yg = (g % ps) / rs; zg = g / ps;
	if (K[g] && (DIM3D(xg,yg,zg) == dddim) && BetaTerminal3d(K, rs, cs, ds, xg, yg, zg))
	  I[g] = 1;
	else if (K[g] && !IsSet(g, EN_RLIFO) && !I[g])
	{
	  f = PaireLibre3d(k, xg, yg, zg);
	  if (f != -1) 
	  { 
	    RlifoPush(&RLIFOb, f); 
	    RlifoPush(&RLIFOb, g);
	    Set(g, EN_RLIFO);
	  }
	}
      } // for u
    } // for i
    else // dddim==3
    for (i = 0; i < RLIFO->Sp; i += 2)
    {
      f = RLIFO->Pts[i];
      UnSet(f, BORDER);
      xf = f % rs; yf = (f % ps) / rs; zf = f / ps;
      Alphacarre3d(rs, cs, ds, xf, yf, zf, tab, &n);
      for (u = 0; u < n; u += 1)
      {
	g = tab[u];
	UnSet(g, BORDER);
	xg = g % rs; yg = (g % ps) / rs; zg = g / ps;
	if (K[g] && ((DIM3D(xg,yg,zg) == 1)||(DIM3D(xg,yg,zg) == 2)) && 
	            BetaTerminal3d(K, rs, cs, ds, xg, yg, zg))
	  I[g] = 1;
	else if (K[g] && !IsSet(g, EN_RLIFO) && !I[g])
	{
	  f = PaireLibre3d(k, xg, yg, zg);
	  if (f != -1) 
	  { 
	    RlifoPush(&RLIFOb, f); 
	    RlifoPush(&RLIFOb, g);
	    Set(g, EN_RLIFO);
	  }
	}
      } // for u
    } // for i

    for (i = 0; i < RLIFOb->Sp; i += 2)
    {
      f = RLIFOb->Pts[i];
      xf = f % rs; yf = (f % ps) / rs; zf = f / ps;
      Alphacarre3d(rs, cs, ds, xf, yf, zf, tab, &n);
      for (u = 0; u < n; u += 1)
      {
	g = tab[u];
	Set(g, BORDER);
      } // for u
    }

    RlifoFlush(RLIFO);

    RLIFOt = RLIFOb;
    RLIFOb = RLIFO;
    RLIFO = RLIFOt;

  } // while (!RlifoVide(RLIFO) && nsteps > 0)

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  RlifoTermine(RLIFO);
  RlifoTermine(RLIFOb);
  return 1;

} /* l3ddetectdyncollapse() */

// 888888888888888888888888888888888888888888888888888888888888888888
// 888888888888888888888888888888888888888888888888888888888888888888
//                        TOPOLOGICAL FLOW
// 888888888888888888888888888888888888888888888888888888888888888888
// 888888888888888888888888888888888888888888888888888888888888888888
 
/* =============================================================== */
graphe * l3dtopoflow_l(struct xvimage * k, struct xvimage * prio, struct xvimage * inhibit, float priomax)
/* =============================================================== */
/* 
  construction du flot topologique associ�� �� un collapse parall��le directionnel
  (voir l3dpardircollapse_f)
  fonction de priorit�� en entiers longs
*/
#undef F_NAME
#define F_NAME "l3dtopoflow_l"
{
  fprintf(stderr, "%s: Not Yet Implemented\n", F_NAME);
  return(NULL);
} /* l3dtopoflow_l() */
 
/* =============================================================== */
graphe * l3dtopoflow_f(struct xvimage * k, struct xvimage * prio, struct xvimage * inhibit, float priomax)
/* =============================================================== */
/* 
  construction du flot topologique associ�� �� un collapse parall��le directionnel
  (voir l3dpardircollapse_f)
  fonction de priorit�� en flottants
  le r��sultat est un graphe dont les sommets (faces du complexe k) 
  sont valu��s par :
    TF_NOT_IN_I: la face n'appartient pas au complexe initial (avant collapse)
    TF_HEAD: la face est une tete de paire libre (face libre)
    TF_TAIL: la face est une queue de paire libre
    TF_PERMANENT: la face appartient toujours au complexe final (apr��s collapse)
*/
#undef F_NAME
#define F_NAME "l3dtopoflow_f"
{
  int32_t u, n;
  index_t g, gg, f, ff, xf, yf, zf, xg, yg, zg;
  index_t i, rs, cs, ds, ps, N;
  int32_t dim, ori, dir, direc, orien;
  uint8_t * K;
  float * P;
  uint8_t * I = NULL;
  Rbt * RBT;
  Rlifo * RLIFO;
  Rlifo * RLIFOb;
  index_t taillemaxrbt;
  index_t tab[GRS3D*GCS3D*GDS3D];
  TypRbtKey p, pp;
  graphe * flow = NULL;
  int32_t narcs, ncoll = 0;

  rs = rowsize(k);
  cs = colsize(k);
  ds = depth(k);
  ps = rs * cs;
  N = ps * ds;
  K = UCHARDATA(k);

  IndicsInit(N);

  if (prio == NULL)
  {
    fprintf(stderr, "%s : prio is needed\n", F_NAME);
    return(NULL);
  }

  if ((rowsize(prio) != rs) || (colsize(prio) != cs) || (depth(prio) != ds))
  {
    fprintf(stderr, "%s : bad size for prio\n", F_NAME);
    return(NULL);
  }
  if (datatype(prio) == VFF_TYP_FLOAT) 
    P = FLOATDATA(prio); 
  else 
  {
    fprintf(stderr, "%s : datatype(prio) must be float\n", F_NAME);
    return(NULL);
  }

  if (inhibit != NULL)
  {
    if ((rowsize(inhibit) != rs) || (colsize(inhibit) != cs) || (depth(inhibit) != ds))
    {
      fprintf(stderr, "%s : bad size for inhibit\n", F_NAME);
      return(NULL);
    }
    if (datatype(inhibit) == VFF_TYP_1_BYTE) 
      I = UCHARDATA(inhibit); 
    else 
    {
      fprintf(stderr, "%s : datatype(inhibit) must be uint8_t\n", F_NAME);
      return(NULL);
    }
  }

  taillemaxrbt = N/8;
  /* cette taille est indicative, le RBT est realloue en cas de depassement */
  RBT = mcrbt_CreeRbtVide(taillemaxrbt);
  if (RBT == NULL)
  {
    fprintf(stderr, "%s : mcrbt_CreeRbtVide failed\n", F_NAME);
    return(NULL);
  }

  RLIFO = CreeRlifoVide(taillemaxrbt);
  RLIFOb = CreeRlifoVide(taillemaxrbt);
  if ((RLIFO == NULL) || (RLIFOb == NULL))
  {
    fprintf(stderr, "%s : CreeRlifoVide failed\n", F_NAME);
    return(NULL);
  }

  /* ================================================ */
  /*               INITIALISATION GRAPHE              */
  /* ================================================ */

  narcs = 0; // evalue la taille max du graphe
  for (zg = 0; zg < ds; zg++)
  for (yg = 0; yg < cs; yg++)
  for (xg = 0; xg < rs; xg++)
  {
    if (K[zg*ps + yg*rs + xg]) 
    { 
      if (CUBE3D(xg,yg,zg)) narcs += 26;
      else if (CARRE3D(xg,yg,zg)) narcs += 8;
      else if (INTER3D(xg,yg,zg)) narcs += 2;
    }
  }
  flow = InitGraphe(N, narcs); // toutes les faces (dans K ou non) sont des sommets
  if (flow == NULL)
  {
    fprintf(stderr, "%s : InitGraphe failed\n", F_NAME);
    return(NULL);
  }
  for (zg = 0; zg < ds; zg++)
  for (yg = 0; yg < cs; yg++)
  for (xg = 0; xg < rs; xg++)
  {
    g = zg*ps + yg*rs + xg;
    flow->x[g] = xg; // coordonn��es des sommets
    flow->y[g] = yg;
    flow->z[g] = zg;
    if (!K[g]) flow->v_sommets[g] = TF_NOT_IN_I;
  }

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  /* ========================================================= */
  /*   INITIALISATION DU RBT */
  /* ========================================================= */

  for (zg = 0; zg < ds; zg++)
  for (yg = 0; yg < cs; yg++)
  for (xg = 0; xg < rs; xg++)
  {
    g = zg*ps + yg*rs + xg;
    if (K[g])
    {
      f = PaireLibre3d(k, xg, yg, zg);
      if ((f != -1) && 
	  (((I != NULL) && (!I[g] && !I[f])) || 
	   ((I == NULL) && (P[g] < priomax) && (P[f] < priomax)) ) )
      {
	pp = (TypRbtKey)(mcmax(P[g],P[f]));
	mcrbt_RbtInsert(&RBT, pp, g);
	Set(g, EN_RBT);
      }
    }
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  while (!mcrbt_RbtVide(RBT))
  {
    // construit la liste de toutes les paires libres ayant la priorit�� courante
    p = RbtMinLevel(RBT); 
    while (!mcrbt_RbtVide(RBT) && (RbtMinLevel(RBT) == p))
    {
      g = RbtPopMin(RBT);
      UnSet(g, EN_RBT);
      xg = g % rs; yg = (g % ps) / rs; zg = g / ps;
      f = PaireLibre3d(k, xg, yg, zg);
      if (f != -1)
      {
	RlifoPush(&RLIFO, f);
	RlifoPush(&RLIFO, g);
	Set(g, EN_RLIFO);
      }
    } // while (!mcrbt_RbtVide(RBT) && (RbtMinLevel(RBT) == p))

    for (dir = 0; dir <= 2; dir++) // For all face directions
      for (ori = 0; ori <= 1; ori++) // For both orientations
      {
#define OLDVERSION
#ifdef OLDVERSION
	for (dim = 3; dim >= 1; dim--) // For dimensions in decreasing order
#else
	for (dim = 3; dim >= 2; dim--) // For dimensions in decreasing order
#endif
	{
	  for (i = 0; i < RLIFO->Sp; i += 2) // Scan the free faces list
	  {
	    f = RLIFO->Pts[i];
	    g = RLIFO->Pts[i+1];
	    xf = f % rs; yf = (f % ps) / rs; zf = f / ps;
	    if (K[f] && K[g] && 
		(((I != NULL) && (!I[g] && !I[f])) || 
		 ((I == NULL) && (P[g] < priomax) && (P[f] < priomax)) ) )
	    {
	      xg = g % rs; yg = (g % ps) / rs; zg = g / ps;
	      if (xf - xg)      { direc = 0; if (xf > xg) orien = 0; else orien = 1; }
	      else if (yf - yg) { direc = 1; if (yf > yg) orien = 0; else orien = 1; }
	      else              { direc = 2; if (zf > zg) orien = 0; else orien = 1; }
	      if ((DIM3D(xf,yf,zf) == dim) && (direc == dir) && (orien == ori))
	      {
		K[g] = K[f] = VAL_NULLE; // COLLAPSE
		ncoll ++;
#ifdef TRACECOL
		printf("Collapse : %d,%d,%d -> %d,%d,%d\n", xg, yg, zg, xf, yf, zf);
#endif
		flow->v_sommets[g] = TF_HEAD;
      		flow->v_sommets[f] = TF_TAIL;
		AjouteArcValue(flow, g, f, (TYP_VARC)1);
#ifdef TRACECONSGRAPH
		printf("Ajoute arc : %d [%d,%d,%d] -> %d [%d,%d,%d] (%g)\n", g, xg, yg, zg, f, xf, yf, zf, (TYP_VARC)1);
#endif
		Alphacarre3d(rs, cs, ds, xf, yf, zf, tab, &n);
		for (u = 0; u < n; u += 1)
		{
		  gg = tab[u];
		  xg = gg % rs; yg = (gg % ps) / rs; zg = gg / ps;
#define OLDVERSION1
#ifdef OLDVERSION1
		  if (K[gg]) 
		  {
#ifdef TRACECONSGRAPH
		    printf("Ajoute arc : %d [%d,%d,%d] -> %d [%d,%d,%d] (%g)\n", f, xf, yf, zf, gg, xg, yg, zg, (TYP_VARC)1);
#endif
		    AjouteArcValue(flow, f, gg, (TYP_VARC)1);
		  }
#else
		  if (K[gg] && !SINGL3D(xg,yg,zg)) 
		  {
#ifdef TRACECONSGRAPH
		    printf("Ajoute arc : %d [%d,%d,%d] -> %d [%d,%d,%d] (%g)\n", f, xf, yf, zf, gg, xg, yg, zg, (TYP_VARC)1);
#endif
		    AjouteArcValue(flow, f, gg, (TYP_VARC)1);
		  }
#endif
		  if (K[gg] && 
		      (((I != NULL) && (!I[gg])) || ((I == NULL) && (P[gg] < priomax))) )
		  {
		    ff = PaireLibre3d(k, xg, yg, zg);
		    if ((ff != -1) &&
			(((I != NULL) && (!I[ff])) || ((I == NULL) && (P[ff] < priomax))) )
		    {
		      if ((P[gg] <= p) && (P[ff] <= p) && !IsSet(gg, EN_RLIFO))
		      { // Pr��paration sous-��tapes suivantes
			RlifoPush(&RLIFOb, ff);
			RlifoPush(&RLIFOb, gg);
		      }
		      if (!IsSet(gg, EN_RBT))
		      { // Pr��paration ��tape suivante
			pp = (TypRbtKey)(mcmax(P[gg],P[ff]));
			mcrbt_RbtInsert(&RBT, pp, gg);
			Set(gg, EN_RBT);
		      }
		    } // if (ff != -1)
		  } // if K[gg] && ...
		} // for u
	      } // if ((DIM3D(xf,yf,zf) == dim) &&...
	    } // if (K[f] && K[g])
	  } // for (i = 0; i < RLIFO->Sp; i += 2)
	  while (!RlifoVide(RLIFOb))
	  { 
	    g = RlifoPop(RLIFOb);
	    f = RlifoPop(RLIFOb);
	    RlifoPush(&RLIFO, f); 
	    RlifoPush(&RLIFO, g); 
	  }
      } // for (dim = 3; dim >= 1; dim--)
    } // for for

    RlifoFlush(RLIFO);

  } /* while (!mcrbt_RbtVide(RBT)) */

  for (g = 0; g < N; g++)
    if (K[g]) flow->v_sommets[g] = TF_PERMANENT;

#ifdef VERBOSE
  fprintf(stderr, "%s: Fin traitement\n", F_NAME);
#endif

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  mcrbt_RbtTermine(RBT);
  RlifoTermine(RLIFO);
  RlifoTermine(RLIFOb);
  return(flow);

} // l3dtopoflow_f()

#define FS_ALPHA 0.1

/* =============================================================== */
int32_t l3dflowskeleton(struct xvimage * k, int32_t mode, double level, struct xvimage * func) 
/* =============================================================== */
#undef F_NAME
#define F_NAME "l3dflowskeleton"
{
  struct xvimage * kk;
  struct xvimage * dist;
  struct xvimage * lambda;
  uint8_t *K;
  index_t rs, cs, ds, ps, N, i, x, y, z;
  float * FUNC;
  float * LAMBDA;
  graphe * flow;
  boolean * perm;
  boolean * head;
  TYP_VSOM vmax;
  TYP_VSOM alpha = FS_ALPHA;

  ONLY_3D(k);
  COMPARE_SIZE(k, func);
  ACCEPTED_TYPES1(k, VFF_TYP_1_BYTE);
  ACCEPTED_TYPES1(func, VFF_TYP_FLOAT);

  rs = rowsize(k);
  cs = colsize(k);
  ds = depth(k);
  ps = rs * cs;
  N = ps * ds;
  K = UCHARDATA(k);

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  assert(dist != NULL);
  if (! lsedt_meijster(k, dist))
  {
    fprintf(stderr, "%s: lsedt_meijster failed\n", F_NAME);
    return 0;
  }

  lambda = allocimage(NULL, rs, cs, ds, VFF_TYP_FLOAT);
  assert(lambda != NULL);
  if (!llambdamedialaxis(dist, lambda))
  {
    fprintf(stderr, "%s: llambdamedialaxis failed\n", F_NAME);
    return 0;
  }

  // -----------------------------------------------------------
  // 1ERE ETAPE : CALCULE LE FLOW GRAPH
  // -----------------------------------------------------------
  kk = copyimage(k); // sauve k car l3dtopoflow modifie le complexe
  assert(kk != NULL);
  if (! (flow = l3dtopoflow_f(kk, lambda, NULL, FLT_MAX)))
  {
    fprintf(stderr, "%s: function l3dtopoflow_f failed\n", F_NAME);
    return 0;
  }
  freeimage(kk);

  perm = (boolean *)calloc(N, sizeof(boolean)); assert(perm != NULL);
  head = (boolean *)calloc(N, sizeof(boolean)); assert(head != NULL);
  for (i = 0; i < N; i++)
    if (flow->v_sommets[i] == TF_PERMANENT)
      perm[i] = TRUE;
    else if (flow->v_sommets[i] == TF_HEAD)
      head[i] = TRUE;
  
  // -------------------------------------------------------------------
  // 2EME ETAPE : CALCULE LA FONCTION DE POIDS INITIALE POUR LES SOMMETS
  // -------------------------------------------------------------------
  LAMBDA = FLOATDATA(lambda);
  FUNC = FLOATDATA(func);

  if (mode == 0)
  { // fonction uniform��ment nulle 
    razimage(func);
  }
  else if (mode == 1)
  { // fonction uniforme (unit��)
    for (i = 0; i < N; i++) 
      if (K[i])
	FUNC[i] = (float)1;
      else
	FUNC[i] = (float)0;
  }
  else if ((mode == 2) || (mode == 3))
  { // fonction uniforme sur la fronti��re, nulle �� l'int��rieur 
    struct xvimage * border = copyimage(k);
    uint8_t *B;
    assert(border != NULL);
    if (! l3dborder(border))
    {
      fprintf(stderr, "%s: function l3dborder failed\n", F_NAME);
      return 0;
    }
    B = UCHARDATA(border);
    for (i = 0; i < N; i++) 
      if (B[i]) 
	FUNC[i] = (float)1;
      else
	FUNC[i] = (float)0;
    freeimage(border);
  }
  else if (mode == 4)
  { // fonction d'ouverture invers��e
    int32_t ret;
    uint32_t *OF, maxof;
    struct xvimage *of = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
    assert(of != NULL);
    ret = lopeningfunction(k, of, 0); assert(ret == 1);
    OF = ULONGDATA(of);
    maxof = OF[0];
    for (i = 0; i < N; i++) if (OF[i] > maxof) maxof = OF[i];
    for (i = 0; i < N; i++) FUNC[i] = (float)(maxof - OF[i]) + 0.1;
    freeimage(of);
  }
  else if (mode == 5)
  { // fonction bisectrice
    if (! lmedialaxis_lbisector(dist, k, func))
    {
      fprintf(stderr, "%s: lmedialaxis_lbisector failed\n", F_NAME);
      return 0;
    }
    MaxAlpha3d(func); // fermeture (en ndg)
  }
  else if (mode == 6)
  { // distance map
    uint32_t *D = ULONGDATA(dist);
    for (i = 0; i < N; i++) FUNC[i] = (float)(1.0/sqrt(D[i]));
  }
  else if (mode == 7)
  { // lambda function
    copy2image(func, lambda);
    MaxAlpha3d(func); // fermeture (en ndg)
  }
  else if (mode == 8)
  { // fonction uniforme (unit��) sur les facettes
    for (z = 0; z < ds; z++) 
    for (y = 0; y < cs; y++) 
    for (x = 0; x < rs; x++)
    {
      i = z*ps + y*rs + x;
      if (K[i] && CUBE3D(x,y,z))
	FUNC[i] = (float)1;
      else
	FUNC[i] = (float)0;
    }
  }
  else if (mode == 9)
  { // fonction uniforme sur les facettes de la fronti��re
    struct xvimage * border = copyimage(k);
    uint8_t *B;
    assert(border != NULL);
    if (! l3dborder(border))
    {
      fprintf(stderr, "%s: function l3dborder failed\n", F_NAME);
      return 0;
    }
    B = UCHARDATA(border);
    for (z = 0; z < ds; z++) 
    for (y = 0; y < cs; y++) 
    for (x = 0; x < rs; x++)
    {
      i = z*ps + y*rs + x;
      if (B[i] && CARRE3D(x,y,z)) 
	FUNC[i] = (float)1;
      else
	FUNC[i] = (float)0;
    }
    freeimage(border);
  }
  else
  {
    fprintf(stderr, "%s: bad mode: %d\n", F_NAME, mode);
    return 0;
  }

  for (i = 0; i < N; i++)
    flow->v_sommets[i] = (TYP_VSOM)FUNC[i];

  // -----------------------------------------------------------
  // 3EME ETAPE : INTEGRE LA FONCTION DE POIDS 
  // -----------------------------------------------------------
  IntegreGSC(flow);
    
  if (mode == 3)
  {
#ifdef ANCIEN1
    MaxAlpha3d(lambda); // fermeture (en ndg)
    for (i = 0; i < N; i++)
      flow->v_sommets[i] = flow->v_sommets[i] - (TYP_VSOM)LAMBDA[i];
#endif
#ifdef ANCIEN2
    int32_t *D = SLONGDATA(dist);
    for (i = 0; i < N; i++)
      if (D[i]) 
	flow->v_sommets[i] = flow->v_sommets[i] / (TYP_VSOM)sqrt(D[i]);
#endif
    int32_t ret;
    uint32_t *OF;
    struct xvimage *of = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
    assert(of != NULL);
    ret = lopeningfunction(k, of, 0); assert(ret == 1);
    OF = ULONGDATA(of);
    for (i = 0; i < N; i++)
      if (OF[i]) 
	flow->v_sommets[i] = flow->v_sommets[i] / (TYP_VSOM)OF[i];
    freeimage(of);
  }

  // -----------------------------------------------------------
  // 4EME ETAPE : TRANSFORME LA FONCTION SUR LES SOMMETS EN 
  // FONCTION DE MORSE (INVERS��E) SUR LE COMPLEXE
  // -----------------------------------------------------------  
  AlphaTopologicalMap(flow, head, alpha);

  // met �� vmax (infini) les sommets "permanents" (non collaps��s)
  vmax = flow->v_sommets[0];
  for (i = 0; i < N; i++)
    if (flow->v_sommets[i] > vmax) vmax = flow->v_sommets[i];
  for (i = 0; i < N; i++)
    if (perm[i])
      flow->v_sommets[i] = vmax;
  
  if (level >= 0)
  {
    uint8_t *KK;
    graphe * flow_s;
    uint32_t j;

    for (i = 0; i < N; i++)
      K[i] = (flow->v_sommets[i] >= level ? 255 : 0);
    writeimage(k, "_seuil_level");

    // detects end points
    if (!l3dseltype(k, 0, 0, 0, 0, 1, 1))
    {
      fprintf(stderr, "%s: function l3dseltype failed\n", F_NAME);
      return 0;
    }
    writeimage(k, "_endpoints");    

    kk = copyimage(k); 
    assert(kk != NULL);
    razimage(kk);

    // foreach end point, compute the upstream
    KK     = UCHARDATA(kk);
    flow_s = Symetrique(flow);

    for (i = 0; i < N; i++)
      if (K[i])
      {
	boolean *D = Descendants(flow_s, i);
	for (j = 0; j < N; j++) if (D[j]) KK[j] = 255;
	free(D);
      }
    writeimage(kk, "_upstreams");    
    TermineGraphe(flow_s);

    // computes a new weighting function:
    // identical to the previous one on upstreams,
    // null elsewhere
    for (i = 0; i < N; i++)
      if (!KK[i])
	flow->v_sommets[i] = (TYP_VSOM)0;
    
    // "morsifies" this function
    AlphaTopologicalMap(flow, head, alpha);
    vmax = flow->v_sommets[0];
    for (i = 0; i < N; i++)
      if (flow->v_sommets[i] > vmax) vmax = flow->v_sommets[i];
    for (i = 0; i < N; i++)
      if (perm[i])
	flow->v_sommets[i] = vmax;
  }
   
  for (i = 0; i < N; i++)
    FUNC[i] = (float)flow->v_sommets[i];

  freeimage(dist);
  freeimage(lambda);
  free(perm);
  free(head);
  TermineGraphe(flow);

  return 1;
} // l3dflowskeleton()
