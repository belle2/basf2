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
  m_testRootfile(nullptr),
  m_testConfig(),
  m_testTrainer(m_testConfig, m_testRootfile)
{
  InitializeVariables();

  //Set module properties
  setDescription("this module analyzes a big number of events (pGun or evtGen) to create raw sectorMaps which are needed for the VXDTF. This information will be exported via root files.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  addParam("spTCarrayName", m_PARAMspTCarrayName,
           "the name of the storeArray containing the SpacePointTrackCands used for the secMap-generation", string(""));
}



/// /// /// /// /// /// /// /// INITIALIZE /// /// /// /// /// /// /// ///
void SecMapTrainerVXDTFModule::initialize()
{
  B2INFO("~~~~~~~~~~~SecMapTrainerVXDTFModule - initialize ~~~~~~~~~~")
  m_testRootfile = new TFile("testName.root", "RECREATE");
  m_testRootfile->cd();
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
    B2DEBUG(1, "event " << thisEvent << ": there is no SpacePointTrackCandidate!")
    return;
  }
  B2DEBUG(5, "SecMapTrainerVXDTFModule, event " << thisEvent << ": size of array nSpacePointTrackCands: " << nSPTCs);


  /// loop over all TCs and all secMaps and assign fitting TCs to their maps, respectively.
  unsigned nAccepted = 0;
  for (unsigned iTC = 0; iTC not_eq nSPTCs; ++ iTC) {
    const SpacePointTrackCand* currentTC = m_spacePointTrackCands[iTC];
    B2DEBUG(10, "currens SPTC has got " << currentTC->getNHits() << " hits stored")

    bool accepted = m_testTrainer.storeTC(*currentTC, iTC);
    nAccepted += (accepted ? 1 : 0);
//     for (auto* secMap : m_sectorMaps) {
    /// TODO different sectorMaps (have to be stored in a vector of secMaps...).
//     }
  }
  B2DEBUG(5, "SecMapTrainerVXDTFModule, event " << thisEvent << ": number of TCs accepted: " << nAccepted);


  // process raw data:
  unsigned nTCsProcessed = m_testTrainer.processTracks();

  B2DEBUG(5, "SecMapTrainerVXDTFModule, event " << thisEvent << ": number of TCs processed: " << nTCsProcessed <<
          ", calculations done!");
}




/// /// /// /// /// /// /// /// TERMINATE /// /// /// /// /// /// /// ///
void SecMapTrainerVXDTFModule::terminate()
{
  B2DEBUG(1, " SecMapTrainerVXDTFModule::terminate:: start.")
  m_testRootfile->cd();
  m_testTrainer.terminate(m_testRootfile);
  B2INFO(" SecMapTrainerVXDTFModule, everything is done. Terminating.")
}
