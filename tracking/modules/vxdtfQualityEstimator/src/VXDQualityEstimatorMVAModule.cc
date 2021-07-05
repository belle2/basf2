/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/vxdtfQualityEstimator/VXDQualityEstimatorMVAModule.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;


REG_MODULE(VXDQualityEstimatorMVA)

VXDQualityEstimatorMVAModule::VXDQualityEstimatorMVAModule() : Module()
{
  //Set module properties
  setDescription("The quality estimator module for SpacePointTrackCandidates.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("EstimationMethod",
           m_EstimationMethod,
           "Identifier which estimation method to use. Valid identifiers are: [tripletFit, circleFit, helixFit]",
           m_EstimationMethod);

  addParam("SpacePointTrackCandsStoreArrayName",
           m_SpacePointTrackCandsStoreArrayName,
           "Name of StoreArray containing the SpacePointTrackCandidates to be estimated.",
           m_SpacePointTrackCandsStoreArrayName);


  addParam("WeightFileIdentifier",
           m_weightFileIdentifier,
           "Identifier of weightfile in Database or local root/xml file.",
           m_weightFileIdentifier);

  addParam("UseTimingInfo",
           m_UseTimingInfo,
           "Whether to use timing information available in the weight file",
           m_UseTimingInfo);

  addParam("ClusterInformation",
           m_ClusterInformation,
           "How to compile information from clusters ['Average']",
           m_ClusterInformation);
}

void VXDQualityEstimatorMVAModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_SpacePointTrackCandsStoreArrayName);

  m_qeResultsExtractor = std::make_unique<QEResultsExtractor>(m_EstimationMethod, m_variableSet);

  m_variableSet.emplace_back("NSpacePoints", &m_nSpacePoints);

  if (m_ClusterInformation == "Average") {
    m_clusterInfoExtractor = std::make_unique<ClusterInfoExtractor>(m_variableSet, m_UseTimingInfo);
  }

  m_mvaExpert = std::make_unique<MVAExpert>(m_weightFileIdentifier, m_variableSet);
  m_mvaExpert->initialize();

  // create pointer to chosen estimator
  if (m_EstimationMethod == "tripletFit") {
    m_estimator = std::make_unique<QualityEstimatorTripletFit>();
  } else if (m_EstimationMethod == "circleFit") {
    m_estimator = std::make_unique<QualityEstimatorCircleFit>();
  } else if (m_EstimationMethod == "helixFit") {
    m_estimator = std::make_unique<QualityEstimatorRiemannHelixFit>();
  }
  B2ASSERT("QualityEstimator could not be initialized with method: " << m_EstimationMethod, m_estimator);
}

void VXDQualityEstimatorMVAModule::beginRun()
{
  m_mvaExpert->beginRun();
  // BField is required by all QualityEstimators
  const double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_estimator->setMagneticFieldStrength(bFieldZ);
}

void VXDQualityEstimatorMVAModule::event()
{
  // assign a QI computed using the selected QualityEstimator for each given SpacePointTrackCand
  for (SpacePointTrackCand& spacePointTrackCand : m_spacePointTrackCands) {
    if (not spacePointTrackCand.hasRefereeStatus(SpacePointTrackCand::c_isActive)) {
      continue;
    }

    std::vector<SpacePoint const*> const sortedHits = spacePointTrackCand.getSortedHits();

    if (m_ClusterInformation == "Average") {
      m_clusterInfoExtractor->extractVariables(sortedHits);
    }

    m_nSpacePoints = sortedHits.size();

    m_qeResultsExtractor->extractVariables(m_estimator->estimateQualityAndProperties(sortedHits));

    const float qi = m_mvaExpert->predict();
    spacePointTrackCand.setQualityIndicator(qi);
  }
}
