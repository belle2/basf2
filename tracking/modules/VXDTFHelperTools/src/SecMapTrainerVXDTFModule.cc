/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2012 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jakob Lettenbichler                                      *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/modules/VXDTFHelperTools/SecMapTrainerVXDTFModule.h>


using namespace std;
using namespace Belle2;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SecMapTrainerVXDTF)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------


/// /// /// /// /// /// /// /// CONSTRUCTOR /// /// /// /// /// /// /// ///
SecMapTrainerVXDTFModule::SecMapTrainerVXDTFModule() :
  Module(),
  m_eventData("EventMetaData", DataStore::c_Event),
  m_testRootfile(new TFile("testFile.root", "RECREATE")),
  m_testConfig(),
  m_testTrainer(m_testConfig, m_testRootfile)
{
  InitializeVariables();

  //Set module properties
  setDescription("this module analyzes a big number of events (pGun or evtGen) to create raw sectorMaps which are needed for the VXDTF. This information will be exported via root files.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("spTCarrayName", m_PARAMspTCarrayName,
           "the name of the storeArray containing the SpacePointTrackCands used for the secMap-generation", string(""));
}



/// /// /// /// /// /// /// /// INITIALIZE /// /// /// /// /// /// /// ///
void SecMapTrainerVXDTFModule::initialize()
{
  B2INFO("~~~~~~~~~~~SecMapTrainerVXDTFModule - initialize ~~~~~~~~~~")
  m_testTrainer.initialize();
  m_spacePointTrackCands.isRequired(m_PARAMspTCarrayName);
}


/// /// /// /// /// /// /// /// EVENT /// /// /// /// /// /// /// ///
void SecMapTrainerVXDTFModule::event()
{
  //get the data
  int thisExperiment = m_eventData->getExperiment();
  int thisRun = m_eventData->getRun();
  int thisEvent = m_eventData->getEvent();
  B2DEBUG(5, "~~~~~~~~~~~SecMapTrainerVXDTFModule - experiment/run/event " << thisExperiment << "/" << thisRun << "/" << thisEvent <<
          " ~~~~~~~~~~")
  m_testTrainer.initializeEvent(thisExperiment, thisRun, thisEvent);

  //simulated particles and hits
  unsigned nSPTCs = m_spacePointTrackCands.getEntries();

  if (nSPTCs == 0) {
    B2WARNING("event " << thisEvent << ": there is no SpacePointTrackCandidate!")
    return;
  }

  B2DEBUG(5, "SecMapTrainerVXDTFModule, event " << thisEvent << ": size of array nSpacePointTrackCands: " << nSPTCs);


  unsigned nAccepted = 0;
  for (unsigned iTC = 0; iTC not_eq nSPTCs; ++ iTC) {
    const SpacePointTrackCand* currentTC = m_spacePointTrackCands[iTC];
    B2DEBUG(10, "currens SPTC has got " << currentTC->getNHits() << " hits stored")

    bool accepted = m_testTrainer.acceptTC(*currentTC, iTC);
    nAccepted += (accepted ? 1 : 0);
//     for (auto* secMap : m_sectorMaps) {
//       chosenMap++;
//       /// can be accepted by several secMaps, because of momentum range or whatever:
//       bool accepted = checkAcceptanceOfSecMap(secMap, currentTC);
//
//       B2DEBUG(15, "currens SPTC with " << currentTC->getNHits() <<
//               " hits stored and pT of " << currentTC->getMomSeed().Perp() <<
//               "GeV/c was " << (accepted ? string("accepted") : string("rejected")) <<
//               " by secMap " << secMap->getName() <<
//               " having momCuts: " << secMap->getPtCuts().first <<
//               "/" << secMap->getPtCuts().second)
//       if (!accepted) continue;
//       m_trackletMomentumCounter.at(chosenMap - 1)++;
//
//       /// converts to internal data structure and attaches it to given secMap
//       VXDTrack newTrack = convertSPTC2VXDTrack(secMap, currentTC);
//
//       trackletsOfEvent.push_back(std::move(newTrack)); // nTracklets >= nSPTCs
//     }
  }
  B2DEBUG(5, "SecMapTrainerVXDTFModule, event " << thisEvent << ": number of TCs accepted: " << nAccepted);


  unsigned nTCsProcessed = m_testTrainer.processTracks();

  B2DEBUG(5, "SecMapTrainerVXDTFModule, event " << thisEvent << ": number of TCs processed: " << nTCsProcessed);

  B2DEBUG(5, "SecMapTrainerVXDTFModule - event " << thisEvent << ", calculations done!")
}




/// /// /// /// /// /// /// /// TERMINATE /// /// /// /// /// /// /// ///
void SecMapTrainerVXDTFModule::terminate()
{
  B2DEBUG(1, " SecMapTrainerVXDTFModule::terminate:: start.")
  m_testTrainer.terminate();
  B2INFO(" SecMapTrainerVXDTFModule, everything is done. Terminating.")
}
