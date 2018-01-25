/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {
  class RecoTrack;

  /// A module to copy only the fitted reco tracks to the output store array.
  class FittedTracksStorerModule : public Module {

  public:
    /// Constructor.
    FittedTracksStorerModule();

    /// Initialize the store arrays.
    void initialize() override;

    /// Do the copying.
    void event() override;

  private:
    /** StoreArray name of the input reco tracks. */
    std::string m_param_inputRecoTracksStoreArrayName = "PrefitRecoTracks";
    /** StoreArray name of the output reco tracks. */
    std::string m_param_outputRecoTracksStoreArrayName = "RecoTracks";
    /// Minimal weight for copying the hits.
    double m_param_minimalWeight = NAN;
  };
}

