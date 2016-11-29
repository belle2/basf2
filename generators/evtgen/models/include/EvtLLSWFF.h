/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Florian Bernlochner                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EvtLLSWFF_HH
#define EvtLLSWFF_HH

#include "EvtGenBase/EvtSemiLeptonicFF.hh"

class EvtId;

/** The class provides the form factors for orbitally excited semileptonic decays
 */
class EvtLLSWFF : public EvtSemiLeptonicFF {

public:

  /** Default constructor */
  EvtLLSWFF(double _tau_w1, double _tau_wp, double zeta_1);

  /** Default constructor */
  EvtLLSWFF(double _tau_w1, double _tau_wp, double _tau_1, double _tau_2);

  /** Returns vector ffs */
  void getvectorff(EvtId, EvtId, double t, double mass, double* a1f, double* a2f, double* vf, double* a0f);

  /** Returns scalar ffs */
  void getscalarff(EvtId, EvtId, double t, double mass, double* fp, double* f0);

  /** Returns tensor ffs */
  void gettensorff(EvtId, EvtId, double t, double mass, double* hf, double* kf, double* bpf, double* bmf);

  /** Returns baryon ffs */
  void getbaryonff(EvtId, EvtId, double, double, double*, double*, double*, double*);

  /** Returns dirac ffs */
  void getdiracff(EvtId, EvtId, double, double, double*, double*, double*, double*, double*, double*);

  /** Returns rarita ffs */
  void getraritaff(EvtId, EvtId, double, double, double*, double*, double*, double*, double*, double*, double*, double*);


private:

  /** Isgur-Wise function */
  double IsgurWiseFunction(double w);

  /** D0 form factor g+ */
  double gpD0(double w);

  /** D0 form factor g- */
  double gmD0(double w);

  /** D1* form factor gV1 */
  double gV1D1p(double w);

  /** D1* form factor gV2 */
  double gV2D1p(double w);

  /** D1* form factor gV3 */
  double gV3D1p(double w);

  /** D1* form factor gA */
  double gAD1p(double w);

  /** D1 form factor fV1 */
  double fV1D1(double w);

  /** D1 form factor fV2 */
  double fV2D1(double w);

  /** D1 form factor fV3 */
  double fV3D1(double w);

  /** D1 form factor fA */
  double fAD1(double w);

  /** D2 form factor kA1 */
  double kA1D2(double w);

  /** D2 form factor kA2 */
  double kA2D2(double w);

  /** D2 form factor kA3 */
  double kA3D2(double w);

  /** D2 form factor kV */
  double kVD2(double w);

  /** doubles to store normalization of IW function */
  double tau_w1{0};

  /** doubles to slope of IW function */
  double tau_wp{0};

  /** doubles to store b quark mass */
  double mb{0};

  /** doubles to store b quark mass */
  double mc{0};

  /** doubles to store mass differences */
  double L{0};

  /** doubles to store mass differences */
  double Ls{0};

  /** doubles to store mass differences */
  double Lp{0};

  /** doubles to store normalizations of sub-leading IW functions  */
  double zeta_1{0};

  /** doubles to store normalizations of sub-leading IW functions  */
  double tau_1{0};

  /** doubles to store normalizations of sub-leading IW functions  */
  double tau_2{0};

  /** doubles to store chromomagnetic terms for narrow states  */
  double eta_1{0};

  /** doubles to store chromomagnetic terms for narrow states  */
  double eta_2{0};

  /** doubles to store chromomagnetic terms for narrow states  */
  double eta_3{0};

  /** doubles to store chromomagnetic terms for narrow states  */
  double eta_b{0};

  /** doubles to store chromomagnetic terms for broad states  */
  double chi_1{0};

  /** doubles to store chromomagnetic terms for broad states  */
  double chi_2{0};

  /** doubles to store chromomagnetic terms for broad states  */
  double chi_b{0};


};

#endif









