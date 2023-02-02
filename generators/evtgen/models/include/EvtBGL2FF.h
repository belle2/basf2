/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include "EvtGenBase/EvtSemiLeptonicFF.hh"

class EvtId;

/** The class provides the form factors for semileptonic D and D* decays with full mass dependence
 */
class EvtBGL2FF : public EvtSemiLeptonicFF {

public:

  /** Default constructor */
  EvtBGL2FF(double bglap_0, double bglap_1, double bglap_2, double bglap_3, double bgla0_0, double bgla0_1, double bgla0_2,
            double bgla0_3);

  /** Default constructor */
  EvtBGL2FF(double bgla_0, double bgla_1, double bglb_0, double bglb_1, double bglc_1, double bglc_2);

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
      Bigi, Dante and Gambino, Paolo, Phys. Rev. D. 94.094008 (2016):
      a+0, a+1, a+2, a+3 : 0.01565, -0.0353, -0.043, 0.194
      a00, a01, a02, a03 : 0.07932, -0.214, 0.17, -0.958

      NOTE : This is an alternative BGL model. Use it if you understand the difference with the
      default one called 'EvtBGLFF' in basf2.

      B -> D*lnu:

      This class should not be used for B -> D* l nu decays. Use the BGL class instead.

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