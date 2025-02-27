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
/*! \file histstretch.c

\brief transforms the gray scale by stretching the histogram

<B>Usage:</B> histstretch in.pgm vmin vmax p out.pgm

<B>Description:</B>

Eliminates the levels of the histogram strictly less than <B>vmin</B> or 
strictly greater than <B>vmax</B>, and also the extremal levels
that have strictly less than N*(<B>p</B>/100) pixels 
(N being the total number of pixels). If <B>p</B> = 0, only the 
extremal levels with 0 pixels are eliminated.
A linear histogram scaling is then performed.

<B>Types supported:</B> byte 2D

<B>Category:</B> histo
\ingroup  histo

\author Michel Couprie
*/

/* Michel Couprie - novembre 2000 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <lhistscal.h>

//#define DEBUG

/* =============================================================== */
int main(int argc, char **argv)
/* =============================================================== */
{
  struct xvimage * image;
  int32_t i;
  int32_t rs, cs, d, N;
  uint8_t *SOURCE;      /* l'image de depart */
  int32_t histo[256];
  int32_t vmin, vmax;
  double p;

  if (argc != 6)
  {
    fprintf(stderr, "usage: %s in.pgm vmin vmax p out.ppm \n", argv[0]);
    exit(1);
  }

  image = readimage(argv[1]);
  if (image == NULL)
  {
    fprintf(stderr, "histstretch: readimage failed\n");
    exit(1);
  }
  rs = rowsize(image);     /* taille ligne */
  cs = colsize(image);     /* taille colonne */
  d = depth(image);        /* nombre plans */
  N = rs * cs * d;         /* taille image */
  SOURCE = UCHARDATA(image);      /* l'image de depart */
  
  vmin = atoi(argv[2]);
  vmax = atoi(argv[3]);
  p = atof(argv[4]);

  for (i = 0; i <= NDG_MAX; i++) histo[i] = 0;
  for (i = 0; i < N; i++) histo[SOURCE[i]] += 1;

  /* elimine les niveaux < vmin ou > vmax */
  for (i = 0; i < vmin; i++) histo[i] = 0;
  for (i = vmax+1; i <= NDG_MAX; i++) histo[i] = 0;

  /* elimine les niveaux extremes qui ont 0 pixels */
  while(histo[vmin] == 0) vmin++;
  while(histo[vmax] == 0) vmax--; 

  /* elimine les niveaux extremes qui ont moins de N/(100/p) pixels */
#ifdef DEBUG
printf("N/(100/p) = %g\n", (N*p)/100);
#endif
  while(histo[vmin] < (N*p)/100) vmin++;
  while(histo[vmax] < (N*p)/100) vmax--; 
#ifdef DEBUG
printf("vmin = %d ; vmax = %d\n", vmin, vmax);
#endif

  if (! lhistscal(image, vmin, NDG_MIN, vmax, NDG_MAX))
  {
    fprintf(stderr, "histscal: function lhistscal failed\n");
    exit(1);
  }

  writeimage(image, argv[argc-1]);
  freeimage(image);

  return 0;
} /* main */
