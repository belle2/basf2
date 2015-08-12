/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// Own include
#include <framework/gearbox/GearDir.h>
#include <arich/modules/arichDatabase/ARICHDatabaseModule.h>
#include <framework/core/ModuleManager.h>
#include <arich/dbobjects/ARICHAerogelInfo.h>
//#include <arich/dbobjects/ARICHHapdInfo.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// framework DB
#include <framework/datastore/DataStore.h>
#include <framework/database/DBStore.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/LocalDatabase.h>
#include <framework/database/ConditionsDatabase.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>


// ROOT
#include <TFile.h>
#include <TH1F.h>
#include <TObject.h>
#include <string>
#include <TClonesArray.h>

using namespace std;
//using namespace boost;

namespace Belle2 {
  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------

  REG_MODULE(ARICHDatabase)


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ARICHDatabaseModule::ARICHDatabaseModule() : Module()
  {
    // Set description()
    setDescription("ARICHDatabase module. Used to read data from database or write data from xml/root files to database.");

    // Add parameters
    addParam("runLow", m_runLow, "IOV:  run lowest", 0);
    addParam("runHigh", m_runHigh, "IOV:  run highest", 0);
    addParam("wr", m_wr, "writing to/reading from DB: writing='w', reading='r'", string(""));
  }

  ARICHDatabaseModule::~ARICHDatabaseModule()
  {
  }

  void ARICHDatabaseModule::initialize()
  {
    m_content = GearDir("/AerogelData/Content");
  }

  void ARICHDatabaseModule::beginRun()
  {
  }


  void ARICHDatabaseModule::event()
  {

    if (m_wr == "w") { // writing data to DB

      StoreObjPtr<EventMetaData> evtAgel;
      DataStore::Instance().setInitializeActive(true);
      evtAgel.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);

      TClonesArray agelConstants("Belle2::ARICHAerogelInfo");
      const auto name = DBStore::arrayName<ARICHAerogelInfo>("testAerogel");
      int agel = 0;

//      auto expNo = evtAgel->getExperiment();
//      IntervalOfValidity iov(expNo, m_runLow, expNo, m_runHigh);
      IntervalOfValidity iov(0, 0, 0, 0); // IOV (0,0,0,0) is valid for all runs and experiments

      for (const auto& aerogel : m_content.getNodes("aerogel")) {
        m_version = aerogel.getDouble("version");
        m_serial = aerogel.getString("serial");
        m_id = aerogel.getString("id");
        m_index = aerogel.getDouble("index");
        m_trlen = aerogel.getDouble("translength");
        m_thickness = aerogel.getDouble("thick");
        for (const auto& transmittance : aerogel.getNodes("transmittance/transpoint")) {
          lambda = transmittance.getInt("@lambda");
          double lambda = (double) lambda;
          val = transmittance.getDouble(".");
          m_lambda.push_back(lambda);
          m_transmittance.push_back(val);
        }

        new(agelConstants[agel]) ARICHAerogelInfo(m_version, m_serial, m_id, m_index, m_trlen, m_thickness);
        // agelConstants[agel]->SetUniqueID(agel);
        // cout << m_version << "," << m_serial << "," << m_id << "," << m_index << "," << m_trlen << "," << m_thickness << endl;
        agel++;
      }

      Database::Instance().storeData(name, &agelConstants, iov);
    }


    if (m_wr == "r") { // reading from DB

      // Get payload from pnnl (it saves root file to /tmp)
      EventMetaData evtAgel;
//      auto expNo = evtAgel->getExperiment();
//      IntervalOfValidity iov(expNo, m_runLow, expNo, m_runHigh);
      IntervalOfValidity iov(0, 0, 0, 0);
      const auto name = DBStore::arrayName<ARICHAerogelInfo>("testAerogel");
      Database::Instance().getData(evtAgel, name);


      // Read data directly from pnnl - not working yet
      /* //      auto expNo = evtAgel->getExperiment();
      //      IntervalOfValidity iov(expNo, m_runLow, expNo, m_runHigh);
            IntervalOfValidity iov(0,0,0,0);
            DBArray<ARICHAerogelInfo> objects;
            const auto name = DBStore::arrayName<ARICHAerogelInfo>("testAerogel");
            objects.getEntries(); */
    }
  }

  void ARICHDatabaseModule::endRun()
  {
  }

  void ARICHDatabaseModule::terminate()
  {
    B2INFO("ARICHDatabase finished.");
  }

  void ARICHDatabaseModule::printModuleParams() const
  {
  }

} // end Belle2 namespace
