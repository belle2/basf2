/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2012 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jakob Lettenbichler                                      *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include "tracking/trackFindingVXD/sectorMapTools/SectorMap.h"

#include <tracking/modules/VXDTFHelperTools/SecMapTrainerBaseModule.h>
#include "tracking/trackFindingVXD/sectorMapTools/SectorMap.h"
#include "framework/datastore/StoreObjPtr.h"

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
{
  InitializeVariables();

  //Set module properties
  setDescription("this module analyzes a big number of events (pGun or evtGen) to create raw sectorMaps which are needed for the VXDTF 2.0. This information will be exported via root files.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  addParam("spTCarrayName", m_PARAMspTCarrayName,
           "the name of the storeArray containing the SpacePointTrackCands used for the secMap-generation", string(""));

  // small lambda for getting random numbers:
  auto rngAppendix = []() -> int { return gRandom->Integer(std::numeric_limits<int>::max()); };
  // What does it mean "Appendix"? E.P.

  StoreObjPtr< SectorMap > sectorMap("", DataStore::c_Persistent);
  for (auto setup : sectorMap->getAllSetups()) {
    auto config = setup.second->getConfig();
    SecMapTrainer<SelectionVariableFactory<SecMapTrainerHit> > newMap(config, rngAppendix());
    m_secMapTrainers.push_back(std::move(newMap));
  }
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
