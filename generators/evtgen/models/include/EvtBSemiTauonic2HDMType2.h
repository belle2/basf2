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

#include "EvtGenBase/EvtDecayAmp.hh"

class EvtParticle;

namespace Belle2 {
  class EvtBSemiTauonicAmplitude;
  class EvtBSemiTauonicHelicityAmplitudeCalculator;

  /** The EvtGen model of semi-tauonic B decays including the charged higgs effect of 2HDM Type2 based on [M. Tanaka and R. Watanabe PRD87,034028(2013)].
   *
   * The model implements B->D*taunu and B->Dtaunu decays including the charged higgs effect of 2HDM Type2
   * based on the calculation in [M. Tanaka and R. Watanabe Phys. Rev. D87, 034028 (2013)].
   * The example of the decay.dec description is as follows
   * Decay B0
   * 0.01 D*+ tau- nu_tau  BSTD2HDMTYPE2 rho_A1^2 R_1(1) R_2(1) a_R3 m_b m_c tan(beta)/m_H^+ [m_H^+] (7 or 8 parameters)
   * 0.01 D+ tau- nu_tau   BSTD2HDMTYPE2 rho_1^2  a_S1 m_b m_c tan(beta)/m_H^+ [m_H^+]  (5 or 6 parameters)
   * Enddecay
   * Here rho_1^2, rho_A1^2, R_1(1) and R_2(1) are the HQET form factor parameters estimated by the data of the D(*)lnu decays.
   * m_b and m_c are bottom and quark masses used in the scalar form factors (m_c is the running mass at m_b scale),
   * The CS2 term corresponding to -m_tau*m_c/(m_H^+)^2 is ignored by default unless the parameter m_H^+ is given explicitly.
   * a_R3 and a_S1 are parameters for the theoretical uncertainties of scalar form factors related to O(1/m_q) correction.
   * The authors of [PRD87,034028] use a_R3 and a_S1 = 1 +/- 1 to estimate the uncertainty for it.
   */
  class EvtBSemiTauonic2HDMType2: public  EvtDecayAmp  {

  public:

    /** The default constructor   */
    EvtBSemiTauonic2HDMType2();
    /** The destructor   */
    virtual ~EvtBSemiTauonic2HDMType2();

    /** The function returns the model name
     *@return name of the model "BSTD_2HDMTYPE2"
     */
    std::string getName();

    /** The function makes a copy of an EvtBSemiTauonic2HDMType2 object.
     *@return a pointer to a new uninitialized EvtBSemiTauonic2HDMType2 object
     */
    EvtDecayBase* clone();

    /** The function evaluates the decay amplitude of the parent particle.
     * @p a pointer to the parent particle
     */
    void decay(EvtParticle* p);

    /** The function sets the maximum value of the probability.   */
    void initProbMax();

    /** The function initializes the decay.
     * The function reads paramters from the decay file and initializes the parameters for the decay amplitude calculator.
     */
    void init();

  private:
    /** A pointer to the helicity amplitude calculator.   */
    EvtBSemiTauonicHelicityAmplitudeCalculator* m_CalcHelAmp;

    /** A pointer to the spin-dependent amplitude calculator specific to the spin type of the daughter meson.   */
    EvtBSemiTauonicAmplitude* m_CalcAmp;
  };

} // Belle 2 Namespace
