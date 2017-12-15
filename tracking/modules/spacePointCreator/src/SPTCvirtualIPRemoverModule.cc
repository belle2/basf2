/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/SPTCvirtualIPRemoverModule.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector3.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;


REG_MODULE(SPTCvirtualIPRemover)

SPTCvirtualIPRemoverModule::SPTCvirtualIPRemoverModule() : Module()
{
  InitializeCounters();

  //Set module properties
  setDescription("The quality estimator module for SpacePointTrackCandidates using a circleFit.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("tcArrayName", m_PARAMtcArrayName, " sets the name of expected StoreArray with SpacePointTrackCand in it", string(""));

  addParam("doCheckOnly", m_PARAMdoCheckOnly, " if true, no vIP is removed, but only nVIPs are counted.", bool(false));

  addParam("maxTCLengthForVIPKeeping", m_PARAMmaxTCLengthForVIPKeeping,
           "If you want to keep the vIP only for short TCs, then set this value to the number of hits a TC is maximally allowed to have to not loose its vIP (number of hits without counting the vIP).",
           unsigned(3));
}


void SPTCvirtualIPRemoverModule::event()
{
  m_eventCounter++;
  B2DEBUG(1, "\n" << "SPTCvirtualIPRemoverModule:event: event " << m_eventCounter << "\n");
  m_nTCsTotal += m_spacePointTrackCands.getEntries();

  unsigned nTC = 0;
  // assign a QI computed using a circleFit for each given SpacePointTrackCand
  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {

    unsigned nHits = aTC.size();
    bool hasVIP = false;

    // search for first vIP in SPTC and remove if parameter doCheckOnly == false. all further VIPs are ignored!
    const std::vector<const SpacePoint*>& spacePoints = aTC.getHits();
    for (unsigned int iSp = 0; iSp < spacePoints.size(); ++iSp) {
      const SpacePoint* aHit = spacePoints[iSp];
      if (aHit->getType() == VXD::SensorInfoBase::SensorType::VXD) { // vIP found
        m_nVIPsTotal++;
        hasVIP = true;
        if (m_PARAMdoCheckOnly or (m_PARAMmaxTCLengthForVIPKeeping + 1 >= nHits)) continue;

        aTC.removeSpacePoint(iSp);
        m_nVIPsRemoved++;
        break; // stopping here, since now the list of SpacePoint* do not anymore more with actual Spacepoints in SPTC!
      }
    }

    B2DEBUG(1, "SPTCvirtualIPRemoverModule:event: event " << m_eventCounter
            << ": TC " << nTC
            << " with " << nHits
            << " hits has vIP: " << (hasVIP ? "true" : "false")
            << " and was removed " << (m_PARAMdoCheckOnly ? "false" : "true")
           );
    ++nTC;
  }

}


void SPTCvirtualIPRemoverModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;

  B2DEBUG(1, "SPTCvirtualIPRemoverModule:endRun: events: " << m_eventCounter
          << ", nSPTCsPerEvent: " << invEvents * float(m_nTCsTotal)
          << ", nVIPsPerEvent: " << invEvents * float(m_nVIPsTotal)
          << ", nVIPsRemovedPerEvent: " << invEvents * float(m_nVIPsRemoved)
         );
}
