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

  /** The EvtGen model of semi-tauonic B decays including new physics effects based on [M. Tanaka and R. Watanabe PRD87,034028(2013)].
   *
   * The model implements B->D*taunu and B->Dtaunu decays including new physics effects
   * described in [M. Tanaka and R. Watanabe Phys. Rev. D87, 034028 (2013)].
   * The example of the decay.dec description is as follows
   * Decay B0
   * 0.01 D*+ tau- nu_tau  BSTD rho_A1^2 R_1(1) R_2(1) a_R3 m_b m_c |CV1| Arg(CV1) |CV2| Arg(CV2) |CS1| Arg(CS1) |CS2| Arg(CS2) |CT| Arg(CT) (16 parameters)
   * 0.01 D+ tau- nu_tau   BSTD rho_1^2  a_S1 |CV1| Arg(CV1) m_b m_c |CV2| Arg(CV2) |CS1| Arg(CS1) |CS2| Arg(CS2) |CT| Arg(CT) (14 paramters)
   * Enddecay
   * Here rho_1^2, rho_A1^2, R_1(1) and R_2(1) are the HQET form factor parameters estimated by the data of the D(*)lnu decays,
   * m_b and m_c are bottom and quark masses used in the scalar form factors (m_c is the running mass at m_b scale),
   * and C{V1,V2,S1,S2,T} are the Wilson Coefficients of the new physics contributions for
   * left handed (SM type), right handed, Scalar (S1:S+P, S2:S-P) and Tensor type, respectively.
   * a_R3 and a_S1 are parameters for the theoretical uncertainties of scalar form factors related to O(1/m_q) correction.
   * The authors of [PRD87,034028] use a_R3 and a_S1 = 1 +/- 1 to estimate the uncertainty for it.
   */
  class EvtBSemiTauonic: public  EvtDecayAmp  {

  public:
    /** The default constructor  */
    EvtBSemiTauonic();
    /** The destructor   */
    virtual ~EvtBSemiTauonic();

    /** The function returns the model name
     *@return name of the model "BSTD"
     */
    std::string getName();

    /** The function makes a copy of an EvtBSTD object.
     *@return a pointer to a new uninitialized EvtBSemiTauonic object
     */
    EvtDecayBase* clone();

    /** The function evaluates the decay amplitude of the parent particle.
     * @param a pointer to the parent particle
     */
    void decay(EvtParticle* p);

    /** The function sets the maximum value of the probability.
     */
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
