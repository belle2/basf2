/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2012 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jakob Lettenbichler                                      *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/modules/VXDTFHelperTools/SecMapTrainerBaseModule.h>
#include <TRandom.h>

using namespace std;
using namespace Belle2;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SecMapTrainerBase)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------


/// /// /// /// /// /// /// /// CONSTRUCTOR /// /// /// /// /// /// /// ///
SecMapTrainerBaseModule::SecMapTrainerBaseModule() :
  Module(),
  m_eventData("EventMetaData", DataStore::c_Event)
//   m_testConfig(),
//   m_testTrainer(m_testConfig)
{
  InitializeVariables();

  //Set module properties
  setDescription("this module analyzes a big number of events (pGun or evtGen) to create raw sectorMaps which are needed for the VXDTF 2.0. This information will be exported via root files.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  addParam("spTCarrayName", m_PARAMspTCarrayName,
           "the name of the storeArray containing the SpacePointTrackCands used for the secMap-generation", string(""));

  // small lambda for getting random numbers:
  auto rngAppendix = []() -> int { return gRandom->Integer(std::numeric_limits<int>::max()); };

  /// TODO nice interface for creating TrainerConfigData:
  TrainerConfigData testData1;
  testData1.pTCuts = {0.02, 0.08};
  testData1.pTSmear = 0.;
  testData1.minMaxLayer = {3, 6};
  testData1.uDirectionCuts = {0., .15, .5, .85, 1.};
  testData1.vDirectionCuts = {0., .1, .3, .5, .7, .9, 1.};
  testData1.pdgCodesAllowed = {};
  testData1.seedMaxDist2IPXY = 23.5;
  testData1.seedMaxDist2IPZ = 23.5;
  testData1.nHitsMin = 3;
  testData1.vIP = B2Vector3D(0, 0, 0);
  testData1.secMapName = "lowTestRedesign";
  testData1.twoHitFilters = { "Distance3DSquared", "Distance2DXYSquared", "SlopeRZ", "CircleDist2IPHighOccupancy"};
  testData1.threeHitFilters = { "Angle3DSimple", "DeltaSlopeRZ"};
  testData1.fourHitFilters = { "DeltaDistCircleCenter", "DeltaCircleRadius"};
  testData1.mField = 1.5;
  testData1.rarenessThreshold = 0.001;
  testData1.quantiles = {0.005, 0.005};
  SecMapTrainer<SelectionVariableFactory<SecMapTrainerHit> > newMap1(testData1, rngAppendix());
  m_secMapTrainers.push_back(std::move(newMap1));

  TrainerConfigData testData2;
  testData2.pTCuts = {0.075, 0.300};
  testData2.pTSmear = 0.;
  testData2.minMaxLayer = {3, 6};
  testData2.uDirectionCuts = {0., .15, .5, .85, 1.};
  testData2.vDirectionCuts = {0., .1, .3, .5, .7, .9, 1.};
  testData2.pdgCodesAllowed = {};
  testData2.seedMaxDist2IPXY = 23.5;
  testData2.seedMaxDist2IPZ = 23.5;
  testData2.nHitsMin = 3;
  testData2.vIP = B2Vector3D(0, 0, 0);
  testData2.secMapName = "medTestRedesign";
  testData2.twoHitFilters = { "Distance3DSquared", "Distance2DXYSquared", "SlopeRZ", "CircleDist2IPHighOccupancy"};
  testData2.threeHitFilters = { "Angle3DSimple", "DeltaSlopeRZ"};
  testData2.fourHitFilters = { "DeltaDistCircleCenter", "DeltaCircleRadius"};
  testData2.mField = 1.5;
  testData2.rarenessThreshold = 0.001;
  testData2.quantiles = {0.005, 0.005};
  SecMapTrainer<SelectionVariableFactory<SecMapTrainerHit> > newMap2(testData2, rngAppendix());
  m_secMapTrainers.push_back(std::move(newMap2));

  TrainerConfigData testData3;
  testData3.pTCuts = {0.290, 3.5};
  testData3.pTSmear = 0.;
  testData3.minMaxLayer = {3, 6};
  testData3.uDirectionCuts = {0., .15, .5, .85, 1.};
  testData3.vDirectionCuts = {0., .1, .3, .5, .7, .9, 1.};
  testData3.pdgCodesAllowed = {};
  testData3.seedMaxDist2IPXY = 23.5;
  testData3.seedMaxDist2IPZ = 23.5;
  testData3.nHitsMin = 3;
  testData3.vIP = B2Vector3D(0, 0, 0);
  testData3.secMapName = "highTestRedesign";
  testData3.twoHitFilters = { "Distance3DSquared", "Distance2DXYSquared", "SlopeRZ"};
  testData3.threeHitFilters = { "Angle3DSimple", "DeltaCircleRadiusHighOccupancy"};
  testData3.fourHitFilters = { "DeltaDistCircleCenter", "DeltaCircleRadius"};
  testData3.mField = 1.5;
  testData3.rarenessThreshold = 0.001;
  testData3.quantiles = {0.005, 0.005};
  SecMapTrainer<SelectionVariableFactory<SecMapTrainerHit> > newMap3(testData3, rngAppendix());
  m_secMapTrainers.push_back(std::move(newMap3));
}



/// /// /// /// /// /// /// /// INITIALIZE /// /// /// /// /// /// /// ///
void SecMapTrainerBaseModule::initialize()
{
  B2INFO("~~~~~~~~~~~SecMapTrainerBaseModule - initialize ~~~~~~~~~~")
  for (auto& trainer : m_secMapTrainers) {
    trainer.initialize();
  }
  m_spacePointTrackCands.isRequired(m_PARAMspTCarrayName);
}


/// /// /// /// /// /// /// /// EVENT /// /// /// /// /// /// /// ///
void SecMapTrainerBaseModule::event()
{
  //get the data
  int thisExperiment = m_eventData->getExperiment();
  int thisRun = m_eventData->getRun();
  int thisEvent = m_eventData->getEvent();
  B2DEBUG(5, "~~~~~~~~~~~SecMapTrainerBaseModule - experiment/run/event " << thisExperiment << "/" << thisRun << "/" << thisEvent <<
          " ~~~~~~~~~~")

  for (auto& trainer : m_secMapTrainers) {
    trainer.initializeEvent(thisExperiment, thisRun, thisEvent);
  }

  //simulated particles and hits
  unsigned nSPTCs = m_spacePointTrackCands.getEntries();

  if (nSPTCs == 0) {
    B2DEBUG(1, "event " << thisEvent << ": there is no SpacePointTrackCandidate!")
    return;
  }
  B2DEBUG(5, "SecMapTrainerBaseModule, event " << thisEvent << ": size of array nSpacePointTrackCands: " << nSPTCs);


  /// loop over all TCs and all secMaps and assign fitting TCs to their maps, respectively.
  unsigned nAccepted = 0;
  for (unsigned iTC = 0; iTC not_eq nSPTCs; ++ iTC) {
    const SpacePointTrackCand* currentTC = m_spacePointTrackCands[iTC];
    B2DEBUG(10, "currens SPTC has got " << currentTC->getNHits() << " hits stored")

    for (auto& trainer : m_secMapTrainers) {
      bool accepted = trainer.storeTC(*currentTC, iTC);
      nAccepted += (accepted ? 1 : 0);
    }
  }
  B2DEBUG(5, "SecMapTrainerBaseModule, event " << thisEvent << ": number of TCs total/accepted: " << nSPTCs << "/" << nAccepted);


  // process raw data:
  for (auto& trainer : m_secMapTrainers) {
    unsigned nTCsProcessed = trainer.processTracks();

    B2DEBUG(5, "SecMapTrainerBaseModule, event " << thisEvent << " with mapTrainer " << trainer.getConfig().secMapName <<
            ": number of TCs processed: " << nTCsProcessed <<
            ", calculations done!");
  }
}




/// /// /// /// /// /// /// /// TERMINATE /// /// /// /// /// /// /// ///
void SecMapTrainerBaseModule::terminate()
{
  B2DEBUG(1, " SecMapTrainerBaseModule::terminate:: start.")
  for (auto& trainer : m_secMapTrainers) {
    trainer.terminate();
  }
  B2INFO(" SecMapTrainerBaseModule, everything is done. Terminating.")
}
