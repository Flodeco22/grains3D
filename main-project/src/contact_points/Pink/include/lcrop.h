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
#ifdef __cplusplus
extern "C" {
#endif

extern struct xvimage * lcrop(struct xvimage *in, int32_t x, int32_t y, int32_t w, int32_t h);
extern struct xvimage * lcrop3d(struct xvimage *in, int32_t x, int32_t y, int32_t z, int32_t w, int32_t h, int32_t d);
extern struct xvimage * lautocrop(struct xvimage *in, double seuil);
extern void lautocrop2(struct xvimage *in, double seuil, index_t *Xmin, index_t *Ymin, index_t *Zmin, index_t *w, index_t *h, index_t *p);
extern void lsetframe(struct xvimage *image, int32_t grayval);
extern void lsetthickframe(struct xvimage *image, int32_t width, int32_t grayval);
extern struct xvimage * lenframe(struct xvimage *image, int32_t grayval, int32_t width);
extern struct xvimage * lenframe3d(struct xvimage *image, int32_t grayval, int32_t width);
extern int32_t linsert(struct xvimage *a, struct xvimage *b, int32_t x, int32_t y, int32_t z);
extern struct xvimage * lexpandframe(struct xvimage *image1, int32_t n);

#ifdef __cplusplus
}
#endif
