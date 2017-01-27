/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Metzner                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/SPTCmomentumSeedRetrieverModule.h>
#include <framework/logging/Logger.h>
#include <geometry/bfieldmap/BFieldMap.h>

// ROOT
#include <TVector3.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;


REG_MODULE(SPTCmomentumSeedRetriever)

SPTCmomentumSeedRetrieverModule::SPTCmomentumSeedRetrieverModule() : Module()
{
  //Set module properties
  setDescription("A module for creating momentum seeds for spacepoint track candidates.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("tcArrayName", m_PARAMtcArrayName, " sets the name of expected StoreArray with SpacePointTrackCand in it.", string(""));

  addParam("stdPDGCode", m_PARAMstdPDGCode, " sets default PDG code for all track candidate in this module.", int(211));
}


void SPTCmomentumSeedRetrieverModule::beginRun()
{
  InitializeCounters();

  // now retrieving the bfield value used in this module
  m_bFieldZ = BFieldMap::Instance().getBField(TVector3(0, 0, 0)).Z();
  B2DEBUG(1, "SPTCmomentumSeedRetrieverModule:beginRun: B-Field z-component: " << m_bFieldZ);
}


void SPTCmomentumSeedRetrieverModule::event()
{
  m_eventCounter++;
  m_nTCsTotal += m_spacePointTrackCands.getEntries();
  B2DEBUG(1, "\n" << "SPTCmomentumSeedRetrieverModule:event: event " << m_eventCounter << ", got " <<
          m_spacePointTrackCands.getEntries() << " TCs\n");

  // create momentum seed for each given SpacePointTrackCand
  B2WARNING("Number of TCs in Event = " << m_spacePointTrackCands.getEntries());
  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {
    B2DEBUG(1, "\n" << "SPTCmomentumSeedRetrieverModule:event: this TC has got " << aTC.size() << " hits\n");
    createSPTCmomentumSeed(aTC);
  }

}


void SPTCmomentumSeedRetrieverModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;

  B2INFO("SPTCmomentumSeedRetrieverModule:endRun: events: " << m_eventCounter
         << ", nSPTCsPerEvent: " << invEvents * float(m_nTCsTotal)
        );
}


//    unsigned int createSPTCmomentumSeeds(SPTCContainerType& tcContainer, std::vector<std::vector<const SpacePoint*> > allPaths)
bool SPTCmomentumSeedRetrieverModule::createSPTCmomentumSeed(SpacePointTrackCand& aTC)
{
  // create tool for generating the momentum seed:
  auto seedGenerator = QualityEstimators();
  seedGenerator.resetMagneticField(m_bFieldZ);

  int chargeSignFactor = 0; /**< == 1 if pdg code is for a lepton, -1 if not. */
  int tempPDG = (m_PARAMstdPDGCode > 0 ? m_PARAMstdPDGCode : -m_PARAMstdPDGCode);
  if (tempPDG > 10 and tempPDG < 18) {
    // in this case, its a lepton. since leptons with positive sign have got negative codes, this must be taken into account
    chargeSignFactor = -1;
  } else { chargeSignFactor = 1; }
  TVectorD stateSeed(6); //(x,y,z,px,py,pz)
  TMatrixDSym covSeed(6);
  covSeed(0, 0) = 0.01 ; covSeed(1, 1) = 0.01 ; covSeed(2, 2) = 0.04 ; // 0.01 = 0.1^2 = dx*dx =dy*dy. 0.04 = 0.2^2 = dz*dz
  covSeed(3, 3) = 0.01 ; covSeed(4, 4) = 0.01 ; covSeed(5, 5) = 0.04 ;


  // containers for conversion to be compatible with old VXDTF-style interface:
  std::vector<PositionInfo> convertedSPTCrawData;
  convertedSPTCrawData.reserve(aTC.size());
  std::vector<PositionInfo*> convertedSPTC;
  convertedSPTC.reserve(aTC.size());

  // collecting actual hits
  // Inverting the hit sequence since the helix fit in the seedGenerator expects the hits in following order:
  // Outermost -> Innermost
  // While SPTCs store the hits from Innermost to Outermost.
  std::vector<const SpacePoint*> Hits = aTC.getHits();
  for (auto aHiti = aTC.getNHits(); aHiti > 0; --aHiti) {
    PositionInfo convertedHit{
      TVector3(Hits.at(aHiti - 1)->getPosition()),
      TVector3(Hits.at(aHiti - 1)->getPositionError()),
      0,
      0};
    convertedSPTCrawData.push_back(std::move(convertedHit));
    convertedSPTC.push_back(&convertedSPTCrawData.back());
  }
  seedGenerator.resetValues(&convertedSPTC);

  std::pair<TVector3, int> seedValue; // first is momentum vector, second is signCurvature

  // parameter means: false -> take last hit as seed hit (true would be other end), 0: do not artificially force a momentum value onto the seed).
  seedValue = seedGenerator.calcMomentumSeed(false,
                                             0); // ATTENTION: can throw an exception, therefore TODO: catche exception and return false if thrown, ATTENTION 2: check and verify that the seed is generated from the _innermost_ SP of the TC!

  int pdgCode = seedValue.second * m_PARAMstdPDGCode * chargeSignFactor; // improved one for curved tracks

  stateSeed(0) = (aTC.getHits().front()->X()); stateSeed(1) = (aTC.getHits().front()->Y());
  stateSeed(2) = (aTC.getHits().front()->Z());
  stateSeed(3) = seedValue.first[0]; stateSeed(4) = seedValue.first[1]; stateSeed(5) = seedValue.first[2];

  aTC.set6DSeed(stateSeed);
  aTC.setPdgCode(pdgCode);
  aTC.setCovSeed(covSeed);

  return true; // TODO: define cases for which a negative value shall be returned (e.g. seed creation failed)
}
