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

#include <TRandom.h>


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
  m_eventData("EventMetaData", DataStore::c_Event)
//   m_testRootfile(nullptr),
//   m_testConfig(),
//   m_testTrainer(m_testConfig)
{
  InitializeVariables();

  //Set module properties
  setDescription("this module analyzes a big number of events (pGun or evtGen) to create raw sectorMaps which are needed for the VXDTF. This information will be exported via root files.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  addParam("spTCarrayName", m_PARAMspTCarrayName,
           "the name of the storeArray containing the SpacePointTrackCands used for the secMap-generation", string(""));

  addParam("allowTraining", m_PARAMallowTraining,
           "If true, training will be executed and filled into rootFiles, if not, only things like basf2 -m work but no training can be done",
           bool(false));


  if (m_PARAMallowTraining == false) return;
  // small lambda for getting random numbers:
  auto rngAppendix = []() -> int { return gRandom->Integer(std::numeric_limits<int>::max()); };


  /// TODO nice interface for creating SectorMapConfig:
  SectorMapConfig testData1;
//   testData1.pTCuts = {0.02, 3.5};
  testData1.pTmin = 0.02;
  testData1.pTmax = 3.5;
  testData1.pTSmear = 0.;
  testData1.allowedLayers = {0, 3, 4, 5, 6};
  testData1.uSectorDivider = { .15, .5, .85, 1.};
  testData1.vSectorDivider = { .1, .3, .5, .7, .9, 1.};
  testData1.pdgCodesAllowed = {};
  testData1.seedMaxDist2IPXY = 23.5;
  testData1.seedMaxDist2IPZ = 23.5;
  testData1.nHitsMin = 3;
  testData1.vIP = B2Vector3D(0, 0, 0);
  testData1.secMapName = "lowTestVXDTF";
  testData1.mField = 1.5;
  testData1.rarenessThreshold = 0.001;
  testData1.quantiles = {0.005, 0.005};

  std::string setupName = "testData1";
  VXDTFFilters<SpacePoint>* filters = new VXDTFFilters<SpacePoint>();
  filters->setConfig(testData1);

  // the filtesrsContainer taked ownership of the filters!
  FiltersContainer<SpacePoint>& filtersContainer = Belle2::FiltersContainer<SpacePoint>::getInstance();
  filtersContainer.assignFilters(setupName, filters);


  SecMapTrainer<XHitFilterFactory<SecMapTrainerHit> > newTrainer(setupName, std::to_string(rngAppendix()));

  m_secMapTrainers.push_back(std::move(newTrainer));
}



/// /// /// /// /// /// /// /// INITIALIZE /// /// /// /// /// /// /// ///
void SecMapTrainerVXDTFModule::initialize()
{
  B2INFO("~~~~~~~~~~~SecMapTrainerVXDTFModule - initialize ~~~~~~~~~~");
  if (m_PARAMallowTraining == false)
    B2FATAL("you want to execute SecMapTrainerVXDTF but the parameter 'allowTraining' is false! Aborting...");

  for (auto& trainer : m_secMapTrainers) {
    trainer.initialize();
  }

  //   m_testTrainer.initialize();
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
          " ~~~~~~~~~~");

  for (auto& trainer : m_secMapTrainers) {
    trainer.initializeEvent(thisExperiment, thisRun, thisEvent);
  }
  //   m_testTrainer.initializeEvent(thisExperiment, thisRun, thisEvent);

  //simulated particles and hits
  unsigned nSPTCs = m_spacePointTrackCands.getEntries();

  if (nSPTCs == 0) {
    B2DEBUG(1, "event " << thisEvent << ": there is no SpacePointTrackCandidate!");
    return;
  }
  B2DEBUG(5, "SecMapTrainerVXDTFModule, event " << thisEvent << ": size of array nSpacePointTrackCands: " << nSPTCs);




  /// loop over all TCs and all secMaps and assign fitting TCs to their maps, respectively.
  unsigned nAccepted = 0;
  for (unsigned iTC = 0; iTC not_eq nSPTCs; ++ iTC) {
    const SpacePointTrackCand* currentTC = m_spacePointTrackCands[iTC];
    B2DEBUG(10, "currens SPTC has got " << currentTC->getNHits() << " hits stored");

    //     bool accepted = m_testTrainer.storeTC(*currentTC, iTC);

    for (auto& trainer : m_secMapTrainers) {
      bool accepted = trainer.storeTC(*currentTC, iTC);
      nAccepted += (accepted ? 1 : 0);
    }
  }
  B2DEBUG(5, "SecMapTrainerVXDTFModule, event " << thisEvent << ": number of TCs total/accepted: " << nSPTCs << "/" << nAccepted);


  // process raw data:
  for (auto& trainer : m_secMapTrainers) {
    unsigned nTCsProcessed = trainer.processTracks();

    B2DEBUG(5, "SecMapTrainerVXDTFModule, event " << thisEvent << " with mapTrainer " << trainer.getConfig().secMapName <<
            ": number of TCs processed: " << nTCsProcessed <<
            ", calculations done!");
  }
  //   unsigned nTCsProcessed = m_testTrainer.processTracks();

  //   B2DEBUG(5, "SecMapTrainerVXDTFModule, event " << thisEvent << ": number of TCs processed: " << nTCsProcessed <<
  //           ", calculations done!");
}




/// /// /// /// /// /// /// /// TERMINATE /// /// /// /// /// /// /// ///
void SecMapTrainerVXDTFModule::terminate()
{
  B2DEBUG(1, " SecMapTrainerVXDTFModule::terminate:: start.");
  for (auto& trainer : m_secMapTrainers) {
    trainer.terminate();
  }
  B2INFO(" SecMapTrainerVXDTFModule, everything is done. Terminating.");
}
