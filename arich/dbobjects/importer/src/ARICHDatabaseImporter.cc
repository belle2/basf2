/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar, Thomas Kuhr                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dbobjects/importer/ARICHDatabaseImporter.h>
#include <arich/dbobjects/structures/ARICHAerogelInfo.h>
#include <arich/dbobjects/structures/ARICHAsicInfo.h>
#include <arich/dbobjects/structures/ARICHHapdQA.h>
#include <arich/dbobjects/structures/ARICHFebTest.h>

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/DBArray.h>

#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TGraph.h>
#include <TFile.h>
#include <TKey.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <TClonesArray.h>


using namespace std;
using namespace Belle2;


ARICHDatabaseImporter::ARICHDatabaseImporter(vector<string> inputFilesHapdQA, vector<string> inputFilesAsicRoot,
                                             vector<string> inputFilesAsicTxt)
{
  for (unsigned int i = 0; i < inputFilesHapdQA.size(); i++) {  m_inputFilesHapdQA.push_back(inputFilesHapdQA[i]); }
  for (unsigned int i = 0; i < inputFilesAsicRoot.size(); i++) {  m_inputFilesAsicRoot.push_back(inputFilesAsicRoot[i]); }
  for (unsigned int i = 0; i < inputFilesAsicTxt.size(); i++) {  m_inputFilesAsicTxt.push_back(inputFilesAsicTxt[i]); }
}


void ARICHDatabaseImporter::importAerogelInfo()
{
  GearDir content = GearDir("/ArichData/AllData/AerogelData/Content");

  // define data array
  TClonesArray agelConstants("Belle2::ARICHAerogelInfo");
  int agel = 0;

  // loop over xml files and extract the data
  for (const auto& aerogel : content.getNodes("aerogel")) {
    float version = (float) aerogel.getDouble("version");
    string serial = aerogel.getString("serial");
    string id = aerogel.getString("id");
    float index = (float) aerogel.getDouble("index");
    float trlen = (float) aerogel.getDouble("translength");
    float thickness = (float) aerogel.getDouble("thick");
    vector<int> lambdas;
    vector<float> transmittances;
    for (const auto& transmittance : aerogel.getNodes("transmittance/transpoint")) {
      int lambda = transmittance.getInt("@lambda");
      float val = (float) transmittance.getDouble(".");
      lambdas.push_back(lambda);
      transmittances.push_back(val);
    }

    // save data as an element of the array
    new(agelConstants[agel]) ARICHAerogelInfo(version, serial, id, index, trlen, thickness, lambdas, transmittances);
    agel++;
  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

  // store under default name:
  //Database::Instance().storeData(&agelConstants, iov);

  // store under user defined name:
  Database::Instance().storeData("testAerogel", &agelConstants, iov);
}


void ARICHDatabaseImporter::exportAerogelInfo()
{

  DBArray<ARICHAerogelInfo> elements("testAerogel");
  elements.getEntries();

  // Print aerogel info

  unsigned int el = 0;
  for (const auto& element : elements) {
    cout << "Element Number: " << el << "; version = " << element.getAerogelVersion() << ", serial = " << element.getAerogelSerial() <<
         ", id = " << element.getAerogelID() << ", n = " << element.getAerogelRefractiveIndex() << ", transmLength = " <<
         element.getAerogelTransmissionLength() << ", thickness = " << element.getAerogelThickness() << endl;
    el++;
  }
}


void ARICHDatabaseImporter::importHapdQA()
{
  // define data array
  TClonesArray hapdQAConstants("Belle2::ARICHHapdQA");
  int hapd = 0;

  // loop over root riles
  for (const string& inputFile : m_inputFilesHapdQA) {

    TFile* f = TFile::Open(inputFile.c_str(), "READ");

    int size = inputFile.length();
    string hapdSerial = inputFile.substr(size - 16, 6);

    vector<TGraph*> leakCurrent;
    TH2F* hitData2D = 0;
    vector<TGraph*> noise;
    vector<TH1S*> hitCount;

    // extract data
    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {

      string strime = key->GetName();

      if (strime.find("gcurrent") == 0) {
        TGraph* hhist1 = (TGraph*)f->Get(strime.c_str());
        leakCurrent.push_back(hhist1);
      }

      else if (strime.find("h2dscan") == 0) {
        hitData2D = (TH2F*)f->Get(strime.c_str());
      }

      else if (strime.find("gnoise_ch") == 0) {
        TGraph* hhist2 = (TGraph*)f->Get(strime.c_str());
        noise.push_back(hhist2);
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
        hitCount.push_back(hhist3short);

      }

      else { B2INFO("Key name does not match any of the following: gcurrent, 2dscan, gnoise, hchscan!"); }
    }

    TTimeStamp measurementDate = {};

    // save data as an element of the array
    new(hapdQAConstants[hapd]) ARICHHapdQA(hapdSerial, measurementDate, leakCurrent, hitData2D, noise, hitCount);
    hapd++;
  }

  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData("testHapd", &hapdQAConstants, iov);
}


void ARICHDatabaseImporter::exportHapdQA()
{
  DBArray<ARICHHapdQA> elements("testHapd");
  elements.getEntries();

  // Print serial numbers of HAPDs
  unsigned int el = 0;
  for (const auto& element : elements) {
    B2INFO("Element Number: " << el << "; serial = " << element.getHapdSerialNumber());
    el++;
  }
}


void ARICHDatabaseImporter::importAsicInfo()
{
  GearDir content = GearDir("/ArichData/AllData/asicList");

  // define data array
  TClonesArray asicConstants("Belle2::ARICHAsicInfo");
  int asic = 0;

  // loop over root files
  for (const string& inputFile : m_inputFilesAsicRoot) {
    vector<TH3F*> m_gain;
    vector<TH3F*> m_offset;
    TTimeStamp timeFinishGain;
    TTimeStamp timeFinishOffset;

    string inputFileNew = (string) inputFile;
    string asicName = inputFileNew.substr(inputFileNew.find("/asicData") + 17);
    size_t findRoot = asicName.find(".root");
    if (findRoot != string::npos) {
      string asicSerial = asicName.erase(findRoot);
      // extract the data from files
      TFile* f = TFile::Open(inputFile.c_str(), "READ");
      TIter next(f->GetListOfKeys());
      TKey* key;
      while ((key = (TKey*)next())) {
        string strname = key->GetName();
        if (strname.find("gain") != string::npos) {
          TH3F* hist3d = (TH3F*)f->Get(strname.c_str());
          hist3d->SetDirectory(0);
          m_gain.push_back(hist3d);
        } else if (strname.find("offset") != string::npos) {
          TH3F* hist3d = (TH3F*)f->Get(strname.c_str());
          hist3d->SetDirectory(0);
          m_offset.push_back(hist3d);
        } else { B2INFO("Key name does not match any of the following: gain, offset!"); }
      }

      // get time of measurement
      timeFinishGain = ARICHDatabaseImporter::getAsicDate(asicSerial, "gain");
      timeFinishOffset = ARICHDatabaseImporter::getAsicDate(asicSerial, "offset");

      // get lists of bad channels
      vector<int> nosignalCHs = ARICHDatabaseImporter::channelsList(asicSerial, "nosignal");
      vector<int> badconnCHs = ARICHDatabaseImporter::channelsList(asicSerial, "badconn");
      vector<int> badoffsetCHs = ARICHDatabaseImporter::channelsList(asicSerial, "badoffset");
      vector<int> badlinCHs = ARICHDatabaseImporter::channelsList(asicSerial, "badlin");

      int num = 0;
      string numCH = "";
      for (const auto& asicInfo : content.getNodes("asicinfo")) {
        string serial = asicInfo.getString("serial");
        if (asicSerial == serial)  {
          // get the number of problematic channels
          numCH = asicInfo.getString("num");
        }
      }
      if (numCH.find("many") != string::npos) {num = 5000; }
      else if (numCH.find("all") != string::npos) {num = 10000; }
      else {num = atoi(numCH.c_str()); }

      // save data as an element of the array
      new(asicConstants[asic])  ARICHAsicInfo(asicSerial, timeFinishGain, timeFinishOffset, nosignalCHs, badconnCHs, badoffsetCHs,
                                              badlinCHs, num,  m_gain, m_offset);
      asic++;
      f->Close();
    }
  }
  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1);
  Database::Instance().storeData("testAsic", &asicConstants, iov);
}

TTimeStamp ARICHDatabaseImporter::getAsicDate(string asicSerial, string type)
{
  TTimeStamp timeFinish;
  string line;
  size_t findText = 100;

  // extract measurement date for the chosen serial number
  for (const string& inputFile : m_inputFilesAsicTxt) {
    string inputFileNew = (string) inputFile;
    if (type == "gain") { findText = inputFileNew.find("dateGain.txt"); }
    else if (type == "offset") { findText = inputFileNew.find("dateOffset.txt"); }
    else {B2INFO("Check type of measurement!"); }
    if (findText != string::npos) {
      std::ifstream ifs(inputFile);
      if (ifs.is_open()) {
        while (ifs.good()) {
          std::getline(ifs, line);
          string asicSerial2 = asicSerial + ":";
          size_t findSerial = line.find(asicSerial2);
          if (findSerial != string::npos) {timeFinish = ARICHDatabaseImporter::timedate(line); }
        }
      }
      ifs.clear();
      ifs.close();
    }
  }
  return timeFinish;
}

TTimeStamp ARICHDatabaseImporter::timedate(std::string enddate)
{
  // convert string into TTimeStamp
  string dateEnd = enddate.substr(enddate.find(":") + 2);
  string yearStr = dateEnd.substr(0, 4);
  string monthStr = dateEnd.substr(4, 2);
  string dayStr = dateEnd.substr(6, 2);
  string hourStr = dateEnd.substr(8, 2);
  string minStr = dateEnd.substr(10, 2);
  int year = atoi(yearStr.c_str());
  int month = atoi(monthStr.c_str());
  int day = atoi(dayStr.c_str());
  int hour = atoi(hourStr.c_str());
  int min = atoi(minStr.c_str());
  TTimeStamp datum = TTimeStamp(year, month, day, hour, min, 0, 0, kTRUE, -9 * 60 * 60);
  return datum;
}

vector<int> ARICHDatabaseImporter::channelsList(string asicSerial, string type)
{

  GearDir content = GearDir("/ArichData/AllData/asicList");
  vector<int> CHs;

  for (const auto& asicInfo : content.getNodes("asicinfo")) {
    string serial = asicInfo.getString("serial");
    // find data for the right serial number
    if (asicSerial == serial)  {
      string badCH = asicInfo.getString(type.c_str());
      // parse string to get numbers of all bad channels
      if (badCH.find(",") != string::npos) {
        while (badCH.find(",") != string::npos) {
          string CH = badCH.substr(0, badCH.find(","));
          CHs.push_back(atoi(CH.c_str()));
          badCH = badCH.substr(badCH.find(",") + 1);
        }
        CHs.push_back(atoi(badCH.c_str()));
      }
      // store 5000 if there are many bad channels
      else if (badCH.find("many") != string::npos) {
        CHs.push_back(5000);
      }
      // store 10000 if all channels are bad
      else if (badCH.find("all") != string::npos) {
        CHs.push_back(10000);
      } else {
        CHs.push_back(atoi(badCH.c_str()));
      }
    }
  }
  return CHs;
}


void ARICHDatabaseImporter::exportAsicInfo()
{
  DBArray<ARICHAsicInfo> elements("testAsic");
  elements.getEntries();

  // Print serial numbers of ASICs
  unsigned int el = 0;
  for (const auto& element : elements) {
    B2INFO("Element Number: " << el << "; serial = " << element.getAsicID());
    el++;
  }
}



void ARICHDatabaseImporter::importFebTest()
{
  // define data array
  TClonesArray febConstants("Belle2::ARICHFebTest");
  int feb = 0;

  GearDir content1 = GearDir("/ArichData/AllData/febmapping");
  GearDir content2 = GearDir("/ArichData/AllData/FEBData/Content");
  GearDir content2HV = GearDir("/ArichData/AllData/FEBDataHV/Content");
  GearDir content2LV = GearDir("/ArichData/AllData/FEBDataLV/Content");

  for (const auto& febmap : content1.getNodes("feb")) {
    int serial = febmap.getInt("sn");
    string dna = febmap.getString("dna");

    // save data as an element of the array
    new(febConstants[feb])  ARICHFebTest();
    auto* febConst = static_cast<ARICHFebTest*>(febConstants[feb]);

    febConst->setFebSerial(serial);
    febConst->setFebDna(dna);

    for (const auto& testFEB : content2.getNodes("febtest")) { // slow control data
      string timeSlowC = testFEB.getString("time");

      int i = 0;
      int positionOld = 0;

      for (const auto& testFEBslowctest : testFEB.getNodes("slowc")) {
        int position = testFEBslowctest.getInt("id");
        if (position == positionOld) {i++; }
        else {i = 1; }
        if (i == 10) {
          string dnaNew = testFEBslowctest.getString("dna");
          if (dnaNew == dna) {
            float tmon0 = (float) testFEBslowctest.getDouble("TMON0");
            float tmon1 = (float) testFEBslowctest.getDouble("TMON1");
            float vdd = (float) testFEBslowctest.getDouble("VDD");
            float v2p = (float) testFEBslowctest.getDouble("V2P");
            float v2n = (float) testFEBslowctest.getDouble("V2N");
            float vss = (float) testFEBslowctest.getDouble("VSS");
            float vth1 = (float) testFEBslowctest.getDouble("VTH1");
            float vth2 = (float) testFEBslowctest.getDouble("VTH2");
            float vcc12 = (float) testFEBslowctest.getDouble("VCC12");
            float vcc15 = (float) testFEBslowctest.getDouble("VCC15");
            float vcc25 = (float) testFEBslowctest.getDouble("VCC25");
            float v38p = (float) testFEBslowctest.getDouble("V38P");

            febConst->setTemperature0(tmon0);
            febConst->setTemperature1(tmon1);
            febConst->setVdd(vdd);
            febConst->setV2p(v2p);
            febConst->setV2n(v2n);
            febConst->setVss(vss);
            febConst->setVth1(vth1);
            febConst->setVth2(vth2);
            febConst->setVcc12(vcc12);
            febConst->setVcc15(vcc15);
            febConst->setVcc25(vcc25);
            febConst->setV38p(v38p);
            febConst->setTimeSlowC(ARICHDatabaseImporter::timedate2(timeSlowC));
          }
        }
        positionOld = position;
      }
    }

    for (const auto& testFEBhv : content2HV.getNodes("febtest")) {
      string timeHV = testFEBhv.getString("time");
      for (const auto& testFEBhv_sn : testFEBhv.getNodes("hvtest/feb")) {
        int serial_hv = testFEBhv_sn.getInt("sn");
        if (serial_hv == serial) {
          for (const auto& testFEBhv_i : testFEBhv_sn.getNodes("febhv/n")) {
            int n_id = testFEBhv_i.getInt("@id");
            if (n_id == 9) {
              float currentV99p = (float) testFEBhv_i.getDouble("n1470/I");
              febConst->setTimeHV(ARICHDatabaseImporter::timedate2(timeHV));
              febConst->setCurrentV99p(currentV99p);
            }
          }
        }
      }
    }


    for (const auto& testFEBlv : content2LV.getNodes("febtest")) {
      string timeLV = testFEBlv.getString("time");
      for (const auto& testFEBlv_sn : testFEBlv.getNodes("lvtest/feb")) {
        int serial_lv = testFEBlv_sn.getInt("sn");
        if (serial_lv == serial) {
          for (const auto& testFEBlv_i : testFEBlv_sn.getNodes("febps/n")) {
            int n_id = testFEBlv_i.getInt("@id");
            if (n_id == 14) {
              float currentV20p = 0.0, currentV21n = 0.0, currentV38p = 0.0;
              for (const auto& testFEBlv_pw : testFEBlv_i.getNodes("pw18")) {
                if (testFEBlv_pw.getInt("@id") == 0) {  currentV20p = (float) testFEBlv_pw.getDouble("I");  }
                if (testFEBlv_pw.getInt("@id") == 1) {  currentV21n = (float) testFEBlv_pw.getDouble("I");  }
                if (testFEBlv_pw.getInt("@id") == 2) {  currentV38p = (float) testFEBlv_pw.getDouble("I");  }
              }
              febConst->setTimeLV(ARICHDatabaseImporter::timedate2(timeLV));
              febConst->setCurrentV20p(currentV20p);
              febConst->setCurrentV21n(currentV21n);
              febConst->setCurrentV38p(currentV38p);
            }
          }
        }
      }
    }

    feb++;
  }

  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1);
  Database::Instance().storeData("testFeb", &febConstants, iov);
}


TTimeStamp ARICHDatabaseImporter::timedate2(std::string time)
{
  // convert string into TTimeStamp
  string dateMeas = time.substr(4);
  string yearStr = dateMeas.substr(16, 4);
  string monthStr = dateMeas.substr(0, 3);
  string dayStr = dateMeas.substr(4, 2);
  string hourStr = dateMeas.substr(7, 2);
  string minStr = dateMeas.substr(10, 2);
  string secStr = dateMeas.substr(13, 2);
  int year = atoi(yearStr.c_str());
  int day = atoi(dayStr.c_str());
  int hour = atoi(hourStr.c_str());
  int min = atoi(minStr.c_str());
  int sec = atoi(secStr.c_str());
  map<string, int> months  {
    { "Jan", 1 },  { "Feb", 2 },  { "Mar", 3 },  { "Apr", 4 },  { "May", 5 },  { "Jun", 6 },  { "Jul", 7 },  { "Aug", 8 },  { "Sep", 9 },  { "Oct", 10 },  { "Nov", 11 },  { "Dec", 12 }
  };
  int month = months.find(monthStr)->second;

  TTimeStamp datum = TTimeStamp(year, month, day, hour, min, sec, 0, kTRUE, 0);
  return datum;
}

void ARICHDatabaseImporter::exportFebTest()
{
  DBArray<ARICHFebTest> elements("testFeb");
  elements.getEntries();

  // Print serial numbers of HAPDs
  unsigned int el = 0;
  for (const auto& element : elements) {
    B2INFO("Element Number: " << el << "; serial = " << element.getFebSerial());
    el++;
    cout << "element" << endl;
  }
}
