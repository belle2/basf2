/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2012 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jakob Lettenbichler                                      *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/trackFindingVXD/filterMap/map/FiltersContainer.h>
#include <tracking/trackFindingVXD/sectorMapTools/SecMapTrainer.h>

#include <tracking/modules/vxdtfRedesign/VXDTFTrainingDataCollectorModule.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>

#include <TRandom.h>

using namespace std;
using namespace Belle2;

REG_MODULE(VXDTFTrainingDataCollector)

/**
 * Constructor
 */

VXDTFTrainingDataCollectorModule::VXDTFTrainingDataCollectorModule() :
  Module(),
  m_eventData("EventMetaData", DataStore::c_Event)
{

  //Set module properties
  setDescription("This module collect the data needed to train the VXDTF sector map. The data are saved on a root file that eventually will be chained and used by the training module.");
  setPropertyFlags(c_ParallelProcessingCertified |
                   c_TerminateInAllProcesses);

  addParam("SpacePointTrackCandsName", m_PARAMSpacePointTrackCandsName,
           "the name of the storeArray containing the SpacePointTrackCands used for extracting and collecting the training data.", string(""));

  addParam("NameTag", m_PARAMNameTag, "A name tag that will be attached to the name of the output file. If left empty (\"\") a "
           "random number will be attached!", std::string(""));
}

/**
 * Initialization
 */

void VXDTFTrainingDataCollectorModule::initialize()
{

  FiltersContainer<SpacePoint>& filtersContainer = Belle2::FiltersContainer<SpacePoint>::getInstance();

  m_spacePointTrackCands.isRequired(m_PARAMSpacePointTrackCandsName);

  for (auto setup : filtersContainer.getAllSetups()) {
    auto config = setup.second->getConfig();

    std::string nameAppendix = m_PARAMNameTag;
    // if the name tag was not set a random number will be attached!
    if (nameAppendix == std::string("")) {
      int randomInt = gRandom->Integer(std::numeric_limits<int>::max());
      nameAppendix = std::to_string(randomInt);
    }

    SecMapTrainer<SelectionVariableFactory<SecMapTrainerHit> >
    newMap(setup.first, nameAppendix);

    m_secMapTrainers.push_back(std::move(newMap));

  }

  for (auto& trainer : m_secMapTrainers) {
    trainer.initialize();
  }


}


/// /// /// /// /// /// /// /// EVENT /// /// /// /// /// /// /// ///
void VXDTFTrainingDataCollectorModule::event()
{
  //get the data
  int thisExperiment = m_eventData->getExperiment();
  int thisRun = m_eventData->getRun();
  int thisEvent = m_eventData->getEvent();

  for (auto& collector : m_secMapTrainers)
    collector.initializeEvent(thisExperiment, thisRun, thisEvent);


  // number of space point track candidates from the MC track finder
  unsigned nSPTCs = m_spacePointTrackCands.getEntries();

  /// loop over all TCs and feed each collector
  /// the collector will decide if the TC is useful for training or not

  for (unsigned iTC = 0; iTC < nSPTCs; ++ iTC) {
    const SpacePointTrackCand* currentTC = m_spacePointTrackCands[iTC];
    for (auto& dataCollector : m_secMapTrainers)
      dataCollector.storeTC(*currentTC, iTC);
  }

  // process raw data:
  for (auto& dataCollector : m_secMapTrainers) {
    unsigned nTCsProcessed = dataCollector.processTracks();

    B2DEBUG(5,
            "VXDTFTrainingDataCollectorModule, event " << thisEvent <<
            " with mapTrainer " << dataCollector.getConfig().secMapName <<
            ": number of TCs processed: " << nTCsProcessed <<
            ", calculations done!");
  }
}

/**
 * Terminate
 */
void VXDTFTrainingDataCollectorModule::terminate()
{
  for (auto& dataCollector : m_secMapTrainers)
    dataCollector.terminate();
}
