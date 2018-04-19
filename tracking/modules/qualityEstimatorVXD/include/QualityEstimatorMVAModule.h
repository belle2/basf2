/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner, Sebastian Racs                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>
#include <tracking/trackFindingVXD/mva/MVAExpert.h>
#include <tracking/trackFindingVXD/variableExtractors/ClusterInfoExtractor.h>
#include <tracking/trackFindingVXD/variableExtractors/QEResultsExtractor.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <memory>
#include <string>
#include <vector>


namespace Belle2 {

  /** Quality estimation module for SpacePointTrackCandidates using multivariate analysis (MVA).
  * This module calculates a qualityIndicator (QI) for each SpacePointTrackCandidate.
  * This module can support most estimation strategies that implement the interface QualityEstimatorBase
  * and use them together with cluster information to calculate a new QI using MVA.
  *  */
  class QualityEstimatorMVAModule : public Module {

  public:

    /** Constructor of the module. */
    QualityEstimatorMVAModule();

    /** Initializes the Module. */
    void initialize() override;

    /** Launches mvaExpert and sets the magnetic field strength */
    void beginRun() override;

    /** Applies the selected quality estimation method for a given set of TCs */
    void event() override;


  protected:

    // -----module parameters---

    /** Identifier which estimation method to use. Valid identifiers are:
     * tripletFit
     * circleFit
     * helixFit
     */
    std::string m_EstimationMethod;

    /** sets the name of the expected StoreArray containing SpacePointTrackCands */
    std::string m_SpacePointTrackCandsStoreArrayName;

    /** identifier of weightfile in Database or local root/xml file */
    std::string m_WeightFileIdentifier;

    /** whether to use timing information available in the weight file */
    bool m_UseTimingInfo;

    /** how to compile information from clusters ['Average'] */
    std::string m_ClusterInformation;

    // -------------------------

    /** the storeArray for SpacePointTrackCands as member, is faster than recreating link for each event */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** pointer to the selected QualityEstimator */
    std::unique_ptr<QualityEstimatorBase> m_estimator;

    /** pointer to the object to interact with the MVA package */
    std::unique_ptr<MVAExpert> m_mvaExpert;

    /** pointer to object that extracts the results from the estimation mehtod
    * (including QI, chi2, p_t and p_mag) */
    std::unique_ptr<QEResultsExtractor> m_qeResultsExtractor;

    /** pointer to object that extracts info from the clusters of a SPTC */
    std::unique_ptr<ClusterInfoExtractor> m_clusterInfoExtractor;

    /** set of named variables to be used in MVA */
    std::vector<Named<float*>>  m_variableSet;

    /** number of SpacePoints in SPTC as additional info for MVA,
     * type is float to be consistend with m_variableSet (and MVA implementation) */
    float m_nSpacePoints = NAN;

  };
}
