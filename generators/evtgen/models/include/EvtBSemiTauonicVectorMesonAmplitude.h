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

#include "generators/evtgen/models/EvtBSemiTauonicAmplitude.h"

namespace Belle2 {
  /** The class calculates the spin dependent amplitudes of B->D*taunu decays for the BSemiTauonic model based on [M. Tanaka and R. Watanabe PRD87,034028(2013)].
   */
  class EvtBSemiTauonicVectorMesonAmplitude : public EvtBSemiTauonicAmplitude {

  public:

    /** The function calculates the spin dependent amplitude.
     *
     * @param parent a pointer to the parent particle.
     * @param amp a pointer to fill the calculated spin dependent amplitude.
     * @param HelicityAmplitudeCalculator a pointer to the calculator of the helicity dependent amplitude.
     * The function calculate the spin dependent amplitude of the semi-tauonic decay to a vector meson (D* meson).
     */
    void CalcAmp(EvtParticle* parent, EvtAmp& amp,
                 EvtBSemiTauonicHelicityAmplitudeCalculator* HelicityAmplitudeCalculator);

  };

} // Belle 2 Namespace
