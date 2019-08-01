/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <string>

namespace Belle2 {

  /**
   * Takes fitted RecoTracks and creates Particles from them directly,
   * skipping the step of creating Tracks and TrackFitResults.
   *
   * FIXME This is somewhat a workaround in order not to mess with chargedStableSet
   * in Const.h. If there is another use except monopoles, maybe this module can be
   * made more general.
   */

  class RecoTrackParticleLoaderModule : public Module {

  public:
    /// Constructor
    RecoTrackParticleLoaderModule();

    /// Require and register the datastore arrays
    void initialize() override;

    /// Build Particle array
    void event() override;

  private:
    /// Name of the collection holding the input RecoTracks
    std::string m_recoTrackColName = "RecoTracks";

    /// Name of the particleList holding the output Particles
    std::string m_particleListName = "ParticlesFromRecoTracks";

    /// PDG code of the hypothesis of the output Particles
    int m_pdgCode = 211;
  };
}
