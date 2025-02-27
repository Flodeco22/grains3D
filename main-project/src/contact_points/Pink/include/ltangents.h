
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

#ifndef LTANGENTS_H
#define LTANGENTS_H

#ifdef __cplusplus
extern "C" {
#endif

extern void ExtractDSSs(int32_t npoints, int32_t *X, int32_t *Y, double thickness, int32_t *end, double *angle);
extern void ExtractDSSs3D(int32_t npoints, int32_t *X, int32_t *Y, int32_t *Z, double thickness, int32_t *end, double *anglexy, double *angleyz, double *anglexz);
extern int32_t CoverByDSSs(int32_t npoints, int32_t *X, int32_t *Y, double thickness);
extern int32_t CoverByDSSs3D(int32_t npoints, int32_t *X, int32_t *Y, int32_t *Z, double thickness);
extern int32_t FindDSSs3D(int32_t npoints, int32_t *X, int32_t *Y, int32_t *Z, int32_t i, double thickness);
extern void LambdaMSTD(int32_t npoints, int32_t *end, double *angle, double *mstd);
extern void LambdaMSTD3D(int32_t npoints, int32_t *end, double *Xtan, double *Ytan, double *Ztan, double *Xmstd, double *Ymstd, double *Zmstd);
extern double ComputeLength(int32_t npoints, double *mstd);
extern double ComputeLength3D(int32_t npoints, double *mstdxy, double *mstdyz, double *mstdxz);
extern int32_t lcurvetangents2D(int32_t mode, int32_t mask, uint64_t *tab_combi, int32_t npoints, int32_t *X, int32_t *Y, double *Xdir, double *Ydir);
extern int32_t lcurvetangents3D(int32_t mode, int32_t mask, uint64_t *tab_combi, int32_t npoints, int32_t *X, int32_t *Y, int32_t *Z, double *Xdir, double *Ydir, double *Zdir);
extern int32_t lcurvenormalplanes3D(int32_t ngauss, int32_t npoints, int32_t *X, int32_t *Y, int32_t *Z, double *Xdir, double *Ydir, double *Zdir);
#ifdef __cplusplus
}
#endif

#endif // LTANGENTS_H
