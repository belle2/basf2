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
#include <tracking/trackFindingVXD/mva/MVAExpert.h>
#include <tracking/trackFindingVXD/utilities/VariableSet.h>
#include <tracking/trackFindingVXD/variableExtractors/ClusterInfoExtractor.h>
#include <tracking/trackFindingVXD/variableExtractors/QEResultsExtractor.h>


#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <memory>
#include <string>
#include <vector>


namespace Belle2 {

  /** Quality estimator module for SpacePointTrackCandidates */
  class QualityEstimatorMVAModule : public Module {

  public:

    /** Constructor of the module. */
    QualityEstimatorMVAModule();

    /** Initializes the Module. */
    virtual void initialize();

    virtual void beginRun();

    /** Applies the selected quality estimation method for a given set of TCs */
    virtual void event();


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

    std::string m_WeightFileIdentifier;

    // member variables

    /** the storeArray for SpacePointTrackCands as member, is faster than recreating link for each event */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** pointer to the selected QualityEstimator */
    std::unique_ptr<QualityEstimatorBase> m_estimator;

    std::unique_ptr<MVAExpert> m_mvaExpert;

    MVAVariableSet m_variableSet;

  };
}
