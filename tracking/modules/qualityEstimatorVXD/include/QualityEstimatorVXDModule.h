/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <memory>
#include <string>
#include <vector>


namespace Belle2 {

  /** Quality estimation module for SpacePointTrackCandidates.
   * This module calculates a qualityIndex for each SpacePointTrackCandidate.
   * Following the Strategy pattern this module can support
   * all estimation strategies that implement the interface QualityEstimatorBase.
   *  */
  class QualityEstimatorVXDModule : public Module {

  public:

    /** Constructor of the module. */
    QualityEstimatorVXDModule();

    /** Initializes the Module. */
    virtual void initialize() override;

    virtual void beginRun() override;

    /** Applies the selected quality estimation method for a given set of TCs */
    virtual void event() override;


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
