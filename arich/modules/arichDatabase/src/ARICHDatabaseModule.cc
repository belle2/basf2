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
#include <arich/dbobjects/ARICHHapdQA.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// framework DB
#include <framework/datastore/DataStore.h>
#include <framework/database/DBStore.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>


// ROOT
#include <TFile.h>
#include <TH1F.h>
#include <TH1.h>
#include <TH1S.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TObject.h>
#include <TKey.h>
#include <TNamed.h>
#include <string>
#include <TClonesArray.h>
#include <TCanvas.h>


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
    vector<string> emptyvector;
    addParam("runLow", m_runLow, "IOV:  run lowest", 0);
    addParam("runHigh", m_runHigh, "IOV:  run highest", 0);
    addParam("wr", m_wr, "writing to (W)/reading from (R) DB: Wagel, Ragel, WhapdQA, RhapdQA", string(""));
    addParam("InputFileNames", m_inputFileNames, "Input root files", emptyvector);
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

    if (m_wr == "Wagel") { // writing aerogel data to DB

      StoreObjPtr<EventMetaData> evtAgel;
      DataStore::Instance().setInitializeActive(true);
      evtAgel.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);

      TClonesArray agelConstants("Belle2::ARICHAerogelInfo");
      const auto name = DBStore::arrayName<ARICHAerogelInfo>("testAerogel3");
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
          double Lambda = (double) lambda;
          val = transmittance.getDouble(".");
          m_lambda.push_back(Lambda);
          m_transmittance.push_back(val);
        }

        new(agelConstants[agel]) ARICHAerogelInfo(m_version, m_serial, m_id, m_index, m_trlen, m_thickness, m_lambda, m_transmittance);

        m_lambda.clear();
        m_transmittance.clear();
        agel++;
      }

      Database::Instance().storeData(name, &agelConstants, iov);
    }


    if (m_wr == "Ragel") { // reading from DB

      DBArray<ARICHAerogelInfo> elements("testAerogel3");
      elements.getEntries();

      /* // Print aerogel info

            unsigned int el = 0;
            for(const auto& element : elements) {
              B2INFO ("Element Number: " << el << "; version = " << element.getAerogelVersion() << ", serial = " << element.getAerogelSerial() << ", id = " << element.getAerogelID() << ", n = " << element.getAerogelRefractiveIndex() << ", transmLength = " << element.getAerogelTransmissionLength() << ", thickness = " << element.getAerogelThickness());
              for (int j = 0; j < 53; j++) {
                B2INFO ("lambda - transmittance = " << element.getLambdaTransmittance(j) << " - " << element.getAerogelTransmittance(j) << "; ");
              }
              el++;
            }
      */

    }


    if (m_wr == "WhapdQA") { // writing hapd QA data to DB

      StoreObjPtr<EventMetaData> evtHapdQA;
      DataStore::Instance().setInitializeActive(true);
      evtHapdQA.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);

      TClonesArray hapdQAConstants("Belle2::ARICHHapdQA");
      const auto name = DBStore::arrayName<ARICHHapdQA>("testHapd");

//      auto expNo = evtAgel->getExperiment();
//      IntervalOfValidity iov(expNo, m_runLow, expNo, m_runHigh);
      IntervalOfValidity iov(0, 0, 0, 0); // IOV (0,0,0,0) is valid for all runs and experiments

      int hapd = 0;

      //   TFile file2("histos12f.root", "recreate"); // create file for saving TH1S histograms

      for (const string& inputFile : m_inputFileNames) {

        TFile* f = TFile::Open(inputFile.c_str(), "READ"); //v python skripto

        int size = inputFile.length();
        m_hapdSerial = inputFile.substr(size - 16, 6);

        TIter next(f->GetListOfKeys());
        TKey* key;
        while ((key = (TKey*)next())) {

          std::string strime = key->GetName();

          if (strime.find("gcurrent") == 0) {
            TGraph* hhist1 = (TGraph*)f->Get(strime.c_str());
            m_leakCurrent.push_back(hhist1);
          }

          else if (strime.find("h2dscan") == 0) {
            TH2F* hitData2D = (TH2F*)f->Get(strime.c_str());
            m_hitData2D = hitData2D;
          }

          else if (strime.find("gnoise_ch") == 0) {
            TGraph* hhist2 = (TGraph*)f->Get(strime.c_str());
            m_noise.push_back(hhist2);
          }

          else if (strime.find("hchscan") == 0) {
            TH1F* hhist3 = (TH1F*)f->Get(strime.c_str());

            // conversion TH1F -> TH1S

            const char* hhist3_ime = hhist3->GetName();
            const char* hhist3_naslov = hhist3->GetTitle();
            int hhist3_nbins = hhist3->GetSize();
            TH1S* hhist3short = new TH1S(hhist3_ime, hhist3_naslov, hhist3_nbins - 2, 0, 1);
            for (int bin = 0; bin < hhist3_nbins; bin++) {
              hhist3short->SetBinContent(bin, hhist3->GetBinContent(bin));
            }
            m_hitCount.push_back(hhist3short);

            //    file2.cd();
            //    hhist3short->Write();

          }

          else { B2INFO("Key name does not match any of the following: gcurrent, 2dscan, gnoise, hchscan!"); }
        }

        m_measurementDate = {};

        new(hapdQAConstants[hapd]) ARICHHapdQA(m_hapdSerial, m_measurementDate, m_leakCurrent, m_hitData2D, m_noise, m_hitCount);
        m_noise.clear();
        m_leakCurrent.clear();
        m_hitCount.clear();

        hapd++;
      }

      Database::Instance().storeData(name, &hapdQAConstants, iov);
    }


    if (m_wr == "RhapdQA") { // reading from DB

      DBArray<ARICHHapdQA> elements("testHapd");
      elements.getEntries();

      /* // Print serial numbers of HAPDs and save graphs (Leakage current) to root file

            unsigned int el = 0;
            for(const auto& element : elements) {
              B2INFO ("Element Number: " << el << "; serial = " << element.getHapdSerialNumber());
              if (el == 1) {
                TFile fileRhapd("hapdLeakage.root", "new");
                for (unsigned int j = 0; j < 10; j++) {
                  TGraph* graf = element.getHapdLeakCurrent(j);
                  fileRhapd.cd();
                  graf->Write();
                }

              }
              el++;
            }
      */
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
