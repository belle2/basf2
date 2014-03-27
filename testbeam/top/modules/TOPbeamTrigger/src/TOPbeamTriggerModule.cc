/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <testbeam/top/modules/TOPbeamTrigger/TOPbeamTriggerModule.h>

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <testbeam/top/dataobjects/TOPTBSimHit.h>



using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPbeamTrigger)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPbeamTriggerModule::TOPbeamTriggerModule() : Module()

  {
    // set module description (e.g. insert text)
    setDescription("Emulation of test beam trigger: returns true if trigger fires");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    std::vector<int> defaultListInt;
    std::vector<double> defaultListDouble;
    addParam("detectorIDs", m_detectorIDs,
             "a list of detector ID's included in trigger", defaultListInt);
    addParam("thresholds", m_thresholds,
             "a list of detector thresholds in MeV", defaultListDouble);

  }

  TOPbeamTriggerModule::~TOPbeamTriggerModule()
  {
  }

  void TOPbeamTriggerModule::initialize()
  {

    if (m_thresholds.size() < m_detectorIDs.size())
      B2ERROR("some thresholds are not given");

  }

  void TOPbeamTriggerModule::beginRun()
  {
  }

  void TOPbeamTriggerModule::event()
  {

    std::vector<double> energyDeposit;
    for (unsigned k = 0; k < m_detectorIDs.size(); k++) {
      energyDeposit.push_back(0.0);
    }

    StoreArray<TOPTBSimHit> simHits;
    for (int i = 0; i < simHits.getEntries(); i++) {
      TOPTBSimHit* simHit = simHits[i];
      int counterID = simHit->getCounterID();
      for (unsigned k = 0; k < m_detectorIDs.size(); k++) {
        if (counterID == m_detectorIDs[k]) {
          energyDeposit[k] += simHit->getEnergyDeposit();
        }
      }
    }

    bool fired = true;
    for (unsigned k = 0; k < m_detectorIDs.size(); k++) {
      if (energyDeposit[k] < m_thresholds[k]) {
        fired = false;
        break;
      }
    }

    setReturnValue(fired);

  }


  void TOPbeamTriggerModule::endRun()
  {
  }

  void TOPbeamTriggerModule::terminate()
  {
  }

  void TOPbeamTriggerModule::printModuleParams() const
  {
  }


} // end Belle2 namespace

