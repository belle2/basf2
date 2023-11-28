/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <tracking/trackFindingVXD/mva/MVAExpert.h>

#include <framework/core/Module.h>
#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/FlipRecoTrackExtractor.h>
#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/Flip2ndRecoTrackExtractor.h>

#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <vector>
#include <string>
#include <memory>

namespace Belle2 {

  /**
   * This module can be used for applying the MVA filters and get the 2 flipping QIs.
   */
  class FlipQualityModule : public Module {
  public:

    /**
     * Constructor
     */
    FlipQualityModule();

    /**
     * Initialize the module.
     */
    virtual void initialize() override;

    /**
     * Called at the beginning of a new run
     */
    virtual void beginRun() override;

    /**
     * Called for each event.
     */
    virtual void event() override;

  private:

    std::string m_identifier; /**< database identifier or filename of the weightfile */
    int m_flipMVAIndex = 1;  /**< index of the QI (must be 1 or 2)  */
    std::unique_ptr<MVAExpert> m_mvaExpert; /**< Pointer to the current MVA Expert  */

    std::unique_ptr<FlipRecoTrackExtractor> m_recoTrackExtractor; /**< the dataExtractor for the 1st MVA */
    std::unique_ptr<FlipRecoTrackExtractor2nd> m_recoTrackExtractor2nd; /**< the dataExtractor for the 2nd MVA */
    std::vector<Named<float*>>  m_variableSet; /**< the variable set for the MVA */

    std::string m_recoTracksStoreArrayName = "RecoTracks"; /**< the recoTracks StoreArray Name*/
    StoreArray<RecoTrack> m_recoTracks; /**< Store Array of the recoTracks*/
  };

} // Belle2 namespace

