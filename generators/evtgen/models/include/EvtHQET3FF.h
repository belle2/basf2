/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Florian Bernlochner                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EvtHQET3FF_HH
#define EvtHQET3FF_HH

#include "EvtGenBase/EvtSemiLeptonicFF.hh"

class EvtId;

/** The class provides the form factors for semileptonic D & D* decays with full mass dependence
 */
class EvtHQET3FF : public EvtSemiLeptonicFF {

public:

  /** Default constructor */
  EvtHQET3FF(double hqetha1_1, double hqetrho2, double hqetr0_1, double hqetr1_1, double hqetr2_1);

  /** Default constructor */
  EvtHQET3FF(double hqetv1_1, double hqetrho2, double hqetdelta);

  /** Returns vector ffs */
  void getvectorff(EvtId parent, EvtId daught,
                   double t, double mass, double* a1f,
                   double* a2f, double* vf, double* a0f);

  /** Returns scalar ffs */
  void getscalarff(EvtId parent, EvtId daught,
                   double t, double mass, double* fp, double* f0);

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

  /** Normalization of leading FF */
  double ha1_1{0};

  /** CLN slope of leading FF */
  double rho2{0};

  /** Normalization of R0(w=1) */
  double r0_1{0};

  /** Normalization of R1(w=1) */
  double r1_1{0};

  /** Normalization of R2(w=1) */
  double r2_1{0};

  /** Normalization of leading scalar FF */
  double v1_1{0};

  /** Normalization of scalar FF */
  double d1_1{0};

};

#endif









