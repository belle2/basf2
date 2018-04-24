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

  /** coeffieients for N=3 for vector form factor f_+   */
  double ap_0{0};

  double ap_1{0};

  double ap_2{0};

  double ap_3{0};

  /** coeffieients for N=3 for scalar form factor f_0   */
  double a0_0{0};

  double a0_1{0};

  double a0_2{0};

  double a0_3{0};

};
#endif









