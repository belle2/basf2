/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Koji Hara                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <assert.h>
#include "EvtGenBase/EvtComplex.hh"

namespace Belle2 {
  /** The class calculates the helicity amplitude of semi-tauonic B decays including new physics effects based on [M. Tanaka and R. Watanabe PRD87,034028(2013)].
   *
   * The class implements helicity amplitude of the B->D*taunu and B->Dtaunu decays including new physics effects
   * described in [M. Tanaka and R. Watanabe Phys. Rev. D87, 034028 (2013)].
   * The five possible new physics contributions are included:
   * V1=left handed vector type (SM like),
   * V2=right handed vector type,
   * S1=scalar type (S+P),
   * S2=scalar type (S-P),
   * T=tensor type.
   */
  class EvtBSemiTauonicHelicityAmplitudeCalculator {

  public:

    /** The default constructor.
     * Initializes with the default parameter values used by the aurhos of PRD87,034028.
     */
    EvtBSemiTauonicHelicityAmplitudeCalculator();

    /** The constructor with HQET form factor parameters, Wilson coefficients of new physics contributions and parent B, daughter D(*) meson masses.
     *@param rho12 HQET form factor parameter rho_1^2 obtained by Dlnu decay data.
     *@param rhoA12 HQET form factor parameter rho_A1^2 obtained by D*lnu decay data.
     *@param ffR11 HQET form factor parameter R_1(1) obtained by D*lnu decay data.
     *@param ffR21 HQET form factor parameter R_2(1) obtained by D*lnu decay data.
     *@param AS1 a parameter to take into account the theoretical error of the scalar form factor for Dtaunu decay.
     *@param AR3 a parameter to take into account the theoretical error of the scalar form factor for D*taunu decay.
     *@param CV1 Wilson coeffcient of the left handed vector type NP contribution.
     *@param CV2 Wilson coeffcient of the right handed vector type NP contribution.
     *@param CS1 Wilson coeffcient of the scalar (S+P) type NP contribution.
     *@param CS2 Wilson coeffcient of the scalar (S-P) type NP contribution.
     *@param CT  Wilson coeffcient of the tensor type NP contribution.
     *@param parentMass mass of the parent (B) meson.
     *@param DMass mass of the scalar type daughter (D) meson.
     *@param DstarMass mass of the vector type daughter (D*) meson.
     *@param bottomMass mass of the bottom quark mass (running mass at the energy of bottom quark mass)
     *@param charmMass mass of the charm quark mass (running mass at the energy of bottom quark mass)
     * The constructor initializes the parameters of the decay.
     * The recommended values of AS1 and AR3 by authors of PRD87,034028 are 1+/-1.
     */
    EvtBSemiTauonicHelicityAmplitudeCalculator(const double rho12, const double rhoA12, const double ffR11, const double ffR21,
                                               const double AS1, const double AR3,
                                               const double bottomMass, const double charmMass,
                                               const EvtComplex& CV1, const EvtComplex& CV2, const EvtComplex& CS1, const EvtComplex& CS2, const EvtComplex& CT,
                                               const double parentMass, const double DMass, const double DstarMass);

    /** The destructor.  */
    virtual ~EvtBSemiTauonicHelicityAmplitudeCalculator() {};

    /** The function calculates the helicity amplitude.
     *@param mtau  daughter lepton mass.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     *@param Dhel   helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@param w      velocity transfer variable.
     *@param costau    cosine of the angle between D(*) meson and the lepton in the (l+nu) rest frame.
     * The overall factor GF/sqrt(2) Vcb omitted because it does not change the distribution.
    */
    EvtComplex helAmp(double mtau, int tauhel, int Dhel, double w, double costau) const;

  public:

    /** The function calculates helicity amplitudes with given Wilson coefficients.
     *@param CV1 Wilson coeffcient of the left handed vector type NP contribution.
     *@param CV2 Wilson coeffcient of the right handed vector type NP contribution.
     *@param CS1 Wilson coeffcient of the scalar (S+P) type NP contribution.
     *@param CS2 Wilson coeffcient of the scalar (S-P) type NP contribution.
     *@param CT  Wilson coeffcient of the tensor type NP contribution.
     *@param mtau  daughter lepton mass.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     *@param Dhel   helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@param w      velocity transfer variable.
     *@param costau    cosine of the angle between D(*) meson and the lepton in the (l+nu) rest frame.
     * The overall factor GF/sqrt(2) Vcb omitted because it does not change the distribution.
    */
    EvtComplex helAmp(const EvtComplex& CV1, const EvtComplex& CV2, const EvtComplex& CS1, const EvtComplex& CS2, const EvtComplex& CT,
                      double mtau, int tauhel, int Dhel, double w, double costau) const;

    /** The function to calculate the Leptonic Amplitudes for B->D*taunu decay of the vector type contribution.
     *@param mtau   daughter lepton mass.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     *@param whel   helicity of the virtual vector boson {+1,0,1,2}.
     *@param q2   q^2 of the decay (square of l+nu invariant mass).
     *@param costau    cosine of the angle between D(*) meson and the lepton in the (l+nu) rest frame.
     *@return calculated amplitude value.
     */
    double Lep(const double mtau, int tauhel, int whel, double q2, double costau) const; // Vector

    /** The function to calculate the Leptonic Amplitudes for B->Dtaunu decay of the scalar type contribution
     *@param mtau   daughter lepton mass.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     *@param q2   q^2 of the decay (square of l+nu invariant mass).
     *@param costau    cosine of the angle between D(*) meson and the lepton in the (l+nu) rest frame.
     *@return calculated amplitude value.
     */
    double Lep(const double mtau, int tauhel, double q2, double costau) const; // Scalar

    /** The function to calculate the Leptonic Amplitudes for B->D*taunu decay of the tensor type contribution.
     *@param mtau   daughter lepton mass.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     *@param whel1   helicity of the one virtual vector boson {+1,0,1,2}.
     *@param whel2   helicity of the another virtual vector boson {+1,0,1,2}.
     *@param q2   q^2 of the decay (square of l+nu invariant mass).
     *@param costau    cosine of the angle between D(*) meson and the lepton in the (l+nu) rest frame.
     *@return calculated amplitude value.
     * The tensor contribution is described by two vector contributions.
     */
    double Lep(const double mtau, int tauhel, int whel1, int whel2, double q2, double costau) const; // Tensor

    /** The function to calculate the Hadronic Amplitudes of left handed (V-A) type contribution.
     *@param Dhel helicity of the daughter D(*) meson {+1,0,1} for D* and 2 for D.
     *@param whel helicity of the virtual vector boson {+1,0,1,2}.
     *@return calculated amplitude value.
     */
    double HadV1(int Dhel, int whel, double w) const; // V-A

    /** The function to calculate the Hadronic Amplitudes of right handed (V+A) type contribution.
     *@param Dhel helicity of the daughter D(*) meson {+1,0,1} for D* and 2 for D.
     *@param whel helicity of the virtual vector boson {+1,0,1,2}.
     *@param w   velocity transfer variable.
     *@return calculated amplitude value.
     */
    double HadV2(int Dhel, int whel, double w) const; // V+A

    /** The function to calculate the Hadronic Amplitudes of scalar (S+P) type contribution.
     *@param Dhel helicity of the daughter D(*) meson {+1,0,1} for D* and 2 for D.
     *@param whel helicity of the virtual vector boson {+1,0,1,2}.
     *@param w   velocity transfer variable.
     *@return calculated amplitude value.
     */
    double HadS1(int Dhel, double w) const; // S+P

    /** The function to calculate the Hadronic Amplitudes of scalar (S-P) type contribution.
     *@param Dhel helicity of the daughter D(*) meson {+1,0,1} for D* and 2 for D.
     *@param whel helicity of the virtual vector boson {+1,0,1,2}.
     *@param w   velocity transfer variable.
     *@return calculated amplitude value.
     */
    double HadS2(int Dhel, double w) const; // S-P

    /** The function to calculate the Hadronic Amplitudes of tensor type contribution.
     *@param Dhel helicity of the daughter D(*) meson {+1,0,1} for D* and 2 for D.
     *@param whel1   helicity of the one virtual vector boson {+1,0,1,2}.
     *@param whel2   helicity of the another virtual vector boson {+1,0,1,2}.
     *@param w   velocity transfer variable.
     *@return calculated amplitude value.
     * The tensor contribution is described by two vector contributions.
     */
    double HadT(int Dhel, int whel1, int whel2, double w) const; // Tensor

    /** Helicity Amplitudes of SM (left handed) contribution.
     *@param mtau   daughter lepton mass.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     *@param Dhel   helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@param w      velocity transfer variable.
     *@param costau    cosine of the angle between D(*) meson and the lepton in the (l+nu) rest frame.
     * Overall factor GF/sqrt(2) Vcb omitted.
     * Wilson coefficients CXX ommited.
     */
    double helampSM(double mtau, int tauhel, int Dhel, double w, double costau) const; // SM

    /** Helicity Amplitudes of left handed (V-A) contribution.
     *@param mtau   daughter lepton mass.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     *@param Dhel   helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@param w      velocity transfer variable.
     *@param costau    cosine of the angle between D(*) meson and the lepton in the (l+nu) rest frame.
     *@return calculated amplitude value.
     * Overall factor GF/sqrt(2) Vcb omitted.
     * Wilson coefficients CXX ommited.
     */
    double helampV1(double mtau, int tauhel, int Dhel, double w, double costau) const; // V-A

    /** Helicity Amplitudes of right handed (V+A) contribution.
     *@param mtau   daughter lepton mass.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     *@param Dhel   helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@param w      velocity transfer variable.
     *@param costau    cosine of the angle between D(*) meson and the lepton in the (l+nu) rest frame.
     *@return calculated amplitude value.
     * Overall factor GF/sqrt(2) Vcb omitted.
     * Wilson coefficients CXX ommited.
     */
    double helampV2(double mtau, int tauhel, int Dhel, double w, double costau) const; // V+A

    /** Helicity Amplitudes of scalar (S+P) type contribution.
     *@param mtau   daughter lepton mass.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     *@param Dhel   helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@param w      velocity transfer variable.
     *@param costau    cosine of the angle between D(*) meson and the lepton in the (l+nu) rest frame.
     *@return calculated amplitude value.
     * Overall factor GF/sqrt(2) Vcb omitted.
     * Wilson coefficients CXX ommited.
     */
    double helampS1(double mtau, int tauhel, int Dhel, double w, double costau) const; // S+P

    /** Helicity Amplitudes of scalar (S-P) type contribution.
     *@param mtau   daughter lepton mass.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     *@param Dhel   helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@param w      velocity transfer variable.
     *@param costau    cosine of the angle between D(*) meson and the lepton in the (l+nu) rest frame.
     *@return calculated amplitude value.
     * Overall factor GF/sqrt(2) Vcb omitted.
     * Wilson coefficients CXX ommited.
     */
    double helampS2(double mtau, int tauhel, int Dhel, double w, double costau) const; // S-P

    /** Helicity Amplitudes of tensor type contribution.
     *@param mtau   daughter lepton mass.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     *@param Dhel   helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@param w      velocity transfer variable.
     *@param costau    cosine of the angle between D(*) meson and the lepton in the (l+nu) rest frame.
     *@return calculated amplitude value.
     * Overall factor GF/sqrt(2) Vcb omitted.
     * Wilson coefficients CXX ommited.
     */
    double helampT(double mtau, int tauhel, int Dhel, double w, double costau) const;  // Tensor

    /** The metric factor.
     *@param whel helicity of the virtual vector boson {+1,0,1,2}.
     *@return metric factor for the helicity state
     */
    double eta(int whel) const {return (whel == 2) ? -1 : 1;}


    ///// HQET form factors /////

    // Vector and axial-vector form factors in terms of CLN form factors

    /** HQET D vector form factor h_+(w).   */
    double hp(double w) const;

    /** HQET D vector form factor h_-(w).   */
    double hm(double w) const;

    /** HQET D* axial vector form factor h_{A1}(w).   */
    double hA1(double w) const;

    /** HQET D* axial vector form factor h_V(w).   */
    double hV(double w) const;

    /** HQET D* axial vector form factor h_{A2}(w).   */
    double hA2(double w) const;

    /** HQET D* axial vector form factor h_{A3}(w).   */
    double hA3(double w) const;

    // Scalar form factors in terms of V and A form factors

    /** D scalar form factor h_S(w) in terms of vector form factors.   */
    double hS(double w) const;

    /** D* pseudo scalar form factor h_P(w) in terms of axial vector form factors.   */
    double hP(double w) const;

    // Tensor form factors in terms of V and A form factors

    /** D tensor form factor h_T(w) in terms of vector form factors.   */
    double hT(double w) const;

    /** D* tensor form factor h_{T1}(w) in terms of axial vector form factors.   */
    double hT1(double w) const;

    /** D* tensor form factor h_{T2}(w).   */
    double hT2(double w) const;

    /** D* tensor form factor h_{T3}(w).   */
    double hT3(double w) const;

    /** CLN form factor z.
     *@param w      velocity transfer variable.
     *@return calculated form factor value.
     */
    double z(double w) const;

    /** CLN form factor V1.
     *@param w      velocity transfer variable.
     *@return calculated form factor value.
     */
    double ffV1(double w) const;

    /** CLN form factor S1.
     *@param w      velocity transfer variable.
     *@return calculated form factor value.
     */
    double ffS1(double w) const;

    /** CLN form factor A1.
     *@param w      velocity transfer variable.
     *@return calculated form factor value.
     */
    double ffA1(double w) const;

    /** CLN form factor R1.
     *@param w      velocity transfer variable.
     *@return calculated form factor value.
     */
    double ffR1(double w) const;

    /** CLN form factor R2.
     *@param w      velocity transfer variable.
     *@return calculated form factor value.
     */
    double ffR2(double w) const;

    /** CLN form factor R3.
     *@param w      velocity transfer variable.
     *@return calculated form factor value.
     */
    double ffR3(double w) const;

    /**  Form factor normalization factor for B->Dlnu.
     *@return the normalization factor (=1).
     * Fixed to 1 because it cancels in R(D(*)) and is not relevant to the decay distribution.
     */
    double ffV11() const {return 1.;} // cancels in R(D)

    /**  Form factor normalization factor for B->D*lnu.
     *@return the normalization factor (=1).
     * Fixed to 1 because it cancels in R(D(*)) and is not relevant to the decay distribution.
     */
    double ffA11() const {return 1.;} // cancels in R(Ds)

    /** HQET correction factor for the scalar form factor for B->Dtaunu.
     *@param w      velocity transfer variable.
     *@return calculated factor.
     */
    double dS1(double w) const;

    /** HQET correction factor for the scalar form factor for B->D*taunu.
     *@param w      velocity transfer variable.
     *@return calculated factor.
     */
    double dR3(double w) const;

    /** HQET correction factor for the uncertainty of 1/m_Q correction.
     *@return value of a_S1 set in the constructor or by setAS1().
     */
    double aS1() const {return getAS1();} // {return 1.;}

    /** HQET correction factor for the uncertainty of 1/m_Q correction.
     *@return value of a_R3 set in the constructor or by setAR3().
     */
    double aR3() const {return getAR3();} // {return 1.;}

    /** Daughter D(*) meson mass.
     *@param Dhel helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@return daughter D(*) meson mass corresponding to the Dhel.
     */
    double mD(int Dhel) const;

    /** Ratio of the daughter meson mass to the parent meson.
     *@param Dhel helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@return daughter D(*) meson mass divided by the parent meson mass.
     */
    double r(int Dhel) const {return mD(Dhel) / m_mB;} // meson mass ratio


    /** Ratio of the charm quark mass to the charm quark mass
     */
    double rq() const {return m_mCharm / m_mBottom;} // quark mass ratio

    /** Function to calculate the tau velocity.
     *@param mtau   daughter lepton mass.
     *@param q2   q^2 of the decay (square of l+nu invariant mass).
     *@return calculated tau velocity.
     */
    double v(double mtau, double q2) const;

    /** Function to calculate the q^2 of the decay (square of l+nu invariant mass).
     *@param Dhel helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@param w    velocity transfer variable.
     *@return calculated q^2.
     */
    double q2(int Dhel, double w) const;

    /** Function to calculate the q^2 divided by the square of parent mass (m_B^2).
     *@param Dhel helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@param w    velocity transfer variable.
     *@return calculated q^2/m_B^2.
     */
    double qh2(int Dhel, double w) const;

    // range of q2 and w

    /** Minimum value of the q^2.
     *@param mtau daughter lepton mass.
     *@return calcualted q^2 minimum.
     */
    double q2min(double mtau) const {return mtau * mtau;}

    /** Maximum value of the q^2.
     *@param mtau daughter lepton mass.
     *@return calcualted q^2 maximum.
     */
    double q2max(int Dhel) const {return (m_mB - mD(Dhel)) * (m_mB - mD(Dhel));}

    /** Calculate the velocity transfer variable w.
     *@param Dhel helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@param q2   q^2 of the decay (square of l+nu invariant mass).
     *@return calcualted w.
     */
    double wfunc(int Dhel, double q2) const {return (1. + r(Dhel) * r(Dhel) - q2 / m_mB / m_mB) / 2. / r(Dhel);}

    /** Minimum value of the velocity transfer variable w.
     *@return the minimum of the w (= 1).
     */
    double wmin() const {return 1.;};

    /** Maximum value of the velocity transfer variable w.
     *@param mtau daughter lepton mass.
     *@param Dhel helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@return the calculated maximum of the w.
     */
    double wmax(double mtau, int Dhel) const {return wfunc(Dhel, q2min(mtau));}

    /** parameter accessor */

    /** Returns form factor parameter rho_1^2.   */
    double getRho12() const {return m_rho12;}

    /** Returns form factor parameter rho_A1^2.   */
    double getRhoA12() const {return m_rhoA12;}

    /** Returns form factor parameter R_1(1).   */
    double getR11() const {return m_ffR11;}

    /** Returns form factor parameter R_2(1).   */
    double getR21() const {return m_ffR21;}

    /** Returns form factor 1/m_Q correction factor a_S1.   */
    double getAS1() const {return m_aS1;}

    /** Returns form factor 1/m_Q correction factor a_R3.   */
    double getAR3() const {return m_aR3;}

    /** Returns the parent (B) meson mass.   */
    double getMB() const {return m_mB;}

    /** Returns the daughter scalar (D) meson mass.   */
    double getMD() const {return m_mD;}

    /** Returns the daughter vector (D*) meson mass.   */
    double getMDst() const {return m_mDst;}

    /** Returns the bottom quark mass */
    double getMBottom() const {return m_mBottom;}

    /** Returns the charm quark mass */
    double getMCharm() const {return m_mCharm;}

    /** Returns the Wilson coeffcient CV1.   */
    EvtComplex getCV1() const {return m_CV1;}

    /** Returns the Wilson coeffcient CV2.   */
    EvtComplex getCV2() const {return m_CV2;}

    /** Returns the Wilson coeffcient CS1.   */
    EvtComplex getCS1() const {return m_CS1;}

    /** Returns the Wilson coeffcient CS2.   */
    EvtComplex getCS2() const {return m_CS2;}

    /** Returns the Wilson coeffcient CT.   */
    EvtComplex getCT() const {return m_CT;}

    /** Sets the form factor parameter rho_1^2.   */
    void setRho12(double v) {m_rho12 = v;}

    /** Sets the form factor parameter rho_A1^2.   */
    void setRhoA12(double v) {m_rhoA12 = v;}

    /** Sets the form factor parameter R_1(1).   */
    void setR11(double v) {m_ffR11 = v;}

    /** Sets the form factor parameter R_2(1).   */
    void setR21(double v) {m_ffR21 = v;}

    /** Sets the form factor 1/m_Q correction parameter a_S1.   */
    void setAS1(double v) {m_aS1 = v;}

    /** Sets the form factor 1/m_Q correction parameter a_R3.   */
    void setAR3(double v) {m_aR3 = v;}

    /** Sets the parent (B) meson mass.   */
    void setMB(double m)  {m_mB = m;}

    /** Sets the daughter scalar (D) meson mass.   */
    void setMD(double m)  {m_mD = m;}

    /** Sets the daughter vector (D) meson mass.   */
    void setMDst(double m) {m_mDst = m;}

    /** Returns the bottom quark mass */
    void setMBottom(double m) {m_mBottom = m;}

    /** Returns the charm quark mass */
    void setMCharm(double m) {m_mCharm = m;}

    /** Sets the Wilson coeffcient CV1.   */
    void setCV1(EvtComplex v) {m_CV1 = v;}

    /** Sets the Wilson coeffcient CV2.   */
    void setCV2(EvtComplex v) {m_CV2 = v;}

    /** Sets the Wilson coeffcient CS1.   */
    void setCS1(EvtComplex v) {m_CS1 = v;}

    /** Sets the Wilson coeffcient CS2.   */
    void setCS2(EvtComplex v) {m_CS2 = v;}

    /** Sets the Wilson coeffcient CT.   */
    void setCT(EvtComplex v) {m_CT = v;}

  private:
    // Parameters

    // physics constant <-- not affect distributions
    //double hbar;
    //double GF;
    //double Vcb;

    /** Form factor slope parameters rho_1^2.   */
    double m_rho12;

    /** Form factor slope parameters rho_A1^2.   */
    double m_rhoA12;

    /** Form factor parameter R_1(1).   */
    double m_ffR11;

    /** Form factor parameter R_2(1).   */
    double m_ffR21;

    /** 1/mQ correcion factor a_S1.   */
    double m_aS1;

    /** 1/mQ correcion factor a_R3.   */
    double m_aR3;

    /** parent (B) meson mass.   */
    double m_mB;

    /** daughter scalar (D) meson mass.   */
    double m_mD;

    /** daughter vector (D*) meson mass.   */
    double m_mDst;

    /** b quark mass (running mass at m_b scale), used for scalar form factor term */
    double m_mBottom;

    /** c quark mass (running mass at m_b scale), used for scalar form factor term ) */
    double m_mCharm;

    /** Wilson coefficient CV1.   */
    EvtComplex m_CV1;

    /** Wilson coefficient CV2.   */
    EvtComplex m_CV2;

    /** Wilson coefficient CS1.   */
    EvtComplex m_CS1;

    /** Wilson coefficient CS2.   */
    EvtComplex m_CS2;

    /** Wilson coefficient CT.   */
    EvtComplex m_CT;

    /** sanity checkers */

    /** Function to check if Dhel is in the valid range.
     *@param Dhel   helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     */
    bool chkDhel(int Dhel) const;

    /** Function to check if whel is in the valid range.
     *@param whel   helicity of the virtual vector boson {+1,0,1,2}.
     */
    bool chkwhel(int whel) const;

    /** Function to check if tauhel is in the valid range.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     */
    bool chktauhel(int tauhel) const;

    /** Function to check if costau is in the valid range.
     *@param costau    cosine of the angle between D(*) meson and the lepton in the (l+nu) rest frame.
     */
    //bool chkcostau(double costau) const;

  };

} // Belle 2 Namespace
