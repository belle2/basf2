/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <boost/optional.hpp>
#include <string>

namespace Belle2 {

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
    boost::optional<double> m_param_minimalWeight = boost::none;
  };
}

