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
extern int32_t lskelpavlidis(struct xvimage *image,
			 int32_t nsteps,
			 struct xvimage *inhibit);
extern int32_t lskelpavlidis1(struct xvimage *image,
			  int32_t pixel);
extern int32_t lskeleckhardt(struct xvimage *image,
			 int32_t nsteps,
			 struct xvimage *inhibit);
extern int32_t lskelmcultime(struct xvimage *image,
			int32_t nsteps,
			struct xvimage *inhibit);
extern int32_t lskelmccurv(struct xvimage *image,
			int32_t nsteps,
			struct xvimage *inhibit);
extern int32_t lskelmccurvrec(struct xvimage *image,
			int32_t nsteps,
			struct xvimage *inhibit);
extern int32_t lskelrutovitz(struct xvimage *image,
			 int32_t nsteps,
			 struct xvimage *inhibit);
extern int32_t lskelzhangwang(struct xvimage *image,
			  int32_t nsteps,
			  struct xvimage *inhibit);
extern int32_t lskelhanlarhee(struct xvimage *image,
			  int32_t nsteps,
			  struct xvimage *inhibit);
extern int32_t lskelguohall(struct xvimage *image,
			int32_t nsteps,
			struct xvimage *inhibit,
			int32_t variante);
extern int32_t lskelchinwan(struct xvimage *image,
			int32_t nsteps,
			struct xvimage *inhibit);
extern int32_t lskeljang(struct xvimage *image,
		     int32_t nsteps,
		     struct xvimage *inhibit);
extern int32_t lskeljangcor(struct xvimage *image,
			int32_t nsteps,
			struct xvimage *inhibit);

extern int32_t lskelmns(struct xvimage *image,
		    int32_t nsteps,
		    struct xvimage *inhibit);
extern int32_t lskeljangrec(struct xvimage *image,
			int32_t nsteps,
			struct xvimage *inhibit);
extern int32_t lskelchoy(struct xvimage *image,
		     int32_t nsteps,
		     struct xvimage *inhibit);
extern int32_t lskelmanz(struct xvimage *image,
		     int32_t nsteps,
		     struct xvimage *inhibit,
		     int32_t variante);
extern int32_t lskelhall(struct xvimage *image,
		     int32_t nsteps,
		     struct xvimage *inhibit,
		     int32_t variante);
extern int32_t lskelwutsai(struct xvimage *image,
		       int32_t nsteps,
		       struct xvimage *inhibit);
extern int32_t lskelNK2(struct xvimage *image,
			      int32_t nsteps,
			      struct xvimage *inhibit);
extern int32_t lskelNK2b(struct xvimage *image,
			      int32_t nsteps,
			      struct xvimage *inhibit);
extern int32_t lskelNK2p(struct xvimage *image,
			 int32_t nsteps,
			 double isthmus_persistence,
			 struct xvimage *inhibit);
extern int32_t lskelbertrand_sym(struct xvimage *image,
			     int32_t nsteps,
			     struct xvimage *inhibit);
extern int32_t lskelbertrand_asym_s(struct xvimage *image,
				int32_t nsteps,
				struct xvimage *inhibit);
extern int32_t lskelMK2(struct xvimage *image,
		    int32_t nsteps,
		    struct xvimage *inhibit);
extern int32_t lskelMK2b(struct xvimage *image,
		    int32_t nsteps,
		    struct xvimage *inhibit);
extern int32_t lskelAK2(struct xvimage *image,
			int32_t nsteps,
			struct xvimage *inhibit);
extern int32_t lskelrosenfeld(struct xvimage *image,
			      int32_t nsteps,
			      struct xvimage *inhibit);
extern int32_t lskelrosenfeld_var1(struct xvimage *image,
			           int32_t nsteps,
			           struct xvimage *inhibit);
extern int32_t lskelrosenfeld_var2(struct xvimage *image,
			           int32_t nsteps,
			           struct xvimage *inhibit);
extern int32_t lskelnemethpalagyi(struct xvimage *image,
					int32_t nsteps,
					struct xvimage *inhibit,
					int32_t type);
extern int32_t lhthinpar(struct xvimage *image, int32_t nsteps);
extern int32_t lhthinpar_asymmetric(struct xvimage *image, int32_t nsteps);
extern int32_t lskelCK2(struct xvimage *image,
			int32_t nsteps,
			struct xvimage *inhibit);
extern int32_t lskelCK2_pers(struct xvimage *image, 
			     struct xvimage *persistence);
extern int32_t lskelNK2_pers(struct xvimage *image, 
			     struct xvimage *persistence);
extern int32_t lskelzhangsuen(struct xvimage *image,
			      int32_t nsteps,
			      struct xvimage *inhibit);
extern int32_t lskelKwonGiKang(struct xvimage *image,
			       int32_t nsteps,
			       struct xvimage *inhibit);
#ifdef __cplusplus
}
#endif
