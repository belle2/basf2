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
#include <arich/dbobjects/structures/ARICHHapdChipInfo.h>
#include <arich/dbobjects/structures/ARICHHapdInfo.h>

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/DBArray.h>

#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TF1.h>
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

  for (const auto& element : elements) {
    B2INFO("Version = " << element.getAerogelVersion() << ", serial = " << element.getAerogelSerial() <<
           ", id = " << element.getAerogelID() << ", n = " << element.getAerogelRefractiveIndex() << ", transmLength = " <<
           element.getAerogelTransmissionLength() << ", thickness = " << element.getAerogelThickness())
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
        hitData2D->SetDirectory(0);
      }

      else if (strime.find("gnoise_ch") == 0) {
        TGraph* hhist2 = (TGraph*)f->Get(strime.c_str());
        noise.push_back(hhist2);
      }

      else if (strime.find("hchscan") == 0) {
        TH1F* hhist3 = (TH1F*)f->Get(strime.c_str());
        hhist3->SetDirectory(0);

        // conversion TH1F -> TH1S

        const char* hhist3_ime = hhist3->GetName();
        const char* hhist3_naslov = hhist3->GetTitle();
        int hhist3_nbins = hhist3->GetSize();
        TH1S* hhist3short = new TH1S(hhist3_ime, hhist3_naslov, hhist3_nbins - 2, 0, 1);
        for (int bin = 0; bin < hhist3_nbins; bin++) {
          hhist3short->SetBinContent(bin, hhist3->GetBinContent(bin));
        }
        hhist3short->SetDirectory(0);
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
      // fill vectors with histograms with different gain and offset settings
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
      f->Close();

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

  // find matching sn and dna
  for (const auto& febmap : content1.getNodes("feb")) {
    int serial = febmap.getInt("sn");
    string dna = febmap.getString("dna");

    // save data as an element of the array
    new(febConstants[feb])  ARICHFebTest();
    auto* febConst = static_cast<ARICHFebTest*>(febConstants[feb]);

    febConst->setFebSerial(serial);
    febConst->setFebDna(dna);

    // slow control data
    for (const auto& testFEB : content2.getNodes("febtest")) {
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

    // high voltage test data
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

    // low voltage test data
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

  // Print serial numbers of FEBs
  unsigned int el = 0;
  for (const auto& element : elements) {
    B2INFO("Element Number: " << el << "; serial = " << element.getFebSerial());
    el++;
  }
}

void ARICHDatabaseImporter::importHapdChipInfo()
{
  // define data array
  TClonesArray chipConstants("Belle2::ARICHHapdChipInfo");
  int hapd_i = 0, chip_i = 0;

  GearDir content = GearDir("/ArichData/AllData/hapdData/Content");

  string serial[1000], chip[4000];
  int channel_label_aval[4000], bias[4000], gain[4000];
  TGraph** bombardmentGain, **avalancheGain;
  bombardmentGain = new TGraph *[4000];
  avalancheGain = new TGraph *[4000];
  TH2F** bias2DV, **bias2DI;
  bias2DV = new TH2F *[4000];
  bias2DI = new TH2F *[4000];
  vector<int> deadlist[4000], cutlist[4000];
  vector<TGraph*> bombCurrents[4000], avalCurrents[4000];

  // extract chip info, such as bias voltage, lists of dead and bad channels etc.
  for (const auto& hapdInfo : content.getNodes("hapd")) {
    serial[hapd_i] = hapdInfo.getString("serial");
    int chip_ABCD = 0;
    for (const auto& chipInfo : hapdInfo.getNodes("chipinfo")) {
      chip_i = 4 * hapd_i + chip_ABCD;
      chip[chip_i] = chipInfo.getString("chip");
      bias[chip_i] = chipInfo.getInt("bias");
      string deadL = chipInfo.getString("deadlist");
      string cutL = chipInfo.getString("cutlist");
      if (deadL.find("ch") != string::npos) { string deadLsub = deadL.substr(3); cutlist[chip_i] = ARICHDatabaseImporter::channelsListHapd(deadLsub.c_str()); }
      if (cutL.find("ch") != string::npos) {  string cutLsub = cutL.substr(3); deadlist[chip_i] = ARICHDatabaseImporter::channelsListHapd(cutLsub.c_str()); }
      string gain_str = chipInfo.getString("gain");
      gain[chip_i] = atoi(gain_str.c_str());
      chip_ABCD++;
    }

    // prepare TGraphs for bombardment gain and current
    int n3 = 30, i3 = 0;
    //int channel_label_bomb;
    float hv_bomb[n3], gain_bomb[n3], current1_bomb[n3], current2_bomb[n3], current3_bomb[n3];
    chip_ABCD = 0;
    for (const auto& BG : hapdInfo.getNodes("bombardmentgain/ch")) {
      chip_i = 4 * hapd_i + chip_ABCD;
      string value = BG.getString("value");
      //string value_1 = value;
      string chip_label = value.erase(1);
      //string value_2 = value_1.substr(2);
      //channel_label_bomb = atoi(value_2.c_str());
      for (const auto& BG2 : BG.getNodes("point")) {
        hv_bomb[i3] = (float) BG2.getDouble("hv");
        gain_bomb[i3] = (float) BG2.getDouble("gain");
        current1_bomb[i3] = (float) BG2.getDouble("current1");
        current2_bomb[i3] = (float) BG2.getDouble("current2");
        current3_bomb[i3] = (float) BG2.getDouble("current3");
        i3++;
      }

      bombardmentGain[chip_i] = ARICHDatabaseImporter::getGraphGainCurrent("Bombardment", "gain", chip_label, i3, hv_bomb, gain_bomb);
      TGraph* bombardmentCurrent1 = ARICHDatabaseImporter::getGraphGainCurrent("Bombardment", "current1", chip_label, i3, hv_bomb,
                                    current1_bomb);
      TGraph* bombardmentCurrent2 = ARICHDatabaseImporter::getGraphGainCurrent("Bombardment", "current2", chip_label, i3, hv_bomb,
                                    current2_bomb);
      TGraph* bombardmentCurrent3 = ARICHDatabaseImporter::getGraphGainCurrent("Bombardment", "current3", chip_label, i3, hv_bomb,
                                    current3_bomb);
      bombCurrents[chip_i].push_back(bombardmentCurrent1);
      bombCurrents[chip_i].push_back(bombardmentCurrent2);
      bombCurrents[chip_i].push_back(bombardmentCurrent3);

      chip_ABCD++;
      i3 = 0;
    }

    // prepare TGraphs for avalanche gain and current
    int n4 = 30, i4 = 0;
    float hv_aval[n4], gain_aval[n4], current1_aval[n4], current2_aval[n4], current3_aval[n4];
    chip_ABCD = 0;
    for (const auto& BG : hapdInfo.getNodes("avalanchegain/ch")) {
      chip_i = 4 * hapd_i + chip_ABCD;
      string value = BG.getString("value");
      string value_1 = value;
      string chip_label = value.erase(1);
      string value_2 = value_1.substr(2);
      channel_label_aval[chip_i] = atoi(value_2.c_str());
      for (const auto& BG2 : BG.getNodes("point")) {
        hv_aval[i4] = (float) BG2.getDouble("biasv");
        gain_aval[i4] = (float) BG2.getDouble("gain");
        current1_aval[i4] = (float) BG2.getDouble("current1");
        current2_aval[i4] = (float) BG2.getDouble("current2");
        current3_aval[i4] = (float) BG2.getDouble("current3");
        i4++;
      }

      avalancheGain[chip_i] = ARICHDatabaseImporter::getGraphGainCurrent("Avalanche", "gain", chip_label, i4, hv_aval, gain_aval);
      TGraph* avalancheCurrent1 = ARICHDatabaseImporter::getGraphGainCurrent("Avalanche", "current1", chip_label, i4, hv_aval,
                                  current1_aval);
      TGraph* avalancheCurrent2 = ARICHDatabaseImporter::getGraphGainCurrent("Avalanche", "current2", chip_label, i4, hv_aval,
                                  current2_aval);
      TGraph* avalancheCurrent3 = ARICHDatabaseImporter::getGraphGainCurrent("Avalanche", "current3", chip_label, i4, hv_aval,
                                  current3_aval);
      avalCurrents[chip_i].push_back(avalancheCurrent1);
      avalCurrents[chip_i].push_back(avalancheCurrent2);
      avalCurrents[chip_i].push_back(avalancheCurrent3);

      chip_ABCD++;
      i4 = 0;
    }

    chip_ABCD = 0;
    // prepare 2D histograms for bias voltage and current
    int n5 = 150, i5 = 0, chipnum[n5];
    float biasv[n5], biasi[n5];
    for (const auto& HI : hapdInfo.getNodes("bias2d/biasvalue")) {
      std::string chip_2d = HI.getString("@chip");
      chipnum[i5] = HI.getInt("@ch");
      biasv[i5] = (float) HI.getDouble("biasv");
      biasi[i5] = (float) HI.getDouble("biasi");
      if (chipnum[i5] == 36) {
        chip_i = 4 * hapd_i + chip_ABCD;
        bias2DV[chip_i] = ARICHDatabaseImporter::getBiasGraph(chip_2d, "voltage", chipnum, biasv);
        bias2DI[chip_i] = ARICHDatabaseImporter::getBiasGraph(chip_2d, "current", chipnum, biasi);
        i5 = -1;
        chip_ABCD++;
      }
      i5++;
    }

    hapd_i++;
  }

  // fill data to ARICHHapdChipInfo class
  for (int l = 0; l < chip_i + 1; l++) {
    new(chipConstants[l])  ARICHHapdChipInfo();
    auto* chipConst = static_cast<ARICHHapdChipInfo*>(chipConstants[l]);

    chipConst->setHapdSerial(serial[l / 4]);
    chipConst->setChipLabel(chip[l]);
    chipConst->setBiasVoltage(bias[l]);
    chipConst->setGain(gain[l]);
    chipConst->setDeadChannel(deadlist[l]);
    chipConst->setBadChannel(cutlist[l]);
    chipConst->setBombardmentGain(bombardmentGain[l]);
    chipConst->setBombardmentCurrent(bombCurrents[l]);
    chipConst->setAvalancheGain(avalancheGain[l]);
    chipConst->setAvalancheCurrent(avalCurrents[l]);
    chipConst->setChannelNumber(channel_label_aval[l]);
    chipConst->setBiasVoltage2D(bias2DV[l]);
    chipConst->setBiasCurrent2D(bias2DI[l]);
  }


  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1);
  Database::Instance().storeData("testHapdChip", &chipConstants, iov);
}

void ARICHDatabaseImporter::importHapdInfo()
{
  // define data array
  TClonesArray hapdConstants("Belle2::ARICHHapdInfo");
  int hapd_i = 0;

  GearDir content = GearDir("/ArichData/AllData/hapdData/Content");

  string serial[1000];
  float qe400[1000], hv[1000], current[1000];
  int guardbias[1000];
  TGraph** qe, **adc;
  qe = new TGraph *[1000];
  adc = new TGraph *[1000];

  for (const auto& hapdInfo : content.getNodes("hapd")) {
    serial[hapd_i] = hapdInfo.getString("serial");
    qe400[hapd_i] = (float) hapdInfo.getDouble("qe400");
    hv[hapd_i] = 1000 * (float) hapdInfo.getDouble("hv");
    current[hapd_i] = (float) hapdInfo.getDouble("current");
    string gb = hapdInfo.getString("guardbias");
    guardbias[hapd_i] = atoi(gb.c_str());

    // prepare TGraph of quantum efficiency as function of lambda
    int n1 = 70;
    float lambda[n1], qepoint[n1];
    int i1 = 0;
    for (const auto& QE : hapdInfo.getNodes("qe/qepoint")) {
      lambda[i1] = (float) QE.getInt("@lambda");
      qepoint[i1] = (float) QE.getDouble(".");
      i1++;
    }

    qe[hapd_i] = new TGraph(i1, lambda, qepoint);
    qe[hapd_i]->SetName("qe");
    qe[hapd_i]->SetTitle("qe");
    qe[hapd_i]->GetXaxis()->SetTitle("lambda");
    qe[hapd_i]->GetYaxis()->SetTitle("qe");


    // prepare TGraph of pulse height distribution
    int n2 = 4100;
    int channel_adc[n2], pulse_adc[n2];
    int i2 = 0;
    for (const auto& ADC : hapdInfo.getNodes("adc/value")) {
      channel_adc[i2] = ADC.getInt("@ch");
      string str = ADC.getString(".");
      pulse_adc[i2] = atoi(str.c_str());
      i2++;
    }

    adc[hapd_i] = new TGraph(i2, channel_adc, pulse_adc);
    adc[hapd_i]->SetName("adc");
    adc[hapd_i]->SetTitle("Pulse Height Distribution");
    adc[hapd_i]->GetXaxis()->SetTitle("channel");
    adc[hapd_i]->GetYaxis()->SetTitle("pulse height");


    hapd_i++;
  }

  // fill data to ARICHHapdInfo class
  for (int m = 0; m < hapd_i; m++) {
    new(hapdConstants[m]) ARICHHapdInfo();
    auto* hapdConst = static_cast<ARICHHapdInfo*>(hapdConstants[m]);
    hapdConst->setSerialNumber(serial[m]);
    hapdConst->setQuantumEfficiency400(qe400[m]);
    hapdConst->setHighVoltage(hv[m]);
    hapdConst->setGuardBias(guardbias[m]);
    hapdConst->setCurrent(current[m]);
    hapdConst->setQuantumEfficiency(qe[m]);
    hapdConst->setPulseHeightDistribution(adc[m]);
    //  for(unsigned int k = 0; k < 4; k++)  {hapdConst[hapd_i].setHapdChipInfo(k, chipConstants[hapd_i+k]); }
  }


  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1);
  Database::Instance().storeData("testHapd", &hapdConstants, iov);
}

std::vector<int> ARICHDatabaseImporter::channelsListHapd(std::string chlist)
{
  vector<int> CHs;

  // parse string to get numbers of all bad channels
  if (chlist.find(",") != string::npos) {
    while (chlist.find(",") != string::npos) {
      string CH = chlist.substr(0, chlist.find(","));
      CHs.push_back(atoi(CH.c_str()));
      chlist = chlist.substr(chlist.find(",") + 1);
    }
    CHs.push_back(atoi(chlist.c_str()));
  } else {
    CHs.push_back(atoi(chlist.c_str()));
  }
  return CHs;
}

TGraph* ARICHDatabaseImporter::getGraphGainCurrent(std::string bomb_aval, std::string g_i, std::string chip_label, int i, float* HV,
                                                   float* gain_current)
{
  TGraph* hapd_graph = new TGraph(i, HV, gain_current);
  string title = bomb_aval + " " + g_i + ", chip " + chip_label;
  hapd_graph->SetTitle(title.c_str());
  hapd_graph->GetXaxis()->SetTitle("high voltage");
  hapd_graph->GetYaxis()->SetTitle(g_i.c_str());
  return hapd_graph;
}

int ARICHDatabaseImporter::getChannelPosition(std::string XY, std::string chip_2d, int chipnum)
{
  // use correct mapping
  int x = 100, y = 100;
  if (chip_2d == "A") {
    y = 12;
    x = 0 + chipnum;
    while (x > 6) {
      x = x - 6;
      y = y - 1;
    }
  } else if (chip_2d == "B") {
    x = 12;
    y = 13 - chipnum;
    while (y < 7) {
      y = y + 6;
      x = x - 1;
    }
  } else if (chip_2d == "C") {
    y = 1;
    x = 13 - chipnum;
    while (x < 7) {
      x = x + 6;
      y = y + 1;
    }
  } else if (chip_2d == "D") {
    x = 1;
    y = 0 + chipnum;
    while (y > 6) {
      y = y - 6;
      x = x + 1;
    }
  }

  if (XY == "x") { return x;}
  else if (XY == "y") { return y;}
  else {return 100;}
}

TH2F* ARICHDatabaseImporter::getBiasGraph(std::string chip_2d, std::string voltage_current, int* chipnum, float* bias_v_i)
{
  string name = "bias " + voltage_current + ", chip " + chip_2d;
  TH2F* bias2d = new TH2F("bias2d", name.c_str(), 6, 0, 6, 6, 0, 6);
  for (int XYname = 0; XYname < 6; XYname++) {
    bias2d->GetXaxis()->SetBinLabel(XYname + 1, to_string(XYname).c_str());
    bias2d->GetYaxis()->SetBinLabel(XYname + 1, to_string(XYname).c_str());
  }
  for (int XY = 0; XY < 36; XY++)  {
    int x = 1;
    int y = 0 + chipnum[XY];
    while (y > 6) {
      y = y - 6;
      x = x + 1;
    }
    bias2d->SetBinContent(x, y, bias_v_i[XY]);
  }
  bias2d->SetDirectory(0);
  return bias2d;
}

void ARICHDatabaseImporter::exportHapdInfo()
{
  DBArray<ARICHHapdInfo> elements("testHapd");
  elements.getEntries();

  for (const auto& element : elements) {
    B2INFO("Serial = " << element.getSerialNumber() << "; HV = " << element.getHighVoltage() << "; qe400 = " <<
           element.getQuantumEfficiency400());
  }
}

void ARICHDatabaseImporter::exportHapdChipInfo()
{
  DBArray<ARICHHapdChipInfo> elements("testHapdChip");
  elements.getEntries();

  for (const auto& element : elements) {
    B2INFO("Serial = " << element.getHapdSerial() << "; chip = " << element.getChipLabel() << "; bias voltage = " <<
           element.getBiasVoltage());
  }
}


void ARICHDatabaseImporter::getBiasVoltagesForHapdChip(std::string serialNumber)
{
  // example that shows how to extract and use data
  // it calculates bias voltage at gain = 40 for each chip

  DBArray<ARICHHapdChipInfo> elements("testHapdChip");
  elements.getEntries();

  for (const auto& element : elements) {
    if (element.getHapdSerial() == serialNumber) {
      TGraph* avalgain = element.getAvalancheGain();

      // use linear interpolation to get bias voltage at gain = 40
      /*
            // 1) you can do it by hand

            double A, B, C, D;
            for(int j = 0; j < 100; j++) {
              avalgain->GetPoint(j, A, B);
              if(B>40) {
                avalgain->GetPoint(j-1, A, B);
                avalgain->GetPoint(j, C, D);
                float k = (B-D)/(A-C);
                float n = B - k*A;
                float xgain = (40 - n)/k;
                B2INFO("serial#-chip = " << element.getHapdSerial() << "-" << element.getChipLabel() << "; " << "V(gain=40) = " << (int)(xgain+0.5));
                j = 100;
              }
            }
      */

      // 2) use "Eval" function
      // - avalgain graph is gain(voltage)
      // - function "Eval" can be used to interpolate around chosen x
      // - convert graph gain(voltage) to voltage(gain) to interpolate around gain = 40
      TGraph* gainnew = new TGraph(avalgain->GetN());
      double xpoint, ypoint;
      for (int j = 0; j < avalgain->GetN(); j++) {
        avalgain->GetPoint(j, xpoint, ypoint);
        gainnew->SetPoint(j, ypoint, xpoint);
      }
      B2INFO("serial#-chip = " << element.getHapdSerial() << "-" << element.getChipLabel() << "; " << "V(gain=40) = " << (int)(
               gainnew->Eval(40) + 0.5));

    }
  }
}

