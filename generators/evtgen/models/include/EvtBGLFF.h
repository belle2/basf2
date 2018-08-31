/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Lu Cao                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EvtHQET3FF_HH
#define EvtHQET3FF_HH

#include "EvtGenBase/EvtSemiLeptonicFF.hh"

class EvtId;

/** The class provides the form factors for semileptonic D decays with full mass dependence
 */
class EvtBGLFF : public EvtSemiLeptonicFF {

public:

  /** Default constructor */
  EvtBGLFF(double bglap_0, double bglap_1, double bglap_2, double bglap_3, double bgla0_0, double bgla0_1, double bgla0_2,
           double bgla0_3);


  /** Returns scalar ffs */
  void getscalarff(EvtId parent, EvtId daught,
                   double t, double mass, double* fp, double* f0);

  /** Returns vector ffs */
  void getvectorff(EvtId, EvtId, double, double, double*,
                   double*, double*, double*);

  /** Returns tensor ffs */
  void gettensorff(EvtId, EvtId, double, double, double*,
                   double*, double*, double*);

  /** Returns baryon ffs */
  void getbaryonff(EvtId, EvtId, double, double, double*,
                   double*, double*, double*);

  /** Returns dirac ffs */
  void getdiracff(EvtId, EvtId, double, double, double*, double*,
                  double*, double*, double*, double*);

  /** Returns tarita ffs */
  void getraritaff(EvtId, EvtId, double, double, double*, double*,
                   double*, double*, double*, double*, double*, double*);

private:


  /** ai_n (i = p ---vector, 0 ---scalar; n = 0,1,2,3) are free coeffieients of z expansion
      in dispersion relation parametrization from
      C.G.Boyd, B.Grinstein, R.F.Lebed, Phys. Rev. Lett. 74,4603(1995)

      Chosen the order of series N=3, i.e.
      a_0 + a_1 * z + a_2 * z^2 + a_3 * z^3

      Fitted values cited from
      R.Glattauer, etc. (Belle) Phys. Rev. D 93,032006 (2016).

   **/

  /** z expansion coeffieients for vector form factor: f_+  */

  double ap_0{0};

  double ap_1{0};

  double ap_2{0};

  double ap_3{0};

  /** z expansion coeffieients for scalar form factor f_0   */
  double a0_0{0};

  double a0_1{0};

  double a0_2{0};

  double a0_3{0};

};
#endif









