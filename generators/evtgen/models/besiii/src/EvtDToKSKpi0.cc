// Model: EvtDToKSKpi0
// This file is an amplitude model for D+ -> K_S0 K+ pi0.
// The model is from the BESIII Collaboration in PRD 104, 012006 (2021). DOI:&nbsp; https://doi.org/10.1103/PhysRevD.104.012006
//
// Permission to include these files in basf2 was generously granted by the BESIII Collaboration.
//
// Please cite the original reference for any public/published results where this model was used.

#include <iostream>
#include <cmath>
#include <string>
#include <EvtGenBase/EvtCPUtil.hh>
#include <EvtGenBase/EvtTensor4C.hh>
#include <EvtGenBase/EvtPatches.hh>
#include <stdlib.h>
#include <EvtGenBase/EvtParticle.hh>
#include <EvtGenBase/EvtGenKine.hh>
#include <EvtGenBase/EvtPDL.hh>
#include <EvtGenBase/EvtVector4R.hh>
#include <EvtGenBase/EvtReport.hh>

#include <generators/evtgen/EvtGenModelRegister.h>
#include <generators/evtgen/models/besiii/EvtDToKSKpi0.h>
#include <string>

using namespace std;

namespace Belle2 {

  /** register the model in EvtGen */
  B2_EVTGEN_REGISTER_MODEL(EvtDToKSKpi0);

  EvtDToKSKpi0::~EvtDToKSKpi0() {}

  std::string EvtDToKSKpi0::getName()
  {
    return "DToKSKpi0";
  }

  EvtDecayBase* EvtDToKSKpi0::clone()
  {
    return new EvtDToKSKpi0;
  }

  void EvtDToKSKpi0::init()
  {
    checkNArg(0);
    checkNDaug(3);
    checkSpinParent(EvtSpinType::SCALAR);

    _nd = 3;
    _mDp = 1.86965;
    c_motherMass = _mDp;
    _mDp2 = _mDp * _mDp;
    _mDp2inv = 1. / _mDp2;
    KsMass = 0.497611;
    KpMass = 0.493677;
    pi0Mass = 0.134977;
    etamass = 0.547862;
    pipMass = 0.13957061;
    c_meson_radius_inter = 1.5;
    c_meson_radius_Dp = 5;
  }
  void EvtDToKSKpi0::initProbMax()
  {
    setProbMax(1343.2);
  }

  void EvtDToKSKpi0::decay(EvtParticle* p)
  {
    p->initializePhaseSpace(getNDaug(), getDaugs());
    for (int i = 0; i < _nd; i++) {
      _p4Lab[i] = p->getDaug(i)->getP4Lab();
      _p4CM[i] = p->getDaug(i)->getP4();
    }
    double prob = AmplitudeSquare();
    setProb(prob);
    return;
  }

  double EvtDToKSKpi0::twoBodyCMmom(double rMassSq, double d1m, double d2m)
  {
    double kin1 = 1 - pow(d1m + d2m, 2) / rMassSq;
    kin1 = kin1 >= 0 ? sqrt(kin1) : 1;
    double kin2 = 1 - pow(d1m - d2m, 2) / rMassSq;
    kin2 = kin2 >= 0 ? sqrt(kin2) : 1;

    double ret = 0.5 * sqrt(rMassSq) * kin1 * kin2;
    return ret;
  }

  double EvtDToKSKpi0::dampingFactorSquare(const double& cmmom, const int& spin, const double& mRadius)
  {
    double square = mRadius * mRadius * cmmom * cmmom;
    double dfsq   = 1 + square;
    double dfsqres = dfsq + 8 + 2 * square + square * square;

    double ret = (spin == 2) ? dfsqres : dfsq;
    return ret;
  }

  double EvtDToKSKpi0::spinFactor(int spin, double motherMass, double daug1Mass, double daug2Mass, double daug3Mass,
                                  double m12, double m13, double m23)
  {
    if (spin == 0) return 1;

    double _mA  = daug1Mass;
    double _mB  = daug2Mass;
    double _mC  = daug3Mass;
    double _mAB = m12;
    double _mAC = m13;
    double _mBC = m23;

    double massFactor = 1.0 / _mAB;
    double sFactor    = -1;
    sFactor *= ((_mBC - _mAC) + (massFactor * (motherMass * motherMass - _mC * _mC) * (_mA * _mA - _mB * _mB)));

    if (spin == 2) {
      sFactor *= sFactor;
      double extraterm = ((_mAB - (2 * motherMass * motherMass) - (2 * _mC * _mC))
                          + massFactor * pow(motherMass * motherMass - _mC * _mC, 2));
      extraterm *= ((_mAB - (2 * _mA * _mA) - (2 * _mB * _mB)) + massFactor * pow(_mA * _mA - _mB * _mB, 2));
      extraterm /= 3;
      sFactor -= extraterm;
    }

    return sFactor;
  }

  EvtComplex EvtDToKSKpi0::RBW(int id, double resmass, double reswidth, int spin)
  {
    double resmass2 = pow(resmass, 2);

    EvtVector4R p1, p2, p3;
    double mass_daug1 = 0, mass_daug2 = 0, mass_daug3 = 0;
    if (id == 1) {
      p1 = _pd[0];
      mass_daug1 = pi0Mass;
      p2 = _pd[1];
      mass_daug2 = KpMass;
      p3 = _pd[2];
      mass_daug3 = KsMass;
    }
    if (id == 2) {
      p1 = _pd[2];
      mass_daug1 = KsMass;
      p2 = _pd[0];
      mass_daug2 = pi0Mass;
      p3 = _pd[1];
      mass_daug3 = KpMass;
    }
    if (id == 3) {
      p1 = _pd[1];
      mass_daug1 = KpMass;
      p2 = _pd[2];
      mass_daug2 = KsMass;
      p3 = _pd[0];
      mass_daug3 = pi0Mass;
    }

    double rMassSq = (p1 + p2).mass2();
    double m12 = (p1 + p2).mass2();
    double m13 = (p1 + p3).mass2();
    double m23 = (p2 + p3).mass2();

    double rMass = sqrt(rMassSq);
    double frFactor = 1;
    double fdFactor = 1;

    double measureDaughterMoms = twoBodyCMmom(rMassSq, mass_daug1, mass_daug2);
    double nominalDaughterMoms = twoBodyCMmom(resmass2, mass_daug1, mass_daug2);

    if (spin != 0) {
      frFactor = dampingFactorSquare(nominalDaughterMoms, spin, c_meson_radius_inter) / dampingFactorSquare(measureDaughterMoms, spin,
                 c_meson_radius_inter);

      double measureDaughterMoms2 = twoBodyCMmom(c_motherMass * c_motherMass, rMass, mass_daug3);
      double nominalDaughterMoms2 = twoBodyCMmom(c_motherMass * c_motherMass, resmass, mass_daug3);
      fdFactor =  dampingFactorSquare(nominalDaughterMoms2, spin, c_meson_radius_Dp) / dampingFactorSquare(measureDaughterMoms2, spin,
                  c_meson_radius_Dp);
    }
    double A = (resmass2 - rMassSq);
    double B = resmass2 * reswidth * pow(measureDaughterMoms / nominalDaughterMoms, 2.0 * spin + 1) * frFactor / sqrt(rMassSq);
    double C = 1.0 / (pow(A, 2) + pow(B, 2));

    EvtComplex ret(A * C, B * C);
    ret *= sqrt(frFactor * fdFactor);
    ret *= spinFactor(spin, c_motherMass, mass_daug1, mass_daug2, mass_daug3, m12, m13, m23);
    return ret;
  }

  EvtComplex EvtDToKSKpi0::Flatte(int id, double resmass, double g1, double rg2og1)
  {

    EvtVector4R p1, p2, p3;
    double mass_daug1 __attribute__((unused)), mass_daug2 __attribute__((unused)), mass_daug3 __attribute__((unused));
    if (id == 1) {
      p1 = _pd[0];
      mass_daug1 = pi0Mass;
      p2 = _pd[1];
      mass_daug2 = KpMass;
      p3 = _pd[2];
      mass_daug3 = KsMass;
    }
    if (id == 2) {
      p1 = _pd[2];
      mass_daug1 = KsMass;
      p2 = _pd[0];
      mass_daug2 = pi0Mass;
      p3 = _pd[1];
      mass_daug3 = KpMass;
    }
    if (id == 3) {
      p1 = _pd[1];
      mass_daug1 = KpMass;
      p2 = _pd[2];
      mass_daug2 = KsMass;
      p3 = _pd[0];
      mass_daug3 = pi0Mass;
    }

    double rMassSq __attribute__((unused)) = (p1 + p2).mass2();
    double m12 = (p1 + p2).mass2();

    double s = m12;

    double rhoetapi = 2 * twoBodyCMmom(s, KsMass, KpMass) / sqrt(s);
    double rhoKKbar = 2 * twoBodyCMmom(s, etamass, pipMass) / sqrt(s);
    double img = rhoetapi * g1 + rhoKKbar * g1 * rg2og1;

    EvtComplex ret = EvtComplex(1, 0) / EvtComplex(resmass * resmass - s, -img);
    return ret;

  }

  EvtComplex EvtDToKSKpi0::LASS(int id, double resmass, double reswidth)
  {
    int spin = 0;
    double resmass2 = pow(resmass, 2);

    EvtVector4R p1, p2, p3;
    double mass_daug1 = 0, mass_daug2 = 0, mass_daug3 = 0;
    if (id == 1) {
      p1 = _pd[0];
      mass_daug1 = pi0Mass;
      p2 = _pd[1];
      mass_daug2 = KpMass;
      p3 = _pd[2];
      mass_daug3 = KsMass;
    }
    if (id == 2) {
      p1 = _pd[2];
      mass_daug1 = KsMass;
      p2 = _pd[0];
      mass_daug2 = pi0Mass;
      p3 = _pd[1];
      mass_daug3 = KpMass;
    }
    if (id == 3) {
      p1 = _pd[1];
      mass_daug1 = KpMass;
      p2 = _pd[2];
      mass_daug2 = KsMass;
      p3 = _pd[0];
      mass_daug3 = pi0Mass;
    }

    double rMassSq = (p1 + p2).mass2();
    double m12 = (p1 + p2).mass2();
    double m13 = (p1 + p3).mass2();
    double m23 = (p2 + p3).mass2();

    double rMass = sqrt(rMassSq);
    double frFactor = 1;
    double fdFactor = 1;

    double measureDaughterMoms = twoBodyCMmom(rMassSq, mass_daug1, mass_daug2);
    double nominalDaughterMoms = twoBodyCMmom(resmass2, mass_daug1, mass_daug2);
    if (spin != 0) {
      frFactor = dampingFactorSquare(nominalDaughterMoms, spin, c_meson_radius_inter) / dampingFactorSquare(measureDaughterMoms, spin,
                 c_meson_radius_inter);
      double measureDaughterMoms2 = twoBodyCMmom(c_motherMass * c_motherMass, rMass, mass_daug3);
      double nominalDaughterMoms2 = twoBodyCMmom(c_motherMass * c_motherMass, resmass, mass_daug3);
      fdFactor =  dampingFactorSquare(nominalDaughterMoms2, spin, c_meson_radius_Dp) / dampingFactorSquare(measureDaughterMoms2, spin,
                  c_meson_radius_Dp);
    }

    double q = measureDaughterMoms;
    double g = reswidth * pow(measureDaughterMoms / nominalDaughterMoms, 2.0 * spin + 1) * frFactor / sqrt(rMassSq);
    g *= resmass;

    const double _a    = 0.113;
    const double _r    = -33.8;
    const double _R    = 1;
    const double _phiR = -109.7 * 3.141592653 / 180.0;
    const double _B    = 0.96;
    const double _phiB = 0.1 * 3.141592653 / 180.0;

    double cot_deltaB  = (1.0 / (_a * q)) + 0.5 * _r * q;
    double qcot_deltaB = (1.0 / _a) + 0.5 * _r * q * q;

    EvtComplex expi2deltaB = EvtComplex(qcot_deltaB, q) / EvtComplex(qcot_deltaB, -q);
    EvtComplex resT = EvtComplex(cos(_phiR + 2 * _phiB), sin(_phiR + 2 * _phiB)) * _R;

    EvtComplex prop = EvtComplex(1, 0) / EvtComplex(resmass2 - rMassSq, -(resmass) * g);
    resT *= prop * (resmass2 * reswidth / nominalDaughterMoms) * expi2deltaB;

    resT += EvtComplex(cos(_phiB), sin(_phiB)) * _B * (cos(_phiB) + cot_deltaB * sin(_phiB)) * sqrt(rMassSq) / EvtComplex(qcot_deltaB,
            -q);

    resT *= sqrt(frFactor * fdFactor);
    resT *= spinFactor(spin, c_motherMass, mass_daug1, mass_daug2, mass_daug3, m12, m13, m23);

    return resT;
  }

  double EvtDToKSKpi0::AmplitudeSquare()
  {
    EvtVector4R dp1 = GetDaugMomLab(0);
    EvtVector4R dp2 = GetDaugMomLab(1);
    EvtVector4R dp3 = GetDaugMomLab(2);
    _pd[0] = dp3;
    _pd[1] = dp2;
    _pd[2] = dp1;

    const double K892pMass = 0.89176;
    const double K892pWidth = 0.0503;

    const double K892zeroMass = 0.89555;
    const double K892zeroWidth = 0.0473;

    const double SwaveKppi0Mass = 1.441;
    const double SwaveKppi0Width = 0.193;

    const double SwaveKspi0Mass = 1.441;
    const double SwaveKspi0Width = 0.193;

    EvtComplex temp(0.0, 0.0);

    EvtComplex amp_K892p(1, 0);
    EvtComplex amp_K892zero(-0.3903972065719, 0.1298035433874);
    EvtComplex amp_SwaveKppi0(-1.543197997647, 1.30109134697);
    EvtComplex amp_SwaveKspi0(-3.123793580183, -0.3449005761848);

    temp += amp_K892p * (RBW(1, K892pMass, K892pWidth, 1));
    temp += amp_K892zero * (RBW(2, K892zeroMass, K892zeroWidth, 1));
    temp += amp_SwaveKppi0 * (LASS(1, SwaveKppi0Mass, SwaveKppi0Width));
    temp += amp_SwaveKspi0 * (LASS(2, SwaveKspi0Mass, SwaveKspi0Width));

    double ret = pow(abs(temp), 2);
    return (ret <= 0) ? 1e-20 : ret;
  }

} // Belle2 namespace
