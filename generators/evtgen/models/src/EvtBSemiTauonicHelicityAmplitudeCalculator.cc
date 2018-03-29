/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Koji Hara                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <fstream>
#include <iomanip>
#include <cmath>
#include <string>
#include "framework/logging/Logger.h"
#include "generators/evtgen/models/EvtBSemiTauonicHelicityAmplitudeCalculator.h"

namespace Belle2 {


  EvtBSemiTauonicHelicityAmplitudeCalculator::EvtBSemiTauonicHelicityAmplitudeCalculator()
  {
    m_mB =   5.279; // value used in PRD87,034028
    m_mD =   1.866; // value used in PRD87,034028
    m_mDst = 2.009; // value used in PRD87,034028

    m_rho12  = 1.186; // HFAG end of year 2011
    m_rhoA12 = 1.207; // HFAG end of year 2011

    m_ffR11 = 1.403; // HFAG end of year 2011
    m_ffR21 = 0.854; // HFAG end of year 2011

    m_aS1 = 1.;
    m_aR3 = 1.;

    m_mBottom = 4.20;  // PRD77,113016
    m_mCharm =  0.901; // PRD77,113016 running mass at m_b scale

    m_CV1 = m_CV2 = m_CS1 = m_CS2 = m_CT = 0;

    B2INFO("EvtBSemiTauonicHelicityAmplitudeCalculator initialized with the default values.");
    B2INFO("rho_1^2 : " << m_rho12);
    B2INFO("rho_A1^22 : " << m_rhoA12);
    B2INFO("R_1(1) : " << m_ffR11);
    B2INFO("R_2(1) : " << m_ffR21);
    B2INFO("a_S1: " << m_aS1);
    B2INFO("a_R3: " << m_aR3);
    B2INFO("bottom quark mass: " << m_mBottom);
    B2INFO("charm quark mass: " << m_mCharm);
    B2INFO("CV1 : " << m_CV1);
    B2INFO("CV2 : " << m_CV2);
    B2INFO("CS1 : " << m_CS1);
    B2INFO("CS2 : " << m_CS2);
    B2INFO("CT : " << m_CT);
    B2INFO("B meson mass: " << m_mB);
    B2INFO("D meson mass: " << m_mD);
    B2INFO("D* meson mass: " << m_mDst);
  }

  EvtBSemiTauonicHelicityAmplitudeCalculator::EvtBSemiTauonicHelicityAmplitudeCalculator(const double rho12, const double rhoA12,
      const double ffR11, const double ffR21, const double aS1, const double aR3,
      const double bottomMass, const double charmMass,
      const EvtComplex& CV1, const EvtComplex& CV2, const EvtComplex& CS1, const EvtComplex& CS2, const EvtComplex& CT,
      const double parentMass, const double DMass, const double DstarMass)
  {
    m_mB = parentMass;
    m_mD = DMass;
    m_mDst = DstarMass;
    m_mBottom = bottomMass;
    m_mCharm = charmMass;

    m_rho12  = rho12;
    m_rhoA12 = rhoA12;

    m_ffR11 =  ffR11;
    m_ffR21 =  ffR21;

    m_aS1 = aS1;
    m_aR3 = aR3;

    m_CV1 = CV1;
    m_CV2 = CV2;
    m_CS1 = CS1;
    m_CS2 = CS2;
    m_CT = CT;

    B2INFO("EvtBSemiTauonicHelicityAmplitudeCalculator initialized.");
    B2INFO("rho_1^2 : " << rho12);
    B2INFO("rho_A1^2 : " << rhoA12);
    B2INFO("R_1(1) : " << ffR11);
    B2INFO("R_2(1) : " << ffR21);
    B2INFO("a_S1: " << m_aS1);
    B2INFO("a_R3: " << m_aR3);
    B2INFO("bottom quark mass: " << m_mBottom);
    B2INFO("charm quark mass: " << m_mCharm);
    B2INFO("CV1 : " << m_CV1);
    B2INFO("CV2 : " << m_CV2);
    B2INFO("CS1 : " << m_CS1);
    B2INFO("CS2 : " << m_CS2);
    B2INFO("CT : " << m_CT);
    B2INFO("Parent meson mass: " << m_mB);
    B2INFO("D meson mass: " << m_mD);
    B2INFO("D* meson mass: " << m_mDst);
  }

// Total amplitude
  EvtComplex EvtBSemiTauonicHelicityAmplitudeCalculator::helAmp(double mtau, int tauhel, int Dhel, double w, double costau) const
  {
    return helAmp(m_CV1, m_CV2, m_CS1, m_CS2, m_CT,
                  mtau, tauhel, Dhel, w, costau);
  }

// Total amplitude
  EvtComplex EvtBSemiTauonicHelicityAmplitudeCalculator::helAmp(const EvtComplex& CV1, const EvtComplex& CV2, const EvtComplex& CS1,
      const EvtComplex& CS2, const EvtComplex& CT,
      double mtau, int tauhel, int Dhel, double w, double costau) const
  {
    // sanity check
    assert(chktauhel(tauhel) && chkDhel(Dhel));

    return //(GF/sqrt(2))*Vcb* // <-- constants which does not affect the distribution omitted
      (1.*helampSM(mtau, tauhel, Dhel, w, costau)
       + CV1 * helampV1(mtau, tauhel, Dhel, w, costau)
       + CV2 * helampV2(mtau, tauhel, Dhel, w, costau)
       + CS1 * helampS1(mtau, tauhel, Dhel, w, costau)
       + CS2 * helampS2(mtau, tauhel, Dhel, w, costau)
       + CT * helampT(mtau, tauhel, Dhel, w, costau));
  }

// Leptonic Amplitudes //

// vector
  double EvtBSemiTauonicHelicityAmplitudeCalculator::Lep(const double mtau, int tauhel,
                                                         int whel,
                                                         double q2, double costau) const
  {
    // sanity check
    assert(chktauhel(tauhel) && chkwhel(whel));

    if (tauhel == -1 && whel == -1)return  sqrt(2.*q2) * v(mtau, q2) * (1. - costau);
    if (tauhel == -1 && whel == 0) return -2.*sqrt(q2) * v(mtau, q2) * sqrt(1 - costau * costau);
    if (tauhel == -1 && whel == +1)return  sqrt(2.*q2) * v(mtau, q2) * (1. + costau);
    if (tauhel == -1 && whel == 2) return  0.;

    if (tauhel == +1 && whel == -1)return -sqrt(2.) * mtau * v(mtau, q2) * sqrt(1. - costau * costau);
    if (tauhel == +1 && whel == 0) return  2.*mtau * v(mtau, q2) * costau;
    if (tauhel == +1 && whel == +1)return  sqrt(2.) * mtau * v(mtau, q2) * sqrt(1 - costau * costau);
    if (tauhel == +1 && whel == 2) return  -2.*mtau * v(mtau, q2);

    // should never reach here ...
    assert(0);
    return 0.;
  }

// Scalar
  double EvtBSemiTauonicHelicityAmplitudeCalculator::Lep(const double mtau, int tauhel,
                                                         double q2, double /*costau*/) const
  {
    // sanity check
    assert(chktauhel(tauhel));

    if (tauhel == -1)return 0.;
    if (tauhel == +1)return -2.*sqrt(q2) * v(mtau, q2);

    // should never reach here ...
    assert(0);
    return 0.;
  }

// Tensor
  double EvtBSemiTauonicHelicityAmplitudeCalculator::Lep(const double mtau, int tauhel,
                                                         int whel1, int whel2,
                                                         double q2, double costau) const
  {
    // sanity check
    assert(chktauhel(tauhel) && chkwhel(whel1) && chkwhel(whel2));

    if (whel1 == whel2)return 0;
    if (whel1 > whel2)return -Lep(mtau, tauhel, whel2, whel1, q2, costau);

    if (tauhel == -1 && whel1 == -1 && whel2 == 0)  return -sqrt(2.) * mtau * v(mtau, q2) * (1. - costau);
    if (tauhel == -1 && whel1 == -1 && whel2 == +1) return 2 * mtau * v(mtau, q2) * sqrt(1. - costau * costau);
    if (tauhel == -1 && whel1 == -1 && whel2 == 2)  return Lep(mtau, -1, -1, 0, q2, costau);
    if (tauhel == -1 && whel1 == 0 && whel2 == +1) return -sqrt(2.) * mtau * v(mtau, q2) * (1. + costau);
    if (tauhel == -1 && whel1 == 0 && whel2 == 2)  return Lep(mtau, -1, -1, +1, q2, costau);
    if (tauhel == -1 && whel1 == +1 && whel2 == 2)  return Lep(mtau, -1, 0, +1, q2, costau);

    if (tauhel == +1 && whel1 == -1 && whel2 == 0)  return sqrt(2.*q2) * v(mtau, q2) * sqrt(1. - costau * costau);
    if (tauhel == +1 && whel1 == -1 && whel2 == +1) return -2.*sqrt(q2) * v(mtau, q2) * costau;
    if (tauhel == +1 && whel1 == -1 && whel2 == 2)  return Lep(mtau, +1, -1, 0, q2, costau);
    if (tauhel == +1 && whel1 == 0 && whel2 == +1) return -Lep(mtau, +1, -1, 0, q2, costau);
    if (tauhel == +1 && whel1 == 0 && whel2 == 2)  return Lep(mtau, +1, -1, +1, q2, costau);
    if (tauhel == +1 && whel1 == +1 && whel2 == 2)  return -Lep(mtau, +1, -1, 0, q2, costau);

    // should never reach here ...
    assert(0);
    return 0.;
  }

// Hadronic Amplitudes //

// V-A
  double EvtBSemiTauonicHelicityAmplitudeCalculator::HadV1(int Dhel, int whel, double w) const
  {
    // sanity check
    assert(chkDhel(Dhel) && chkwhel(whel));

    const double r0 = r(Dhel);
    if (Dhel == 2 && whel == 0) {
      return m_mB * sqrt(r0 * (w * w - 1.) / qh2(2, w)) * ((1 + r0) * hp(w) - (1 - r0) * hm(w));
    }
    if (Dhel == 2 && whel == 2) {
      return m_mB * sqrt(r0 / qh2(2, w)) * ((1 - r0) * (w + 1) * hp(w) - (1 + r0) * (w - 1) * hm(w));
    }
    if (Dhel == +1 && whel == +1) {
      return m_mB * sqrt(r0) * ((w + 1) * hA1(w) - sqrt(w * w - 1) * hV(w));
    }
    if (Dhel == -1 && whel == -1) {
      return m_mB * sqrt(r0) * ((w + 1) * hA1(w) + sqrt(w * w - 1) * hV(w));
    }
    if (Dhel == 0 && whel == 0) {
      return m_mB * sqrt(r0 / qh2(0, w)) * (w + 1) *
             (-(w - r0) * hA1(w) + (w - 1) * (r0 * hA2(w) + hA3(w)));
    }
    if (Dhel == 0 && whel == 2) {
      return m_mB * sqrt(r0 * (w * w - 1) / qh2(0, w)) * (-(w + 1) * hA1(w) + (1 - r0 * w) * hA2(w) + (w - r0) * hA3(w));
    }

    // other cases
    return 0.;

  }

// V+A
  double EvtBSemiTauonicHelicityAmplitudeCalculator::HadV2(int Dhel, int whel, double w) const
  {
    // sanity check
    assert(chkDhel(Dhel) && chkwhel(whel));

    if (Dhel == 2) return HadV1(2, whel, w);
    if (Dhel == +1 && whel == +1)return -HadV1(-1, -1, w);
    if (Dhel == -1 && whel == -1)return -HadV1(+1, +1, w);
    if (Dhel == 0) return -HadV1(0, whel, w);
    return 0;
  }

//S+P
  double EvtBSemiTauonicHelicityAmplitudeCalculator::HadS1(int Dhel, double w) const
  {
    // sanity check
    assert(chkDhel(Dhel));

    if (Dhel == 2) return  m_mB * sqrt(r(2)) * (w + 1) * hS(w);
    if (Dhel == 0) return -m_mB * sqrt(r(0)) * sqrt(w * w - 1) * hP(w);
    return 0.;

  }

// S-P
  double EvtBSemiTauonicHelicityAmplitudeCalculator::HadS2(int Dhel, double w) const
  {
    // sanity check
    assert(chkDhel(Dhel));

    if (Dhel == 2) return HadS1(2, w);
    else return -HadS1(Dhel, w);
  }

// Tensor
  double EvtBSemiTauonicHelicityAmplitudeCalculator::HadT(int Dhel, int whel1, int whel2, double w) const
  {
    // sanity check
    assert(chkDhel(Dhel) && chkwhel(whel1) && chkwhel(whel2));

    if (whel1 == whel2)return 0.;
    if (whel1 > whel2)return -HadT(Dhel, whel2, whel1, w);

    const double r0 = r(Dhel);
    if (Dhel == 2 && whel1 == -1 && whel2 == +1)return m_mB * sqrt(r(2)) * sqrt(w * w - 1) * hT(w);
    if (Dhel == 2 && whel1 == 0 && whel2 == 2) return -HadT(2, -1, +1, w);
    if (Dhel == -1 && whel1 == -1 && whel2 == 0)
      return -m_mB * sqrt(r0 / qh2(-1, w)) * (1 - r0 * (w + sqrt(w * w - 1))) *
             (hT1(w) + hT2(w) + (w - sqrt(w * w - 1)) * (hT1(w) - hT2(w)));
    if (Dhel == -1 && whel1 == -1 && whel2 == 2) return -HadT(-1, -1, 0, w);
    if (Dhel == 0 && whel1 == -1 && whel2 == +1)
      return m_mB * sqrt(r0) * ((w + 1) * hT1(w) + (w - 1) * hT2(w) + 2 * (w * w - 1) * hT3(w));
    if (Dhel == 0 && whel1 == 0 && whel2 == 2) return -HadT(0, -1, +1, w);
    if (Dhel == +1 && whel1 == 0 && whel2 == +1)
      return -m_mB * sqrt(r0 / qh2(+1, w)) * (1 - r0 * (w - sqrt(w * w - 1))) *
             (hT1(w) + hT2(w) + (w + sqrt(w * w - 1)) * (hT1(w) - hT2(w)));

    if (Dhel == +1 && whel1 == +1 && whel2 == 2) return -HadT(+1, 0, +1, w);

    // other cases
    return 0.;
  }

// Helicity Amplitudes
// overall factor GF/sqrt(2) Vcb omitted
// Wilson coefficients cXX ommited

// SM
  double EvtBSemiTauonicHelicityAmplitudeCalculator::helampSM(double mtau, int tauhel, int Dhel, double w, double costau) const
  {
    double amp(0.);
    for (int whel = -1; whel <= 2; whel++) {
      amp += eta(whel) * Lep(mtau, tauhel, whel, q2(Dhel, w), costau)
             * HadV1(Dhel, whel, w);
    }
    return amp;
  }

// V-A
  double EvtBSemiTauonicHelicityAmplitudeCalculator::helampV1(double mtau, int tauhel, int Dhel, double w, double costau) const
  {
    return helampSM(mtau, tauhel, Dhel, w, costau);
  }

// V+A
  double EvtBSemiTauonicHelicityAmplitudeCalculator::helampV2(double mtau, int tauhel, int Dhel, double w, double costau) const
  {
    double amp(0.);
    for (int whel = -1; whel <= 2; whel++) {
      amp += eta(whel) * Lep(mtau, tauhel, whel, q2(Dhel, w), costau)
             * HadV2(Dhel, whel, w);
    }
    return amp;
  }

// S+P
  double EvtBSemiTauonicHelicityAmplitudeCalculator::helampS1(double mtau, int tauhel, int Dhel, double w, double costau) const
  {
    return  -Lep(mtau, tauhel, q2(Dhel, w), costau) * HadS1(Dhel, w);
  }

// S-P
  double EvtBSemiTauonicHelicityAmplitudeCalculator::helampS2(double mtau, int tauhel, int Dhel, double w, double costau) const
  {
    return  -Lep(mtau, tauhel, q2(Dhel, w), costau) * HadS2(Dhel, w);
  }

// Tensor
  double EvtBSemiTauonicHelicityAmplitudeCalculator::helampT(double mtau, int tauhel, int Dhel, double w, double costau) const
  {
    double amp(0.);
    for (int whel1 = -1; whel1 <= 2; whel1++) {
      for (int whel2 = -1; whel2 <= 2; whel2++) {
        amp += -1 * eta(whel1) * eta(whel2) * Lep(mtau, tauhel, whel1, whel2, q2(Dhel, w), costau)
               * HadT(Dhel, whel1, whel2, w);
      }
    }
    return amp;
  }


// HQET form factors

// Vector and axial-vector form factors in terms of CLN form factors
// D vector form factor h_+(w)
  double EvtBSemiTauonicHelicityAmplitudeCalculator::hp(double w) const
  {
    const double r0 = r(2);
    return -((1 + r0) * (1 + r0) * (w - 1) * ffV1(w)
             - (1 - r0) * (1 - r0) * (w + 1) * ffS1(w)) / (2 * qh2(2, w));
  }
// D vector form factor h_-(w)
  double EvtBSemiTauonicHelicityAmplitudeCalculator::hm(double w) const
  {
    return - (1 - r(2) * r(2)) * (w + 1) * (ffV1(w) - ffS1(w)) / (2 * qh2(2, w));
  }

// D* axial vector form factor h_{A1}(w)
  double EvtBSemiTauonicHelicityAmplitudeCalculator::hA1(double w) const
  {
    return ffA1(w);
  }

// D* axial vector form factor h_V(w)
  double EvtBSemiTauonicHelicityAmplitudeCalculator::hV(double w) const
  {
    return ffR1(w) * ffA1(w);
  }

// D* axial vector form factor h_{A2}(w)
  double EvtBSemiTauonicHelicityAmplitudeCalculator::hA2(double w) const
  {
    return (ffR2(w) - ffR3(w)) * ffA1(w) / (2 * r(1));
  }

// D* axial vector form factor h_{A3}(w)
  double EvtBSemiTauonicHelicityAmplitudeCalculator::hA3(double w) const
  {
    return (ffR2(w) + ffR3(w)) * ffA1(w) / 2;
  }

// Scalar and pseudo-scalar form factors in terms of V and A form factors
// D scalar form factor h_S(w)
  double EvtBSemiTauonicHelicityAmplitudeCalculator::hS(double w) const
  {
    // M. Tanaka, private communication, without approximating quark masses to the meson mass
    const double r0 = r(2);
    return m_mB / m_mBottom * ((1 - r0) / (1 - rq()) * hp(w)
                               - (1 + r0) / (1 - rq()) * (w - 1) / (w + 1) * hm(w));

    // when quark masses are approximated to the meson masses
    //return ffS1(w);
  }

// D* pseudo scalar form factor h_P(w)
  double EvtBSemiTauonicHelicityAmplitudeCalculator::hP(double w) const
  {
    // M. Tanaka, private communication, without approximating quark masses to the meson mass
    const double r0 = r(1);
    return m_mB / m_mBottom * ((w + 1) * hA1(w) - (1 - r0 * w) * hA2(w) - (w - r0) * hA3(w)) / (1 + rq());

    // when quark masses are approximated to the meson masses
    //return ( (w+1) * hA1(w) - (1-r0*w) * hA2(w) - (w-r0) * hA3(w) ) / (1+r0); // no Lambda/mQ
  }

// Tensor form factors in terms of V and A form factors
// D tensor form factor h_T(w)
  double EvtBSemiTauonicHelicityAmplitudeCalculator::hT(double w) const
  {
    const double r0 = r(2);
    return hp(w) - (1 + r0) * hm(w) / (1 - r0);
  }

// D* tensor form factor h_{T1}(w)
  double EvtBSemiTauonicHelicityAmplitudeCalculator::hT1(double w) const
  {
    const double r0 = r(1);
    return -((1 + r0) * (1 + r0) * (w - 1) * hV(w)
             - (1 - r0) * (1 - r0) * (w + 1) * hA1(w))
           / (2 * qh2(1, w));
  }

// D* tensor form factor h_{T2}(w)
  double EvtBSemiTauonicHelicityAmplitudeCalculator::hT2(double w) const
  {
    const double r0 = r(1);
    return -(1 - r0 * r0) * (w + 1) * (hV(w) - hA1(w))
           / (2 * qh2(1, w));
  }

// D* tensor form factor h_{T3}(w)
  double EvtBSemiTauonicHelicityAmplitudeCalculator::hT3(double w) const
  {
    const double r0 = r(1);
    return ((1 + r0) * (1 + r0) * hV(w) - 2 * r0 * (w + 1) * hA1(w)
            + qh2(1, w) * (r0 * hA2(w) - hA3(w)))
           / (2 * qh2(1, w) * (1 + r0));
  }

// CLN form factors
  double EvtBSemiTauonicHelicityAmplitudeCalculator::z(double w) const
  {
    return (sqrt(w + 1) - sqrt(2)) / (sqrt(w + 1) + sqrt(2));
  }

  double EvtBSemiTauonicHelicityAmplitudeCalculator::ffV1(double w) const
  {
    return ffV11() * (1 - 8 * m_rho12 * z(w)
                      + (51 * m_rho12 - 10) * z(w) * z(w)
                      - (252 * m_rho12 - 84) * z(w) * z(w) * z(w));
  }

  double EvtBSemiTauonicHelicityAmplitudeCalculator::ffS1(double w) const
  {
    return (1 + aS1() * dS1(w)) * ffV1(w);
  }

  double EvtBSemiTauonicHelicityAmplitudeCalculator::ffA1(double w) const
  {
    return ffA11() * (1 - 8 * m_rhoA12 * z(w)
                      + (53 * m_rhoA12 - 15) * z(w) * z(w)
                      - (231 * m_rhoA12 - 91) * z(w) * z(w) * z(w));
  }

  double EvtBSemiTauonicHelicityAmplitudeCalculator::ffR1(double w) const
  {
    return m_ffR11 - 0.12 * (w - 1) + 0.05 * (w - 1) * (w - 1);
  }

  double EvtBSemiTauonicHelicityAmplitudeCalculator::ffR2(double w) const
  {
    return m_ffR21 + 0.11 * (w - 1) - 0.06 * (w - 1) * (w - 1);
  }

  double EvtBSemiTauonicHelicityAmplitudeCalculator::ffR3(double w) const
  {
    return 1 + aR3() * dR3(w);
  }

  double EvtBSemiTauonicHelicityAmplitudeCalculator::dS1(double w) const
  {
    return -0.019 + 0.041 * (w - 1.) - 0.015 * (w - 1.) * (w - 1.);
  }

  double EvtBSemiTauonicHelicityAmplitudeCalculator::dR3(double w) const
  {
    return 0.22 - 0.052 * (w - 1.) + 0.026 * (w - 1.) * (w - 1.);
  }

/// kinematics
  double EvtBSemiTauonicHelicityAmplitudeCalculator::mD(int Dhel) const
  {
    assert(chkDhel(Dhel));
    double mesonMass(-1.);
    if (Dhel == 2)mesonMass = m_mD;
    else mesonMass = m_mDst;
    assert(mesonMass >= 0.);
    return mesonMass;
  }

// tau velocity
  double EvtBSemiTauonicHelicityAmplitudeCalculator::v(double mtau, double q2) const
  {
    return sqrt(1 - mtau * mtau / q2);
  }

// q^2/mB^2
  double EvtBSemiTauonicHelicityAmplitudeCalculator::qh2(int Dhel, double w) const
  {
    return 1 - 2 * r(Dhel) * w + r(Dhel) * r(Dhel);
  }

// q^2
  double EvtBSemiTauonicHelicityAmplitudeCalculator::q2(int Dhel, double w) const
  {
    return m_mB * m_mB * qh2(Dhel, w);
  }

// sanity checker
  bool EvtBSemiTauonicHelicityAmplitudeCalculator::chkDhel(int Dhel) const
  {
    if (Dhel == -1 || Dhel == 0 || Dhel == 1 || Dhel == 2)return true;
    else return false;
  }

  bool EvtBSemiTauonicHelicityAmplitudeCalculator::chkwhel(int whel) const
  {
    if (whel == -1 || whel == 0 || whel == 1 || whel == 2)return true;
    else return false;
  }

  bool EvtBSemiTauonicHelicityAmplitudeCalculator::chktauhel(int tauhel) const
  {
    if (tauhel == -1 || tauhel == 1)return true;
    else return false;
  }

//bool EvtBSemiTauonicHelicityAmplitudeCalculator::chkcostau(double costau) const
//{
//  if (costau >= -1.0 && costau <= 1.0)return true;
//  else return false;
//}
} // Belle 2 Namespace
