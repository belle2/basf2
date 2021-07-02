/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
                 EvtBSemiTauonicHelicityAmplitudeCalculator* HelicityAmplitudeCalculator) override;

  };

} // Belle 2 Namespace
