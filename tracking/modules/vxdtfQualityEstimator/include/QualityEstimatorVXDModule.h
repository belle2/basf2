/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <memory>
#include <string>

namespace Belle2 {

  /** Quality estimation module for SpacePointTrackCandidates.
   * This module calculates a qualityIndicator (QI) for each SpacePointTrackCandidate.
   * Following the Strategy pattern this module can support
   * all estimation strategies that implement the interface QualityEstimatorBase.
   *  */
  class QualityEstimatorVXDModule : public Module {

  public:

    /** Constructor of the module. */
    QualityEstimatorVXDModule();

    /** Initializes the Module. */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /** Applies the selected quality estimation method for a given set of TCs */
    void event() override;


  protected:

    // module parameters

    /** Identifier which estimation method to use. Valid identifiers are:
     * mcInfo
     * circleFit
     * tripletFit
     * helixFit
     * random
     */
    std::string m_EstimationMethod;

    /** sets the name of the expected StoreArray containing SpacePointTrackCands */
    std::string m_SpacePointTrackCandsStoreArrayName;

    /** sets the name of the expected StoreArray containing MCRecoTracks. Only required for MCInfo method */
    std::string m_MCRecoTracksStoreArrayName;
    /** Only required for MCInfo method */
    bool m_MCStrictQualityEstimator;

    // member variables

    /** the storeArray for SpacePointTrackCands as member, is faster than recreating link for each event */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** pointer to the selected QualityEstimator */
    std::unique_ptr<QualityEstimatorBase> m_estimator;

  };
}
