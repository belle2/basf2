/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/qualityEstimatorVXD/QETrainingDataCollectorModule.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRandom.h>
#include <tracking/trackFindingVXD/utilities/UniquePointerHelper.h>
#include <framework/logging/Logger.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <math.h>
#include <numeric>


using namespace Belle2;


REG_MODULE(QETrainingDataCollector)

QETrainingDataCollectorModule::QETrainingDataCollectorModule() : Module()
{
  //Set module properties
  setDescription("Module to collect training data for a specified qualityEstimator and store it in a root file.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  addParam("EstimationMethod", m_EstimationMethod,
           "Identifier which estimation method to use. Valid identifiers are: [circleFit, tripletFit, helixFit]", std::string(""));

  addParam("SpacePointTrackCandsStoreArrayName", m_SpacePointTrackCandsStoreArrayName,
           "Name of StoreArray containing the SpacePointTrackCandidates to be estimated.", std::string(""));

  addParam("MCRecoTracksStoreArrayName", m_MCRecoTracksStoreArrayName,
           "Name of StoreArray containing MCRecoTracks. Only required for MCInfo method", std::string("MCRecoTracks"));

  addParam("MCStrictQualityEstimator", m_MCStrictQualityEstimator,
           "Only required for MCInfo method. If false combining several MCTracks is allowed.", bool(true));

  addParam("TrainingDataOutputName", m_TrainingDataOutputName, "Name of the output rootfile.", std::string("QETrainingOutput.root"));
}

void QETrainingDataCollectorModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_SpacePointTrackCandsStoreArrayName);

  // prepare recorder
  m_variableSet.setVariable("NHits", -1);
  m_variableSet.setVariable("truth", -1);
  m_variableSet.setVariables(m_EstimationMethod, QualityEstimationResults());


  m_recorder = make_unique<SimpleVariableRecorder>(m_variableSet, m_TrainingDataOutputName, "tree");
}

void QETrainingDataCollectorModule::event()
{
  // BField is required by all QualityEstimators
  double bFieldZ = BFieldMap::Instance().getBField(TVector3(0, 0, 0)).Z();
  m_estimatorMC = make_unique<QualityEstimatorMC>(m_MCRecoTracksStoreArrayName, m_MCStrictQualityEstimator);

  m_estimatorMC->setMagneticFieldStrength(bFieldZ);

  // create pointer to chosen estimator
  if (m_EstimationMethod == "tripletFit") {
    m_estimator = make_unique<QualityEstimatorTripletFit>();
  } else if (m_EstimationMethod == "circleFit") {
    m_estimator = make_unique<QualityEstimatorCircleFit>();
  } else if (m_EstimationMethod == "helixFit") {
    m_estimator = make_unique<QualityEstimatorRiemannHelixFit>();
  }
  B2ASSERT("Not all QualityEstimators could be initialized!", m_estimator);

  m_estimator->setMagneticFieldStrength(bFieldZ);

  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {

    std::vector<SpacePoint const*> const sortedHits = aTC.getSortedHits();
    m_variableSet.setVariable("NHits", sortedHits.size());

    float qi = m_estimatorMC->estimateQuality(sortedHits);
    qi = qi > 0 ? 1 : 0;
    m_variableSet.setVariable("truth", qi);

    QualityEstimationResults results = m_estimator->estimateQualityAndProperties(sortedHits);
    m_variableSet.setVariables(m_EstimationMethod, results);

    // record variables
    m_recorder->record();
  }
}

void QETrainingDataCollectorModule::terminate()
{
  m_recorder->write();
  m_recorder.reset();
}
