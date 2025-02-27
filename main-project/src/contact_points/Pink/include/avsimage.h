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
/*
  Functions to be added to mcimage.h (or not)

  Author: Andre Vital Saude, 2005
*/

//Datatypes
typedef struct AVS_Coordinates {
    int32_t x, y, z;
} AVS_Coordinates;
    
typedef AVS_Coordinates AVS_Point;

typedef struct AVS_PointSet {
  int32_t size;
  AVS_Point points[48];
} AVS_PointSet;

//Image data access
#define AVS_point2d(I, p, rs) (I[(p.y)*rs + (p.x)])
#define AVS_point3d(I, p, ps, rs) (I[(p.z)*ps + (p.y)*rs + (p.x)])
#define AVS_pixel(I, x, y, rs) (I[(y)*(rs) + (x)])
#define AVS_voxel(I, x, y, z, ps, rs) (I[(z)*(ps) + (y)*(rs) + (x)])

//Additional functions
extern int32_t findMaxLong(uint32_t *gg, int32_t n);
extern int32_t samedimsimages(struct xvimage *im1, struct xvimage *im2);
#ifdef __cplusplus
}
#endif
