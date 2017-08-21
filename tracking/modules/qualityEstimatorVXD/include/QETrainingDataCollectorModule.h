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
#include <tracking/trackFindingVXD/mva/SimpleVariableRecorder.h>
#include <tracking/trackFindingVXD/utilities/VariableSet.h>
#include <tracking/trackFindingVXD/variableExtractors/ClusterInfoExtractor.h>
#include <tracking/trackFindingVXD/variableExtractors/QEResultsExtractor.h>


#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <memory>


namespace Belle2 {
  class QETrainingDataCollectorModule : public Module {

  public:
    /** Constructor of the module. */
    QETrainingDataCollectorModule();

    /** Initializes the Module. */
    virtual void initialize();

    /** Applies the selected quality estimation method for a given set of TCs */
    virtual void event();

    virtual void terminate();


  protected:

    // module parameters

    /** Identifier which estimation method to use. Valid identifiers are:
     * CircleFit
     * TripletFit
     * Random
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

    /** QualityEstimatorMC as target for training */
    std::unique_ptr<QualityEstimatorBase> m_estimatorMC;

    std::string m_TrainingDataOutputName;

    std::unique_ptr<SimpleVariableRecorder> m_recorder;

    MVAVariableSet m_variableSet;

  };
}
