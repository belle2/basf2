/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

class EvtParticle;
class EvtAmp;
class EvtSpinDensity;

namespace Belle2 {
  class EvtBSemiTauonicHelicityAmplitudeCalculator;

  /** The base class for the calculation of the spin dependent amplitudes for the BSemiTauonic model based on [M. Tanaka and R. Watanabe PRD87,034028(2013)]. */
  class EvtBSemiTauonicAmplitude {
  public:

    /** base constructor does nothing   */
    EvtBSemiTauonicAmplitude() {};

    /** base destructor does nothing   */
    virtual ~EvtBSemiTauonicAmplitude() {};


    /** The function calculates the spin dependent amplitude.
     *
     * @param parent a pointer to the parent particle.
     * @param amp a pointer to fill the calculated spin dependent amplitude.
     * @param HelicityAmplitudeCalculator a pointer to the calculator of the helicity dependent amplitude.
     * The function calculate the spin dependent amplitude of the decay of parent.
     */
    virtual void CalcAmp(EvtParticle* parent, EvtAmp& amp,
                         EvtBSemiTauonicHelicityAmplitudeCalculator* HelicityAmplitudeCalculator) = 0;

    /** The function calculates the maximum probability.
     *
     *@param parent a ID of the parent meson.
     *@param meson  a ID of the daughter meson.
     *@param lepton a ID of the daughter lepton.
     *@param nudaug a ID of the daughter neutrino.
     *@param HelicityAmplitudeCalculator a pointer to the calculator of the helicity dependent amplitude.
     *@return the maxmum probability multiplied by 1.1.
     * The function scan the q^2 and angle between the daughter meson and lepton and search for the the maximum probability value.
     */
    double CalcMaxProb(EvtId parent, EvtId meson, EvtId lepton,
                       EvtId nudaug,
                       EvtBSemiTauonicHelicityAmplitudeCalculator* HelicityAmplitudeCalculator);

    /** The function calculates the rotation matrix to convert the spin basis to the helicity basis in the boosted frame.
     *
     * @param p a pointer to the particle
     * @param p4boost a 4 vector to specify the rest frame to boost to.
     * The function calculate the rotation matrix from the spin basis defined in the p rest frame
     * to the helicity basis in the rest frame of p4boost.
     */
    EvtSpinDensity RotateToHelicityBasisInBoostedFrame(const EvtParticle* p,
                                                       EvtVector4R p4boost);

  };

} // Belle 2 Namespace
