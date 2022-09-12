/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/SPTCmomentumSeedRetrieverModule.h>
#include <framework/logging/Logger.h>
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;


REG_MODULE(SPTCmomentumSeedRetriever);

SPTCmomentumSeedRetrieverModule::SPTCmomentumSeedRetrieverModule() : Module()
{
  //Set module properties
  setDescription("A module for creating momentum seeds for spacepoint track candidates.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("tcArrayName", m_PARAMtcArrayName, " sets the name of expected StoreArray with SpacePointTrackCand in it.",
           std::string(""));
}


void SPTCmomentumSeedRetrieverModule::beginRun()
{
  InitializeCounters();

  // BField is required by all QualityEstimators
  double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;

  m_estimator = QualityEstimatorRiemannHelixFit();
  m_estimator.setMagneticFieldStrength(bFieldZ);

  B2DEBUG(20, "SPTCmomentumSeedRetrieverModule:beginRun: B-Field z-component: " << m_bFieldZ);
}


void SPTCmomentumSeedRetrieverModule::event()
{
  m_eventCounter++;
  m_nTCsTotal += m_spacePointTrackCands.getEntries();
  B2DEBUG(20, "\n" << "SPTCmomentumSeedRetrieverModule:event: event " << m_eventCounter << ", got " <<
          m_spacePointTrackCands.getEntries() << " TCs\n");

  // create momentum seed for each given SpacePointTrackCand
  B2DEBUG(20, "Number of TCs in Event = " << m_spacePointTrackCands.getEntries()); // demoted to a B2DEBUG, has been a warning
  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {
    if (!aTC.hasRefereeStatus(SpacePointTrackCand::c_isActive)) continue;
    B2DEBUG(20, "\n" << "SPTCmomentumSeedRetrieverModule:event: this TC has got " << aTC.size() << " hits\n");
    createSPTCmomentumSeed(aTC);
  }

}


void SPTCmomentumSeedRetrieverModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;

  B2DEBUG(1, "SPTCmomentumSeedRetrieverModule:endRun: events: " << m_eventCounter
          << ", nSPTCsPerEvent: " << invEvents * float(m_nTCsTotal)
         );
}


bool SPTCmomentumSeedRetrieverModule::createSPTCmomentumSeed(SpacePointTrackCand& aTC)
{
  TVectorD stateSeed(6); //(x,y,z,px,py,pz)
  TMatrixDSym covSeed(6);
  // TODO: find out where these numbers come from!
  covSeed(0, 0) = 0.01 ; covSeed(1, 1) = 0.01 ; covSeed(2, 2) = 0.04 ; // 0.01 = 0.1^2 = dx*dx =dy*dy. 0.04 = 0.2^2 = dz*dz
  covSeed(3, 3) = 0.01 ; covSeed(4, 4) = 0.01 ; covSeed(5, 5) = 0.04 ;

  auto sortedHits = aTC.getSortedHits();

  QualityEstimationResults results = m_estimator.estimateQualityAndProperties(sortedHits);

  stateSeed(0) = (sortedHits.front()->X());
  stateSeed(1) = (sortedHits.front()->Y());
  stateSeed(2) = (sortedHits.front()->Z());
  if (results.p) {
    auto momentumSeed = *(results.p);
    stateSeed(3) = momentumSeed.X();
    stateSeed(4) = momentumSeed.Y();
    stateSeed(5) = momentumSeed.Z();
  } else {
    stateSeed(3) = 0;
    stateSeed(4) = 0;
    stateSeed(5) = 0;
  }

  aTC.set6DSeed(stateSeed);
  aTC.setCovSeed(covSeed);

  double chargeSeed = results.curvatureSign ? -1 * (*(results.curvatureSign)) : 0;
  aTC.setChargeSeed(chargeSeed);

  return (results.p && results.curvatureSign);
}
