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
#include <arich/dbobjects/ARICHAsicInfo.h>
#include <arich/dbobjects/ARICHHapdInfo.h>
#include <arich/dbobjects/ARICHHapdChipInfo.h>

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
#include <TH3F.h>
#include <TH1S.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TObject.h>
#include <TKey.h>
#include <string>
#include <fstream>
#include <vector>
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
    vector<string> emptyvector;
    addParam("runLow", m_runLow, "IOV:  run lowest", 0);
    addParam("runHigh", m_runHigh, "IOV:  run highest", -1);
    addParam("wr", m_wr, "writing to (W)/reading from (R) DB: Wagel, Ragel, WhapdQA, RhapdQA", string(""));
    addParam("InputFileNames", m_inputFileNames, "Input root files (HAPDs)", emptyvector);
    addParam("InputFileNames2", m_inputFileNames2, "Input root files (ASICs)", emptyvector);
    addParam("GlobalTag", m_globalTag, "Global tag for ASICs", string(""));
  }

  ARICHDatabaseModule::~ARICHDatabaseModule()
  {
  }

  void ARICHDatabaseModule::initialize()
  {
    m_content = GearDir("/AerogelData/Content");
    m_content2 = GearDir("/hapdData/Content");
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
      const auto name = DBStore::arrayName<ARICHAerogelInfo>(m_globalTag);
      int agel = 0;

//      auto expNo = evtAgel->getExperiment();
//      IntervalOfValidity iov(expNo, m_runLow, expNo, m_runHigh);
      IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

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

      DBArray<ARICHAerogelInfo> elements(m_globalTag);
      elements.getEntries();

      /*       // Print aerogel info

            unsigned int el = 0;
            for(const auto& element : elements) {
              B2INFO ("Element Number: " << el << "; version = " << element.getAerogelVersion() << ", serial = " << element.getAerogelSerial() << ", id = " << element.getAerogelID() << ", n = " << element.getAerogelRefractiveIndex() << ", transmLength = " << element.getAerogelTransmissionLength() << ", thickness = " << element.getAerogelThickness());
              for (int j = 0; j < 53; j++) {
                B2INFO ("lambda - transmittance = " << element.getAerogelTransmittanceLambda(j) << " - " << element.getAerogelTransmittance(j) << "; ");
              }
              el++;
            }*/

    }

    if (m_wr == "WhapdQA") { // writing hapd QA data to DB

      StoreObjPtr<EventMetaData> evtHapdQA;
      DataStore::Instance().setInitializeActive(true);
      evtHapdQA.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);

      TClonesArray hapdQAConstants("Belle2::ARICHHapdQA");
      const auto name = DBStore::arrayName<ARICHHapdQA>(m_globalTag);

//      auto expNo = evtAgel->getExperiment();
//      IntervalOfValidity iov(expNo, m_runLow, expNo, m_runHigh);
      IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

      int hapd = 0;

      for (const string& inputFile : m_inputFileNames) {
        TFile* f = TFile::Open(inputFile.c_str(), "READ");
        int size = inputFile.length();
        m_hapdSerial = inputFile.substr(size - 16, 6);
        TIter next(f->GetListOfKeys());
        TKey* key;
        while ((key = (TKey*)next())) {
          std::string strime = key->GetName();
          if (strime.find("gcurrent") == 0) {
            TGraph* hhist1 = (TGraph*)f->Get(strime.c_str());
            m_leakCurrent.push_back(hhist1);
          } else if (strime.find("h2dscan") == 0) {
            TH2F* hitData2D = (TH2F*)f->Get(strime.c_str());
            m_hitData2D = hitData2D;
          } else if (strime.find("gnoise_ch") == 0) {
            TGraph* hhist2 = (TGraph*)f->Get(strime.c_str());
            m_noise.push_back(hhist2);
          } else if (strime.find("hchscan") == 0) {
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
          } else { B2INFO("Key name does not match any of the following: gcurrent, 2dscan, gnoise, hchscan!"); }
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

      DBArray<ARICHHapdQA> elements(m_globalTag);
      elements.getEntries();

      /*       // Print serial numbers of HAPDs and save graphs (Leakage current) to root file

            unsigned int el = 0;
            for(const auto& element : elements) {
              B2INFO ("Element Number: " << el << "; serial = " << element.getHapdSerialNumber());
            el++;
            }*/

    }

    if (m_wr == "Wasic") { // writing hapd QA data to DB

      StoreObjPtr<EventMetaData> evtAsic;
      DataStore::Instance().setInitializeActive(true);
      evtAsic.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);

      TClonesArray asicConstants("Belle2::ARICHAsicInfo");
      const auto name = DBStore::arrayName<ARICHAsicInfo>(m_globalTag);

//      auto expNo = evtAgel->getExperiment();
//      IntervalOfValidity iov(expNo, m_runLow, expNo, m_runHigh);
      IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

      int asic = 0;
      for (const string& inputFile : m_inputFileNames2) {
        std::string inputFileNew = (std::string) inputFile;
        asicName = inputFileNew.substr(inputFileNew.find("/asicData") + 17);
        findRoot = asicName.find(".root");
        if (findRoot != std::string::npos) {
          asicSerial = asicName.erase(findRoot);
          TFile* f = TFile::Open(inputFile.c_str(), "READ");
          TIter next(f->GetListOfKeys());
          TKey* key;
          while ((key = (TKey*)next())) {
            std::string strname = key->GetName();
            std::string name1 = asicSerial + "_gain_g0";
            std::string name2 = asicSerial + "_gain_g1";
            std::string name3 = asicSerial + "_gain_g2";
            std::string name4 = asicSerial + "_gain_g3";
            std::string name5 = asicSerial + "_offset_c";
            std::string name6 = asicSerial + "_offset_f";
            if ((strname.find(name1) == 0) || (strname.find(name2) == 0) || (strname.find(name3) == 0) || (strname.find(name4) == 0)) { TH3F* hist3d = (TH3F*)f->Get(strname.c_str()); m_gain.push_back(hist3d);}
            else if ((strname.find(name5) == 0) || (strname.find(name6) == 0)) { TH3F* hist3d = (TH3F*)f->Get(strname.c_str()); m_offset.push_back(hist3d);}
            else { B2INFO("Key name does not match any of the following: gain, offset!"); }
          }
          for (const string& inputFile : m_inputFileNames2) {
            std::string inputFileNew = (std::string) inputFile;
            findTxt = inputFileNew.find("dateGain.txt");
            findTxt2 = inputFileNew.find("dateOffset.txt");
            if (findTxt != std::string::npos) {
              std::ifstream ifs(inputFile);
              if (ifs.is_open()) {
                while (ifs.good()) {
                  std::getline(ifs, line);
                  findSerial = line.find(asicSerial);
                  if (findSerial != std::string::npos) {
                    dateEnd = line.substr(line.find(":") + 2);
                    yearStr = dateEnd.substr(0, 4);
                    monthStr = dateEnd.substr(4, 2);
                    dayStr = dateEnd.substr(6, 2);
                    hourStr = dateEnd.substr(8, 2);
                    minStr = dateEnd.substr(10, 2);
                    year = atoi(yearStr.c_str());
                    month = atoi(monthStr.c_str());
                    day = atoi(dayStr.c_str());
                    hour = atoi(hourStr.c_str());
                    min = atoi(minStr.c_str());
                    timeFinishGain = TTimeStamp(year, month, day, hour, min, 0, 0, kTRUE, -9 * 60 * 60);
                  }
                }
              }
              ifs.clear();
              ifs.close();
            }
            if (findTxt2 != std::string::npos) {
              std::ifstream ifs(inputFile);
              if (ifs.is_open()) {
                while (ifs.good()) {
                  std::getline(ifs, line);
                  findSerial = line.find(asicSerial);
                  if (findSerial != std::string::npos) {
                    dateEnd = line.substr(line.find(":") + 2);
                    yearStr = dateEnd.substr(0, 4);
                    monthStr = dateEnd.substr(4, 2);
                    dayStr = dateEnd.substr(6, 2);
                    hourStr = dateEnd.substr(8, 2);
                    minStr = dateEnd.substr(10, 2);
                    year = atoi(yearStr.c_str());
                    month = atoi(monthStr.c_str());
                    day = atoi(dayStr.c_str());
                    hour = atoi(hourStr.c_str());
                    min = atoi(minStr.c_str());
                    timeFinishOffset = TTimeStamp(year, month, day, hour, min, 0, 0, kTRUE, -9 * 60 * 60);
                  }
                }
              }
              ifs.clear();
              ifs.close();
            }
          }
          new(asicConstants[asic]) ARICHAsicInfo(asicSerial, timeFinishGain, timeFinishOffset, m_gain, m_offset);
          m_gain.clear();
          m_offset.clear();
          asic++;
        }
      }
      Database::Instance().storeData(name, &asicConstants, iov);
    }

    if (m_wr == "Rasic") { // reading from DB
      DBArray<ARICHAsicInfo> elements(m_globalTag);
      elements.getEntries();
      unsigned int el = 0;
      for (const auto& element : elements) {
        B2INFO("Element Number: " << el << "; ID = " << element.getAsicID());
        el++;
      }
    }


    if (m_wr == "WhapdInfo") { // writing hapd chip data to DB

      StoreObjPtr<EventMetaData> evtHapdChipInfo;
      DataStore::Instance().setInitializeActive(true);
      evtHapdChipInfo.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);

      TClonesArray hapdChipInfoConstants("Belle2::ARICHHapdChipInfo");
      const auto nameChip = DBStore::arrayName<ARICHHapdChipInfo>(m_globalTag);

      int hapd = 0;
//      auto expNo = evtAgel->getExperiment();
//      IntervalOfValidity iov(expNo, m_runLow, expNo, m_runHigh);
      IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

      for (const auto& hapdInfo : m_content2.getNodes("hapd")) {
        m_serial = hapdInfo.getString("serial");
        new(hapdChipInfoConstants[hapd]) ARICHHapdChipInfo(m_serial);
        cout << m_serial << endl;
        hapd++;
      }
      Database::Instance().storeData(nameChip, &hapdChipInfoConstants, iov);
    }





  } // end event

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
