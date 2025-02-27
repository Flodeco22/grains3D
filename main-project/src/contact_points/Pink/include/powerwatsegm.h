/*Copyright ESIEE (2009) 

Author :
Camille Couprie (c.couprie@esiee.fr)

Contributors : 
Hugues Talbot (h.talbot@esiee.fr)
Leo Grady (leo.grady@siemens.com)
Laurent Najman (l.najman@esiee.fr)

This software contains some image processing algorithms whose purpose is to be
used primarily for research.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  use, 
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




//#define SPEEDY
#ifndef POWERWATSEGM_H
#define POWERWATSEGM_H


#ifdef __cplusplus
extern "C" {
#endif



  //  const double epsilon=0.00001;
  //  const index_t MAX_DEGREE=3000;
  // // #define SIZE_MAX_PLATEAU 2000000
typedef float DBL_TYPE;
#include <stdint.h>
#include <mclifo.h>
extern int powerwatsegm(int algo, char * image_name, char * seeds_name, bool geod) ;

    int compute_power_watershed_col(struct xvimage *image_r,
                                    struct xvimage *image_g,
                                    struct xvimage *image_b,
                                    struct xvimage *seeds,
                                    bool geodesic,
                                    bool mult,
                                    struct xvimage *output    ) ;

    int compute_power_watershed_bw(struct xvimage *image_r,
                                   struct xvimage *seeds,
                                   bool geodesic,
                                   bool mult,
                                   struct xvimage *output    ) ;   

#ifdef __cplusplus
}
#endif


    
#endif // POWERWATSEGM_H
