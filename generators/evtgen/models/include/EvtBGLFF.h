/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Lu Cao and Chaoyi Lyu                                    *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EvtHQET3FF_HH
#define EvtHQET3FF_HH

#include "EvtGenBase/EvtSemiLeptonicFF.hh"

class EvtId;

/** The class provides the form factors for semileptonic D and D* decays with full mass dependence
 */
class EvtBGLFF : public EvtSemiLeptonicFF {

public:

  /** Default constructor */
  EvtBGLFF(double bglap_0, double bglap_1, double bglap_2, double bglap_3, double bgla0_0, double bgla0_1, double bgla0_2,
           double bgla0_3);

  /** Default constructor */
  EvtBGLFF(double bgla_0, double bgla_1, double bglb_0, double bglb_1, double bglc_1, double bglc_2);

  /** Returns scalar ffs */
  void getscalarff(EvtId parent, EvtId daught,
                   double t, double mass, double* fp, double* f0);

  /** Returns vector ffs */
  void getvectorff(EvtId parent, EvtId daught, double t, double mass, double* a1f,
                   double* a2f, double* vf, double* a0f);

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


  /** B -> Dlnu:
      ai_n (i = p ---vector, 0 ---scalar; n = 0,1,2,3) are free coefficients of z expansion
      in dispersion relation parametrization from
      C.G.Boyd, B.Grinstein, R.F.Lebed, Phys. Rev. Lett. 74,4603(1995)

      Chosen the order of series N=3, i.e.
      a_0 + a_1 * z + a_2 * z^2 + a_3 * z^3

      Fitted values cited from
      R.Glattauer, etc. (Belle) Phys. Rev. D 93,032006 (2016).

      B -> D*lnu:
      a_n, b_n (n = 0,1) and c_n (n = 0,1,2) are free coefficients of z expansion parametrization from
      C.G.Boyd, B.Grinstein and R.F.Lebed, Phys. Rev. D 56,6895(1997) &
      B.Grinstein, A.Kobach, Phys. Lett. B 771(2017)359-364

      For the expansion of form factors g and f, the order of series N=1, i.e.
      a_0 + a_1*z
      For the expansion of form factors F1, the order of series N=2, i.e.
      c_0 + c_1 * z + c_2 * z**2
      (g, f and F1 are the sub-terms of helicity amplitude)

      Fitted values are taken from a private discussion of Florian Bernlochner based on
      B.Grinstein and A.Kobach, Phys. Lett. B 771(2017)359-364


   **/

  /** 0th-order z expansion coeffieient for vector form factor: f_+  */
  double ap_0{0};

  /** 1st-order z expansion coeffieient for vector form factor: f_+  */
  double ap_1{0};

  /** 2nd-order z expansion coeffieient for vector form factor: f_+  */
  double ap_2{0};

  /** 3rd-order z expansion coeffieient for vector form factor: f_+  */
  double ap_3{0};



  /** 0th-order z expansion coeffieient for scalar form factor f_0   */
  double a0_0{0};

  /** 1st-order z expansion coeffieient for scalar form factor f_0   */
  double a0_1{0};

  /** 2nd-order z expansion coeffieient for scalar form factor f_0   */
  double a0_2{0};

  /** 3rd-order z expansion coeffieient for scalar form factor f_0   */
  double a0_3{0};


  /** B->D*lnu z expansion coeffieients  */

  /** 0th-order z expansion coeffieient for form factor g   */
  double a_0{0};

  /** 1st-order z expansion coeffieient for form factor g   */
  double a_1{0};

  /** 0th-order z expansion coeffieient for form factor f   */
  double b_0{0};

  /** 1st-order z expansion coeffieient for form factor f   */
  double b_1{0};

  /** 1st-order z expansion coeffieient for form factor F1   */
  double c_1{0};

  /** 2nd-order z expansion coeffieient for form factor F1   */
  double c_2{0};

};
#endif









