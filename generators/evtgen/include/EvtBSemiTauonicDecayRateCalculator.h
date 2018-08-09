/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Koji Hara                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVTBSEMITAUONICDECAYRATECALCULATOR_H_INCLUDED
#define EVTBSEMITAUONICDECAYRATECALCULATOR_H_INCLUDED


#include <complex>
#include <iostream>

namespace Belle2 {

  class EvtBSemiTauonicHelicityAmplitudeCalculator;

  /** Class to calculate the differential decay rate, R(D), R(D*), polarizations of tau and D* using BSTD model based on [M. Tanaka and R. Watanabe PRD87,034028(2013)]. */
  class EvtBSemiTauonicDecayRateCalculator {

  public:
    /** The default constructor */
    EvtBSemiTauonicDecayRateCalculator(): m_BSTD(NULL) {}

    /** The destructor */
    ~EvtBSemiTauonicDecayRateCalculator() {}

    /** Function calculates the differential decay rate dGamma/dw/dcostau.
     *@param BSTD helicity ampliutude calculator initialized properly by user.
     *@param mtau daughter lepton mass.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     *@param Dhel   helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@param w      velocity transfer variable.
     *@param costau    cosine of the angle between D(*) meson and the lepton in the (l+nu) rest frame.
     *Note that overall factor (GF/sqrt(2) Vcb)^2 is omitted.
     */
    double dGammadwdcostau(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, double mtau, int tauhel, int Dhel, double w,
                           double costau);

    /** Function calculates the differential decay rate dGamma/dw, integrated for costau.
     *@param BSTD helicity ampliutude calculator initialized properly by user.
     *@param mtau daughter lepton mass.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     *@param Dhel   helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@param w      velocity transfer variable.
     *Note that overall factor (GF/sqrt(2) Vcb)^2 is omitted.
     */
    double dGammadw(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, double mtau, int tauhel, int Dhel, double w);

    /** Function calculates the differential decay rate dGamma/dcostau, integrated for w.
     *@param BSTD helicity ampliutude calculator initialized properly by user.
     *@param mtau daughter lepton mass.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     *@param Dhel   helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@param costau    cosine of the angle between D(*) meson and the lepton in the (l+nu) rest frame.
     *Note that overall factor (GF/sqrt(2) Vcb)^2 is omitted.
     */
    double dGammadcostau(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, double mtau, int tauhel, int Dhel, double costau);

    /** Function calculates the helicity dependent decay rate Gamma, integrated for w and costau.
     *@param BSTD helicity ampliutude calculator initialized properly by user.
     *@param mtau daughter lepton mass.
     *@param tauhel helicity of the lepton in the (l+nu) rest frame {+1,-1}.
     *@param Dhel   helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *Note that overall factor (GF/sqrt(2) Vcb)^2 is omitted.
     */
    double Gamma(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, double mtau, int tauhel, int Dhel);

    /** Function calculates the decay rate Gamma for Dtaunu decay, integrated for w and costau and summed for helicities.
     *@param BSTD helicity ampliutude calculator initialized properly by user.
     *@param mtau daughter lepton mass.
     *Note that overall factor (GF/sqrt(2) Vcb)^2 is omitted.
     */
    double GammaD(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, double mtau);

    /** Function calculates the differential decay rate Gamma for D*taunu decay, integrated for w and costau and summed for helicities.
     *@param BSTD helicity ampliutude calculator initialized properly by user.
     *@param mtau daughter lepton mass.
     *Note that overall factor (GF/sqrt(2) Vcb)^2 is omitted.
     */
    double GammaDstar(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, double mtau);

    /** Function calculates the SM decay rate Gamma for Dlnu decay, integrated for w and costau and summed for helicities.
     *@param BSTD helicity ampliutude calculator initialized properly by user.
     *@param mtau daughter lepton mass (default is the electron mass).
     *Note that overall factor (GF/sqrt(2) Vcb)^2 is omitted.
     */
    double GammaSMD(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, const double mlep = 0.0005110);

    /** Function calculates the SM decay rate Gamma for D*lnu decay, integrated for w and costau and summed for helicities.
     *@param BSTD helicity ampliutude calculator initialized properly by user.
     *@param mtau daughter lepton mass (default is the electron mass).
     *Note that overall factor (GF/sqrt(2) Vcb)^2 is omitted.
     */
    double GammaSMDstar(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, const double mlep = 0.0005110);

    /** Function calculates the ratio of Br(B->Dtaunu)/Br(B->Dlnu), R(D).
     *@param BSTD helicity ampliutude calculator initialized properly by user.
     *@param mtau daughter tau lepton mass for B->Dtaunu decay.
     *@param mlep daughter lepton mass for B->Dlnu decay (default is the electron mass).
     *Note that overall factor (GF/sqrt(2) Vcb)^2 is omitted.
     */
    double RGammaD(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, const double mtau, const double mlep = 0.0005110);

    /** Function calculates the ratio of Br(B->Dtaunu)/Br(B->Dlnu), R(D*).
     *@param BSTD helicity ampliutude calculator initialized properly by user.
     *@param mtau daughter tau lepton mass for B->Dtaunu decay.
     *@param mlep daughter lepton mass for B->Dlnu decay (default is the electron mass).
     */
    double RGammaDstar(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, const double mtau, const double mlep = 0.0005110);

    /** Function calculates the polarization of tau, (RH - LH)/(LH + RH), in B->Dtaunu decay.
     *@param BSTD helicity ampliutude calculator initialized properly by user.
     *@param mtau daughter tau lepton mass for B->Dtaunu decay.
     */
    double PtauD(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, const double mtau);

    /** Function calculates the polarization of tau, (RH - LH)/(LH + RH), in B->D*taunu decay.
     *@param BSTD helicity ampliutude calculator initialized properly by user.
     *@param mtau daughter tau lepton mass for B->Dtaunu decay.
     */
    double PtauDstar(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, const double mtau);

    /** Function calculates the polarization of D*, longitudinal/(longitudinal + transverse), in B->D*taunu decay.
     *@param BSTD helicity ampliutude calculator initialized properly by user.
     *@param mtau daughter tau lepton mass for B->Dtaunu decay.
     */
    double PDstar(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, const double mtau);

    /** Phase space factor, which is multiplied to the helicity amplitude to calculate the decay rate.
     *@param BSTD helicity ampliutude calculator initialized properly by user.
     *@param mtau daughter tau lepton mass for B->Dtaunu decay.
     *@param Dhel helicity of the D(*) meson in the rest frame of the parent meson {+1,0,-1} for D* and 2 for D.
     *@param w      velocity transfer variable.
     */
    double pf(const EvtBSemiTauonicHelicityAmplitudeCalculator& BSTD, double mtau, int Dhel, double w);

  private:
    /** Function used internally for numerical integration */
    double EvaluateByW(double* x, double* param);   // x=w

    /** Function used internally for numerical integration */
    double EvaluateByCostau(double* x, double* param); // x=costau

    /** Function used internally for numerical integration */
    double EvaluateBy2D(double* x, double* param);  // x={w,costau}

    /** temporal pointer to the helicity amplitude calculator for EvaluateBy* functions */
    const EvtBSemiTauonicHelicityAmplitudeCalculator* m_BSTD;
  };

} // namespace Belle2

#endif
