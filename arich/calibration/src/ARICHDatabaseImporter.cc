/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar, Thomas Kuhr                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/calibration/ARICHDatabaseImporter.h>
#include <arich/dbobjects/ARICHAerogelInfo.h>
#include <arich/dbobjects/ARICHAsicInfo.h>
#include <arich/dbobjects/ARICHHapdQA.h>
#include <arich/dbobjects/ARICHFebTest.h>
#include <arich/dbobjects/ARICHHapdChipInfo.h>
#include <arich/dbobjects/ARICHHapdInfo.h>
#include <arich/dbobjects/ARICHHapdQE.h>
#include <arich/dbobjects/ARICHBadChannels.h>
#include <arich/dbobjects/ARICHSensorModuleInfo.h>
#include <arich/dbobjects/ARICHSensorModuleMap.h>
#include <arich/dbobjects/ARICHModuleTest.h>

// database classes used by simulation/reconstruction software
#include <arich/dbobjects/ARICHChannelMask.h>
#include <arich/dbobjects/ARICHChannelMapping.h>
#include <arich/dbobjects/ARICHModulesInfo.h>
#include <arich/dbobjects/ARICHMergerMapping.h>
#include <arich/dbobjects/ARICHCopperMapping.h>
#include <arich/dbobjects/ARICHSimulationPar.h>
#include <arich/dbobjects/ARICHGeometryConfig.h>

// channel histogram
#include <arich/utility/ARICHChannelHist.h>


#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/database/EventDependency.h>

#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/DBStore.h>
#include <framework/database/LocalDatabase.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>

#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TF1.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TKey.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <TClonesArray.h>
#include <TTree.h>
#include <tuple>

using namespace std;
using namespace Belle2;


ARICHDatabaseImporter::ARICHDatabaseImporter(vector<string> inputFilesHapdQA, vector<string> inputFilesAsicRoot,
                                             vector<string> inputFilesAsicTxt, vector<string> inputFilesHapdQE)
{
  for (unsigned int i = 0; i < inputFilesHapdQA.size(); i++) {  m_inputFilesHapdQA.push_back(inputFilesHapdQA[i]); }
  for (unsigned int i = 0; i < inputFilesAsicRoot.size(); i++) {  m_inputFilesAsicRoot.push_back(inputFilesAsicRoot[i]); }
  for (unsigned int i = 0; i < inputFilesAsicTxt.size(); i++) {  m_inputFilesAsicTxt.push_back(inputFilesAsicTxt[i]); }
  for (unsigned int i = 0; i < inputFilesHapdQE.size(); i++) {  m_inputFilesHapdQE.push_back(inputFilesHapdQE[i]); }
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

  // define interval of validity
//  IntervalOfValidity iov(0, 0, 1, 99);
  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

  // store under default name:
  // Database::Instance().storeData(&agelConstants, iov);
  // store under user defined name:
  Database::Instance().storeData("ARICHAerogelInfo", &agelConstants, iov);
}

void ARICHDatabaseImporter::exportAerogelInfo()
{

  // Definition:
  // This function extracts data from the database for chosen event.

  /*
    // Extract data from payload with bounded validity

    // Define event, run and experiment numbers
    EventMetaData event = EventMetaData(1200,4,0); // (event, run, exp)

    // Extract object and IOV from database
    std::pair<TObject*, IntervalOfValidity> podatki = Database::Instance().getData(event, "dbstore", "ARICHAerogelInfo");

    // print interval of validity
  //  IntervalOfValidity iov = std::get<1>(podatki);
  //  B2INFO("iov = " << iov);

    // Convert between different class types and extract TClonesArray
    // for chosen event
    TObject* data;
    data = std::get<0>(podatki);
    TClonesArray* elements = static_cast<TClonesArray*>(data);

    // Get entries from TClonesArray and print aerogel info
    (*elements).GetEntries();
    for (int i = 0; i < elements->GetSize(); i++) {
      ARICHAerogelInfo* myelement = (ARICHAerogelInfo*)elements->At(i);
      B2INFO("Version = " << myelement->getAerogelVersion() << ", SN = " << myelement->getAerogelSerial() << ", n = " << myelement->getAerogelRefractiveIndex() << ", trl = " << myelement->getAerogelTransmissionLength() << ", thickness = " << myelement->getAerogelThickness());
    }
  */



  // Extract data from payload with unbounded validity
  DBArray<ARICHAerogelInfo> elements("ARICHAerogelInfo");
  elements.getEntries();

  // Print aerogel info

  for (const auto& element : elements) {
    B2INFO("Version = " << element.getAerogelVersion() << ", serial = " << element.getAerogelSerial() <<
           ", id = " << element.getAerogelID() << ", n = " << element.getAerogelRefractiveIndex() << ", transmLength = " <<
           element.getAerogelTransmissionLength() << ", thickness = " << element.getAerogelThickness());
  }

}



void ARICHDatabaseImporter::importAerogelInfoEventDep()
{
  GearDir content = GearDir("/ArichData/AllData/AerogelData/Content");

  // define data arrays
  TClonesArray agelConstantsA("Belle2::ARICHAerogelInfo");
  TClonesArray agelConstantsB("Belle2::ARICHAerogelInfo");
  TClonesArray agelConstantsC("Belle2::ARICHAerogelInfo");


  for (int someint = 0; someint < 3; someint++) {
    int agel = 0;
    // loop over xml files and extract the data
    for (const auto& aerogel : content.getNodes("aerogel")) {
      // different version is made up - only used to check performance
      // of intrarun dependat function
      float version = 0;
      if (someint == 0)     version = (float) aerogel.getDouble("version");
      if (someint == 1)     version = 4.0;
      if (someint == 2)     version = 5.0;

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
      if (someint == 0)    new(agelConstantsA[agel]) ARICHAerogelInfo(version, serial, id, index, trlen, thickness, lambdas,
            transmittances);
      if (someint == 1)    new(agelConstantsB[agel]) ARICHAerogelInfo(version, serial, id, index, trlen, thickness, lambdas,
            transmittances);
      if (someint == 2)    new(agelConstantsC[agel]) ARICHAerogelInfo(version, serial, id, index, trlen, thickness, lambdas,
            transmittances);
      agel++;
    }
  }

  // set interval of validity
  IntervalOfValidity iov(0, 0, 0, 99);
//  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

  // convert pointers to ARICHAerogelInfo into pointers to TObject
  TObject* agelObj[3];
  agelObj[0] = static_cast<TObject*>(&agelConstantsA);
  agelObj[1] = static_cast<TObject*>(&agelConstantsB);
  agelObj[2] = static_cast<TObject*>(&agelConstantsC);

  // add objects with different validity
  EventDependency intraRun(agelObj[0]);
  intraRun.add(500, agelObj[1]);   // valid from event number 500
  intraRun.add(1000, agelObj[2]);  // valid from event number 1000

  // store under user defined name
  Database::Instance().storeData("ARICHAerogelInfoEventDep", &intraRun, iov);

}

void ARICHDatabaseImporter::exportAerogelInfoEventDep()
{
  // Definition:
  // This function extracts intrarun dependant data from the database.
  // It converts between different class types to get the saved
  // TClonesArray for chosen event, run and experiment.


  // Define event, run and experiment numbers
  EventMetaData event = EventMetaData(1200, 4, 0); // (event, run, exp)

  // Extract object and IOV from database
  std::pair<TObject*, IntervalOfValidity> podatki = Database::Instance().getData(event, "dbstore", "ARICHAerogelInfoEventDep");

  // print interval of validity
//  IntervalOfValidity iov = std::get<1>(podatki);
//  B2INFO("iov = " << iov);

  // Convert between different class types and extract TClonesArray
  // for chosen event
  TObject* data = std::get<0>(podatki);
  EventDependency* data2 = static_cast<EventDependency*>(data);
  TObject* myobject = data2->getObject(event);
  TClonesArray* elements = static_cast<TClonesArray*>(myobject);

  // Get entries from TClonesArray and print aerogel info
  (*elements).GetEntries();
  for (int i = 0; i < elements->GetSize(); i++) {
    ARICHAerogelInfo* myelement = (ARICHAerogelInfo*)elements->At(i);
    B2INFO("Version = " << myelement->getAerogelVersion() << ", SN = " << myelement->getAerogelSerial() << ", n = " <<
           myelement->getAerogelRefractiveIndex() << ", trl = " << myelement->getAerogelTransmissionLength() << ", thickness = " <<
           myelement->getAerogelThickness());
  }

  /*

    // Extract data from the last added payload
    DBArray<ARICHAerogelInfo> elements("ARICHAerogelInfo");
    elements.getEntries();

    // Print aerogel info

    for (const auto& element : elements) {
      B2INFO("Version = " << element.getAerogelVersion() << ", serial = " << element.getAerogelSerial() <<
             ", id = " << element.getAerogelID() << ", n = " << element.getAerogelRefractiveIndex() << ", transmLength = " <<
             element.getAerogelTransmissionLength() << ", thickness = " << element.getAerogelThickness());
    }

  */
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
    TTimeStamp arrivalDate;
    int arrival;
    TTree* tree = 0;

    // extract data
    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {

      string strime = key->GetName();

      if (strime.find("gcurrent") == 0) {
        TGraph* graphcurrent = (TGraph*)f->Get(strime.c_str());
        leakCurrent.push_back(graphcurrent);
      }

      else if (strime.find("h2dscan") == 0) {
        hitData2D = (TH2F*)f->Get(strime.c_str());
        hitData2D->SetDirectory(0);
      }

      else if (strime.find("gnoise_ch") == 0) {
        TGraph* graphnoise = (TGraph*)f->Get(strime.c_str());
        noise.push_back(graphnoise);
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

      else if (strime.find("tree") == 0) {
        tree = (TTree*)f->Get(strime.c_str());
        tree->SetBranchAddress("arrival", &arrival);
        tree->GetEntry(0);
        arrivalDate = TTimeStamp(arrival, 0);
      }

      else { B2INFO("Key name does not match any of the following: gcurrent, 2dscan, gnoise, hchscan, tree_ts! - serial number: " << hapdSerial << "; key name = " << strime.c_str()); }
    }



    // save data as an element of the array
    new(hapdQAConstants[hapd]) ARICHHapdQA(hapdSerial, arrivalDate, leakCurrent, hitData2D, noise, hitCount);
    hapd++;
  }

  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData("ARICHHapdQA", &hapdQAConstants, iov);
}


void ARICHDatabaseImporter::exportHapdQA()
{
  DBArray<ARICHHapdQA> elements("ARICHHapdQA");
  elements.getEntries();

  // Print serial numbers of HAPDs
  unsigned int el = 0;
  for (const auto& element : elements) {
    B2INFO("Serial number = " << element.getHapdSerialNumber() << "; arrival date = " << element.getHapdArrivalDate());
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
    string asicName = inputFileNew.substr(inputFileNew.find("/asicData") + 19);
    size_t findRoot = asicName.find(".root");
    if (findRoot != string::npos) {
      string asicSerial = asicName.erase(findRoot);

      // extract the data from files
      TFile* f = TFile::Open(inputFile.c_str(), "READ", "", 0);
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

      // get time of measurement
      timeFinishGain = ARICHDatabaseImporter::getAsicDate(asicSerial, "gain");
      timeFinishOffset = ARICHDatabaseImporter::getAsicDate(asicSerial, "offset");

      // get lists of bad channels
      vector<int> nosignalCHs = ARICHDatabaseImporter::channelsList(asicSerial, "nosignal", 0);
      vector<int> badconnCHs = ARICHDatabaseImporter::channelsList(asicSerial, "badconn", 0);
      vector<int> badoffsetCHs = ARICHDatabaseImporter::channelsList(asicSerial, "badoffset", 0);
      vector<int> badlinCHs = ARICHDatabaseImporter::channelsList(asicSerial, "badlin", 0);

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
      new(asicConstants[asic])  ARICHAsicInfo();
      auto* asicConst = static_cast<ARICHAsicInfo*>(asicConstants[asic]);

      asicConst->setAsicID(asicSerial);
      asicConst->setTimeFinishGain(timeFinishGain);
      asicConst->setTimeFinishOffset(timeFinishOffset);
      asicConst->setDeadChannels(nosignalCHs);
      asicConst->setBadConnChannels(badconnCHs);
      asicConst->setBadOffsetChannels(badoffsetCHs);
      asicConst->setBadLinChannels(badlinCHs);
      asicConst->setNumOfProblematicChannels(num);
      asicConst->setGainMeasurement(m_gain);
      asicConst->setOffsetMeasurement(m_offset);

      f->Close();
      // define IOV and store data to the DB
      IntervalOfValidity iov(0, 0, -1, -1);
      string nameAsicInfo = "ARICHAsicInfo_" + asicSerial;
      Database::Instance().storeData(nameAsicInfo, asicConstants[asic], iov);

      asic++;
    }
  }
//  IntervalOfValidity iov(0, 0, -1, -1);
//  Database::Instance().storeData("ARICHAsicInfo", &asicConstants, iov);
}

TTimeStamp ARICHDatabaseImporter::getAsicDate(string asicSerial, string type)
{
  TTimeStamp timeFinish;
  string line;
  size_t findText = 100;

  // extract measurement date for serial number
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

vector<int> ARICHDatabaseImporter::channelsList(string asicSerial, string type, int chDelay)
{

  GearDir content = GearDir("/ArichData/AllData/asicList");
  vector<int> CHs;

  for (const auto& asicInfo : content.getNodes("asicinfo")) {
    string serial = asicInfo.getString("serial");
    // find data for the right serial number
    if (asicSerial == serial)  {
      string badCH = asicInfo.getString(type.c_str());
      // parse string to get numbers of all bad channels
      if (!badCH.empty()) {
        if (badCH.find(",") != string::npos) {
          while (badCH.find(",") != string::npos) {
            string CH = badCH.substr(0, badCH.find(","));
            int badchannel = atoi(CH.c_str()) + chDelay;
            CHs.push_back(badchannel);
            badCH = badCH.substr(badCH.find(",") + 1);
          }
          int badchannel = atoi(badCH.c_str()) + chDelay;
          CHs.push_back(badchannel);
        }
        // store 5000 if there are many bad channels
        else if (badCH.find("many") != string::npos) {
          CHs.push_back(5000);
        }
        // store 10000 if all channels are bad
        else if (badCH.find("all") != string::npos) {
          CHs.push_back(10000);
        } else {
          int badchannel = atoi(badCH.c_str()) + chDelay;
          CHs.push_back(badchannel);
        }
      }
    }
  }
  return CHs;
}


void ARICHDatabaseImporter::exportAsicInfo()
{
  DBArray<ARICHAsicInfo> elements("ARICHAsicInfo");
  elements.getEntries();

  // Print serial numbers of ASICs
  unsigned int el = 0;
  for (const auto& element : elements) {
    B2INFO("Element Number: " << el << "; serial = " << element.getAsicID() << "; dead ch no.1 = " << element.getDeadChannel(
             0) << "; all bad ch, N = " << element.getNumOfProblematicChannels());
    /*  TH3F* hist1 = element.getOffsetMeasurement(0);
      TFile file("histogrami.root", "update");
      hist1->Write();
      file.Close();*/

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

  // find matching sn and dna
  for (const auto& febmap : content1.getNodes("feb")) {
    int serial = febmap.getInt("sn");
    string dna = febmap.getString("dna");

    // save data as an element of the array
    new(febConstants[feb])  ARICHFebTest();
    auto* febConst = static_cast<ARICHFebTest*>(febConstants[feb]);

    febConst->setFebSerial(serial);
    febConst->setFebDna(dna);
    febConst->setDeadChannels(ARICHDatabaseImporter::getDeadChFEB(dna));

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
    std::tuple<std::string, float> HVtest = ARICHDatabaseImporter::getFebHVtestData(serial);
    febConst->setTimeHV(ARICHDatabaseImporter::timedate2(std::get<0>(HVtest)));
    febConst->setCurrentV99p(std::get<1>(HVtest));

    // low voltage test data
    if (serial < 84) {
      std::tuple<std::string, float, float, float> LVtest = ARICHDatabaseImporter::getFebLVtestData(serial);
      febConst->setTimeLV(ARICHDatabaseImporter::timedate2(std::get<0>(LVtest)));
      febConst->setCurrentV20p(std::get<1>(LVtest));
      febConst->setCurrentV21n(std::get<2>(LVtest));
      febConst->setCurrentV38p(std::get<3>(LVtest));
    }

    // slopes (from offset settings measurements)
    std::pair<std::vector<float>, std::vector<float>> slopes = ARICHDatabaseImporter::getSlopes(dna);

    febConst->setSlopesFine(std::get<0>(slopes)); // std::vector<float>
    febConst->setSlopesRough(std::get<1>(slopes)); // std::vector<float>


//    febConst->setOffsetFine3D(offsetFine); // TH3F*
//    febConst->setOffsetRough3D(offsetRough); // TH3F*
//    febConst->setTestPulse2D(testPulse); // TH2F*

    feb++;
  }

  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1);
  Database::Instance().storeData("ARICHFebTest", &febConstants, iov);
}

std::tuple<std::string, float, float, float> ARICHDatabaseImporter::getFebLVtestData(int serial)
{
  GearDir content2LV = GearDir("/ArichData/AllData/FEBDataLV/Content");
  std::tuple<std::string, float, float, float> LVtest;

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
            LVtest = std::make_tuple(timeLV, currentV20p, currentV21n, currentV38p);
          }
        }
      }
    }
  }
  return LVtest;
}


std::tuple<std::string, float> ARICHDatabaseImporter::getFebHVtestData(int serial)
{
  GearDir content2HV = GearDir("/ArichData/AllData/FEBDataHV/Content");
  std::tuple<std::string, float> HVtest;

  for (const auto& testFEBhv : content2HV.getNodes("febtest")) {
    string timeHV = testFEBhv.getString("time");
    for (const auto& testFEBhv_sn : testFEBhv.getNodes("hvtest/feb")) {
      int serial_hv = testFEBhv_sn.getInt("sn");
      if (serial_hv == serial) {
        for (const auto& testFEBhv_i : testFEBhv_sn.getNodes("febhv/n")) {
          int n_id = testFEBhv_i.getInt("@id");
          if (n_id == 9) {
            float currentV99p = (float) testFEBhv_i.getDouble("n1470/I");
            HVtest = std::make_tuple(timeHV, currentV99p);
          }
        }
      }
    }
  }
  return HVtest;
}

std::vector<int> ARICHDatabaseImporter::getDeadChFEB(std::string dna)
{
  vector<int> listCHs;
  string line;
  ifstream fileFEB("febTest/FEBdeadChannels.txt");
  if (fileFEB.is_open()) {
    while (getline(fileFEB, line)) {
      string ch2 = line.substr(line.find(",") + 1);
      string dna2 = line.erase(line.find(",") - 1);
      if (dna2 == dna) { listCHs.push_back(atoi(ch2.c_str())); }
    }
  } else { B2INFO("No file FEBdeadChannels.txt"); }
  fileFEB.close();

  return listCHs;
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

std::pair<std::vector<float>, std::vector<float>> ARICHDatabaseImporter::getSlopes(std::string dna)
{
  vector<float> slopesF, slopesR;
  std::pair<std::vector<float>, std::vector<float>> slopes;
  string line;
  ifstream fileSlopes("febTest/SlopesFEBpravi.txt");
  if (fileSlopes.is_open()) {
    while (getline(fileSlopes, line)) {
      string dna2 = line.substr(0, line.find(",") - 1);
      string line2 = line.substr(line.find(",") + 1);
      string slopeR = line2.substr(0, line2.find(",") - 1);
      string slopeF = line2.substr(line2.find(",") + 1);
      if (dna2 == dna) {
        float slopeRfloat = (-1) * 2500.0 / 1024 * stof(slopeR.c_str());
        slopesR.push_back(slopeRfloat);
        float slopeFfloat = (-1) * 2500.0 / 1024 * stof(slopeF.c_str());
        slopesF.push_back(slopeFfloat);
      }
    }
  } else { B2INFO("No file SlopesFEBpravi.txt"); }
  fileSlopes.close();
  slopes = std::make_pair(slopesF, slopesR);

  return slopes;
}


void ARICHDatabaseImporter::exportFebTest()
{
  DBArray<ARICHFebTest> elements("ARICHFebTest");
  elements.getEntries();

  // Print serial numbers of FEBs
  unsigned int el = 0;
  for (const auto& element : elements) {
    B2INFO("Element Number: " << el << "; serial = " << element.getFebSerial() << "; dna = " << element.getFebDna() <<
           "; slope fine ch. 143 = " << element.getSlopeFine(143));
    el++;
  }
}



void ARICHDatabaseImporter::importHapdInfo()
{
  int hapd_i = 0;
  GearDir content = GearDir("/ArichData/AllData/hapdData/Content");

  // define data array
  TClonesArray hapdConstants("Belle2::ARICHHapdInfo");

  // extract chip info, such as bias voltage, lists of dead and bad channels etc.
  for (const auto& hapdInfo : content.getNodes("hapd")) {
    // define element of TClonesArray
    new(hapdConstants[hapd_i]) ARICHHapdInfo();
    auto* hapdConst = static_cast<ARICHHapdInfo*>(hapdConstants[hapd_i]);

    // extract information about HAPD
    string serial = hapdInfo.getString("serial");
    float qe400 = (float) hapdInfo.getDouble("qe400");
    float hv = 1000 * (float) hapdInfo.getDouble("hv");
    float current = (float) hapdInfo.getDouble("current");
    string gb = hapdInfo.getString("guardbias");
    int guardbias = atoi(gb.c_str());

    // prepare TGraph of quantum efficiency as function of lambda
    const int n1 = 70;
    float lambda[n1], qepoint[n1];
    int i1 = 0;
    for (const auto& QE : hapdInfo.getNodes("qe/qepoint")) {
      lambda[i1] = (float) QE.getInt("@lambda");
      qepoint[i1] = (float) QE.getDouble(".");
      i1++;
    }
    TGraph* qe = new TGraph(i1, lambda, qepoint);
    qe->SetName("qe");
    qe->SetTitle("qe");
    qe->GetXaxis()->SetTitle("lambda");
    qe->GetYaxis()->SetTitle("qe");

    // prepare TGraph of pulse height distribution
    const int n2 = 4100;
    int channel_adc[n2], pulse_adc[n2];
    int i2 = 0;
    for (const auto& ADC : hapdInfo.getNodes("adc/value")) {
      channel_adc[i2] = ADC.getInt("@ch");
      string str = ADC.getString(".");
      pulse_adc[i2] = atoi(str.c_str());
      i2++;
    }
    TGraph* adc = new TGraph(i2, channel_adc, pulse_adc);
    adc->SetName("adc");
    adc->SetTitle("Pulse Height Distribution");
    adc->GetXaxis()->SetTitle("channel");
    adc->GetYaxis()->SetTitle("pulse height");

    // save HAPD data to the element of TClonesArray
    hapdConst->setSerialNumber(serial);
    hapdConst->setQuantumEfficiency400(qe400);
    hapdConst->setHighVoltage(hv);
    hapdConst->setGuardBias(guardbias);
    hapdConst->setCurrent(current);
    hapdConst->setQuantumEfficiency(qe);
    hapdConst->setPulseHeightDistribution(adc);

    // define objects for chip info
    string chip[4];
    int channel_label_aval[4], bias[4], gain[4];
    TGraph** bombardmentGain, **avalancheGain;
    bombardmentGain = new TGraph *[4];
    avalancheGain = new TGraph *[4];
    TH2F** bias2DV, **bias2DI;
    bias2DV = new TH2F *[4];
    bias2DI = new TH2F *[4];
    vector<int> badlist[4], cutlist[4];
    vector<TGraph*> bombCurrents[4], avalCurrents[4];
    int chip_ABCD = 0;

    // extract information for each chip
    for (const auto& chipInfo : hapdInfo.getNodes("chipinfo")) {
      chip[chip_ABCD] = chipInfo.getString("chip");
      bias[chip_ABCD] = chipInfo.getInt("bias");
      string badL = chipInfo.getString("deadlist");
      string cutL = chipInfo.getString("cutlist");
      if (badL.find("ch") != string::npos) { string badLsub = badL.substr(3); badlist[chip_ABCD] = ARICHDatabaseImporter::channelsListHapd(badLsub.c_str(), 0); }
      if (cutL.find("ch") != string::npos) {  string cutLsub = cutL.substr(3); cutlist[chip_ABCD] = ARICHDatabaseImporter::channelsListHapd(cutLsub.c_str(), 0); }
      string gain_str = chipInfo.getString("gain");
      gain[chip_ABCD] = atoi(gain_str.c_str());
      chip_ABCD++;
    }

    // prepare TGraphs for bombardment gain and current
    const int n3 = 30;
    int i3 = 0;
    //int channel_label_bomb;
    float hv_bomb[n3], gain_bomb[n3], current1_bomb[n3], current2_bomb[n3], current3_bomb[n3];
    chip_ABCD = 0;
    for (const auto& BG : hapdInfo.getNodes("bombardmentgain/ch")) {
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

      bombardmentGain[chip_ABCD] = ARICHDatabaseImporter::getGraphGainCurrent("Bombardment", "gain", chip_label, i3, hv_bomb, gain_bomb);
      TGraph* bombardmentCurrent1 = ARICHDatabaseImporter::getGraphGainCurrent("Bombardment", "current1", chip_label, i3, hv_bomb,
                                    current1_bomb);
      TGraph* bombardmentCurrent2 = ARICHDatabaseImporter::getGraphGainCurrent("Bombardment", "current2", chip_label, i3, hv_bomb,
                                    current2_bomb);
      TGraph* bombardmentCurrent3 = ARICHDatabaseImporter::getGraphGainCurrent("Bombardment", "current3", chip_label, i3, hv_bomb,
                                    current3_bomb);
      bombCurrents[chip_ABCD].push_back(bombardmentCurrent1);
      bombCurrents[chip_ABCD].push_back(bombardmentCurrent2);
      bombCurrents[chip_ABCD].push_back(bombardmentCurrent3);
      chip_ABCD++;
      i3 = 0;
    }

    // prepare TGraphs for avalanche gain and current
    const int n4 = 30;
    int i4 = 0;
    float hv_aval[n4], gain_aval[n4], current1_aval[n4], current2_aval[n4], current3_aval[n4];
    chip_ABCD = 0;
    for (const auto& BG : hapdInfo.getNodes("avalanchegain/ch")) {
      string value = BG.getString("value");
      string value_1 = value;
      string chip_label = value.erase(1);
      string value_2 = value_1.substr(2);
      channel_label_aval[chip_ABCD] = atoi(value_2.c_str());
      for (const auto& BG2 : BG.getNodes("point")) {
        hv_aval[i4] = (float) BG2.getDouble("biasv");
        gain_aval[i4] = (float) BG2.getDouble("gain");
        current1_aval[i4] = (float) BG2.getDouble("current1");
        current2_aval[i4] = (float) BG2.getDouble("current2");
        current3_aval[i4] = (float) BG2.getDouble("current3");
        i4++;
      }

      avalancheGain[chip_ABCD] = ARICHDatabaseImporter::getGraphGainCurrent("Avalanche", "gain", chip_label, i4, hv_aval, gain_aval);
      TGraph* avalancheCurrent1 = ARICHDatabaseImporter::getGraphGainCurrent("Avalanche", "current1", chip_label, i4, hv_aval,
                                  current1_aval);
      TGraph* avalancheCurrent2 = ARICHDatabaseImporter::getGraphGainCurrent("Avalanche", "current2", chip_label, i4, hv_aval,
                                  current2_aval);
      TGraph* avalancheCurrent3 = ARICHDatabaseImporter::getGraphGainCurrent("Avalanche", "current3", chip_label, i4, hv_aval,
                                  current3_aval);
      avalCurrents[chip_ABCD].push_back(avalancheCurrent1);
      avalCurrents[chip_ABCD].push_back(avalancheCurrent2);
      avalCurrents[chip_ABCD].push_back(avalancheCurrent3);

      chip_ABCD++;
      i4 = 0;
    }


    chip_ABCD = 0;
    // prepare 2D histograms for bias voltage and current
    const int n5 = 150;
    int i5 = 0, chipnum[n5];
    float biasv[n5], biasi[n5];
    for (const auto& HI : hapdInfo.getNodes("bias2d/biasvalue")) {
      string chip_2d = HI.getString("@chip");
      chipnum[i5] = HI.getInt("@ch");
      biasv[i5] = (float) HI.getDouble("biasv");
      biasi[i5] = (float) HI.getDouble("biasi");
      if (chipnum[i5] == 36) {
        bias2DV[chip_ABCD] = ARICHDatabaseImporter::getBiasGraph(chip_2d, "voltage", chipnum, biasv);
        bias2DI[chip_ABCD] = ARICHDatabaseImporter::getBiasGraph(chip_2d, "current", chipnum, biasi);
        i5 = -1;
        chip_ABCD++;
      }
      i5++;
    }

    // prepare ARICHHapdChipInfo class for each chip and
    // add it as an element of ARICHHapdInfo class
    for (unsigned int l = 0; l < 4; l++)  {
      ARICHHapdChipInfo* chipConstants = new ARICHHapdChipInfo();
      auto* chipConst = static_cast<ARICHHapdChipInfo*>(chipConstants);

      chipConst->setHapdSerial(serial);
      chipConst->setChipLabel(chip[l]);
      chipConst->setBiasVoltage(bias[l]);
      chipConst->setGain(gain[l]);
      chipConst->setBadChannel(badlist[l]);
      chipConst->setCutChannel(cutlist[l]);
      chipConst->setBombardmentGain(bombardmentGain[l]);
      chipConst->setBombardmentCurrent(bombCurrents[l]);
      chipConst->setAvalancheGain(avalancheGain[l]);
      chipConst->setAvalancheCurrent(avalCurrents[l]);
      chipConst->setChannelNumber(channel_label_aval[l]);
      chipConst->setBiasVoltage2D(bias2DV[l]);
      chipConst->setBiasCurrent2D(bias2DI[l]);

      hapdConst->setHapdChipInfo(l, *chipConst);
    }

    hapd_i++;
  }

  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1);
  Database::Instance().storeData("ARICHHapdInfo", &hapdConstants, iov);
}


// get list of bad channels on HAPD
std::vector<int> ARICHDatabaseImporter::channelsListHapd(std::string chlist, int channelDelay)
{
  vector<int> CHs;
  // parse string to get numbers of all bad channels
  if (chlist.find(",") != string::npos) {
    while (chlist.find(",") != string::npos) {
      string CH = chlist.substr(0, chlist.find(","));
      int badChannel = atoi(CH.c_str()) + channelDelay;
      CHs.push_back(badChannel);
      chlist = chlist.substr(chlist.find(",") + 1);
    }
    int badChannel = atoi(chlist.c_str()) + channelDelay;
    CHs.push_back(badChannel);
  } else {
    int badChannel = atoi(chlist.c_str()) + channelDelay;
    CHs.push_back(badChannel);
  }
  return CHs;
}

// prepare TGraph for bombardment/avalanche gain/current
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

// use correct mapping of channels on HAPD
int ARICHDatabaseImporter::getChannelPosition(std::string XY, std::string chip_2d, int chipnum)
{
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

// prepare 2D histogram for bias voltage/current
TH2F* ARICHDatabaseImporter::getBiasGraph(std::string chip_2d, std::string voltage_current, int* chipnum, float* bias_v_i)
{
  string name = "bias " + voltage_current + ", chip " + chip_2d;
  TH2F* bias2d = new TH2F("bias2d", name.c_str(), 6, 0, 6, 6, 0, 6);
  for (int XYname = 0; XYname < 6; XYname++) {
    bias2d->GetXaxis()->SetBinLabel(XYname + 1, to_string(XYname).c_str());
    bias2d->GetYaxis()->SetBinLabel(XYname + 1, to_string(6 * XYname).c_str());
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
  DBArray<ARICHHapdInfo> elements("ARICHHapdInfo");
  elements.getEntries();

  for (const auto& element : elements) {
    B2INFO("Serial = " << element.getSerialNumber() << "; HV = " << element.getHighVoltage() << "; qe400 = " <<
           element.getQuantumEfficiency400());
    TGraph* adc = element.getPulseHeightDistribution();
    TFile file("histogrami.root", "update");
    adc->Write();
    file.Close();
    ARICHHapdChipInfo newelement;
    for (int n = 0; n < 4; n++)  {
      newelement = element.getHapdChipInfo(n);
      B2INFO("biasV(chip" << n << ") = " << newelement.getBiasVoltage());
    }
  }
}


void ARICHDatabaseImporter::importHapdQE()
{
  // define data array
  TClonesArray hapdQEConstants("Belle2::ARICHHapdQE");
  int hapd = 0;

  // loop over root riles
  for (const string& inputFile : m_inputFilesHapdQE) {

    TFile* f = TFile::Open(inputFile.c_str(), "READ");

    int size = inputFile.length();
    string hapdSerial = inputFile.substr(size - 11, 6);
    TH2F* qe2D = 0;

    // extract data
    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {

      string strime = key->GetName();

      if (strime.find("hqe2d_pixel") == 0) {
        qe2D = (TH2F*)f->Get(strime.c_str());
        qe2D->SetTitle("quantum efficiency");
        qe2D->SetName("QE");
        qe2D->SetDirectory(0);
      }

      else { B2INFO("Key name does not match 'hqe2d_pixel'!"); }
    }

    // save data as an element of the array
    new(hapdQEConstants[hapd]) ARICHHapdQE(hapdSerial, qe2D);
    hapd++;
  }

  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData("ARICHHapdQE", &hapdQEConstants, iov);
}


void ARICHDatabaseImporter::exportHapdQE()
{
  DBArray<ARICHHapdQE> elements("ARICHHapdQE");
  elements.getEntries();

  // Print serial numbers of HAPDs
  unsigned int el = 0;
  for (const auto& element : elements) {
    B2INFO("SN = " << element.getHapdSerialNumber());
    /* TH2F* qe2d = element.getQuantumEfficiency2D();
      TFile file("histogrami.root", "update");
      qe2d->Write();
      file.Close(); */

    el++;
  }
}

void ARICHDatabaseImporter::importBadChannels()
{
  // define array
  TClonesArray channelConstants("Belle2::ARICHBadChannels");

  int i = 0, j = 0;

  GearDir hapdContent = GearDir("/ArichData/AllData/hapdData/Content");
  GearDir febContent = GearDir("/ArichData/AllData/febmapping");
  GearDir asicContent = GearDir("/ArichData/AllData/febasicmapping");

  string hapdSerial[1000], chip;
  vector<int> hapdBadlist[1000], hapdCutlist[1000], febDeadlist[1000], asicNosignalCHs[1000], asicBadconnCHs[1000],
         asicBadoffsetCHs[1000], asicBadlinCHs[1000];
  int febSerial[1000];

  // extract chip info, such as bias voltage, lists of dead and bad channels etc.
  for (const auto& hapdInfo : hapdContent.getNodes("hapd")) {
    hapdSerial[i] = hapdInfo.getString("serial");
    for (const auto& chipInfo : hapdInfo.getNodes("chipinfo")) {
      chip = chipInfo.getString("chip");
      string badL = chipInfo.getString("deadlist");
      string cutL = chipInfo.getString("cutlist");
      int channelDelay = 0;
      if (chip == "B") channelDelay = 36;
      if (chip == "C") channelDelay = 2 * 36;
      if (chip == "D") channelDelay = 3 * 36;

      if (badL.find("ch") != string::npos) {  string badLsub = badL.substr(3); hapdBadlist[i] = ARICHDatabaseImporter::channelsListHapd(badLsub.c_str(), channelDelay); }

      if (cutL.find("ch") != string::npos) {  string cutLsub = cutL.substr(3); hapdCutlist[i] = ARICHDatabaseImporter::channelsListHapd(cutLsub.c_str(), channelDelay); }
    }
    i++;
  }

  // find matching sn and dna and get list of dead channels
  for (const auto& febmap : febContent.getNodes("feb")) {
    febSerial[j] = febmap.getInt("sn");
    string dna = febmap.getString("dna");
    febDeadlist[j] = ARICHDatabaseImporter::getDeadChFEB(dna);
    for (const auto& asicmap : asicContent.getNodes("febasic")) {
      int febSerial2 = asicmap.getInt("sn");
      if (febSerial2 == febSerial[j]) {
        string asic1 = asicmap.getString("asic1");
        string asic2 = asicmap.getString("asic2");
        string asic3 = asicmap.getString("asic3");
        string asic4 = asicmap.getString("asic4");

        vector<int> asic_NosignalCHs[4], asic_BadconnCHs[4], asic_BadoffsetCHs[4], asic_BadlinCHs[4];
        string asicNo = "";
        int chDelay = 0;

        for (int n = 0; n < 4; n++)  {
          if (n == 0) { asicNo = asic1; chDelay = 0;}
          if (n == 1) { asicNo = asic2; chDelay = 36;}
          if (n == 2) { asicNo = asic3; chDelay = 2 * 36;}
          if (n == 3) { asicNo = asic4; chDelay = 3 * 36;}
          asic_NosignalCHs[n] = ARICHDatabaseImporter::channelsList(asicNo, "nosignal", chDelay);
          asic_BadconnCHs[n] = ARICHDatabaseImporter::channelsList(asicNo, "badconn", chDelay);
          asic_BadoffsetCHs[n] = ARICHDatabaseImporter::channelsList(asicNo, "badoffset", chDelay);
          asic_BadlinCHs[n] = ARICHDatabaseImporter::channelsList(asicNo, "badlin", chDelay);
        }

        for (int n = 0; n < 4; n++) {
          asicNosignalCHs[j].insert(asicNosignalCHs[j].end(), asic_NosignalCHs[n].begin(), asic_NosignalCHs[n].end()) ;
          asicBadconnCHs[j].insert(asicBadconnCHs[j].end(), asic_BadconnCHs[n].begin(), asic_BadconnCHs[n].end()) ;
          asicBadoffsetCHs[j].insert(asicBadoffsetCHs[j].end(), asic_BadoffsetCHs[n].begin(), asic_BadoffsetCHs[n].end()) ;
          asicBadlinCHs[j].insert(asicBadlinCHs[j].end(), asic_BadlinCHs[n].begin(), asic_BadlinCHs[n].end()) ;
        }
      }
    }
    j++;
  }


  int l = 0;
  // fill HAPD data to ARICHBadChannels class
  for (l = 0; l < i; l++) {
    new(channelConstants[l])  ARICHBadChannels();
    auto* channelConst = static_cast<ARICHBadChannels*>(channelConstants[l]);
    channelConst->setHapdSerial(hapdSerial[l]);
    channelConst->setHapdBadChannel(hapdBadlist[l]);
    channelConst->setHapdCutChannel(hapdCutlist[l]);
    channelConst->setID(l);
  }

  // fill FEB data to ARICHBadChannels class
  for (int k = i; k < (i + j); k++) {
    new(channelConstants[k])  ARICHBadChannels();
    auto* channelConst = static_cast<ARICHBadChannels*>(channelConstants[k]);
    channelConst->setFebSerial(febSerial[k - i]);
    channelConst->setFebDeadChannels(febDeadlist[k - i]);
    channelConst->setAsicDeadChannels(asicNosignalCHs[k - i]);
    channelConst->setAsicBadConnChannels(asicBadconnCHs[k - i]);
    channelConst->setAsicBadOffsetChannels(asicBadoffsetCHs[k - i]);
    channelConst->setAsicBadLinChannels(asicBadlinCHs[k - i]);
    channelConst->setID(k);
  }

  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1);
  //Database::Instance().storeData("ARICHBadChannels", &channelConstants, iov);

  /*  TObject* channelObj = static_cast<TObject*>(&channelConstants);

    EventDependency intraRun(channelObj);
    intraRun.add(500, channelObj);  // configuration B is valid staring from event number 500
    intraRun.add(1000, channelObj);  // configuration C is valid staring from event number 1000

    Database::Instance().storeData("ARICHBadChannels", &intraRun, iov);*/
  Database::Instance().storeData("ARICHBadChannels", &channelConstants, iov);


}

void ARICHDatabaseImporter::exportBadChannels()
{
  DBArray<ARICHBadChannels> elements("ARICHBadChannels");
  elements.getEntries();

  // Print bad channels
  /*  for (const auto& element : elements) {
      if (!(element.getHapdSerial()).empty()) {
        B2INFO("HAPD sn = " << element.getHapdSerial());
        if (element.getHapdListOfBadChannelsSize() != 0) B2INFO("all bad CHs: ");
        for (int i = 0; i < element.getHapdListOfBadChannelsSize(); i++) {
          B2INFO(element.getHapdListOfBadChannel(i));
        }
        if (element.getHapdCutChannelsSize() != 0) {
          B2INFO("cut CHs: ");
          for (int i = 0; i < element.getHapdCutChannelsSize(); i++) {
            B2INFO(element.getHapdCutChannel(i));
          }
        }
        if (element.getHapdBadChannelsSize() != 0) {
          B2INFO("bad CHs: ");
          for (int i = 0; i < element.getHapdBadChannelsSize(); i++) {
            B2INFO(element.getHapdBadChannel(i));
          }
        }
      } else {
        B2INFO("FEB sn = " << element.getFebSerial());
        int numAllCh = element.getFebDeadChannelsSize() + element.getAsicDeadChannelsSize() + element.getAsicBadConnChannelsSize() +
                       element.getAsicBadOffsetChannelsSize() + element.getAsicBadLinChannelsSize();
        if (numAllCh != 0) B2INFO("all bad CHs: ");
        for (int i = 0; i < numAllCh; i++) {
          B2INFO(element.getFebListOfBadChannel(i));
        }
        if (element.getFebDeadChannelsSize() != 0) {
          B2INFO("dead CHs (FEB test): ");
          for (int i = 0; i < element.getFebDeadChannelsSize(); i++) {
            B2INFO(element.getFebDeadChannel(i));
          }
        }
        if (element.getAsicDeadChannelsSize() != 0) {
          B2INFO("dead CHs (asic): ");
          for (int i = 0; i < element.getAsicDeadChannelsSize(); i++) {
            B2INFO(element.getAsicDeadChannel(i));
          }
        }
        if (element.getAsicBadConnChannelsSize() != 0) {
          B2INFO("bad conn CHs (asic): ");
          for (int i = 0; i < element.getAsicBadConnChannelsSize(); i++) {
            B2INFO(element.getAsicBadConnChannel(i));
          }
        }
        if (element.getAsicBadOffsetChannelsSize() != 0) {
          B2INFO("bad offset CHs (asic): ");
          for (int i = 0; i < element.getAsicBadOffsetChannelsSize(); i++) {
            B2INFO(element.getAsicBadOffsetChannel(i));
          }
        }
        if (element.getAsicBadLinChannelsSize() != 0) {
          B2INFO("bad lin CHs (asic): ");
          for (int i = 0; i < element.getAsicBadLinChannelsSize(); i++) {
            B2INFO(element.getAsicBadLinChannel(i));
          }
        }
      }
    }*/
}



void ARICHDatabaseImporter::getBiasVoltagesForHapdChip(std::string serialNumber)
{
  // example that shows how to extract and use data
  // it calculates bias voltage at gain = 40 for each chip

  DBArray<ARICHHapdChipInfo> elements("ARICHHapdChipInfo");
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



void ARICHDatabaseImporter::getMyParams(std::string aeroSerialNumber)
{
  map<string, float> aerogelParams = ARICHDatabaseImporter::getAerogelParams(aeroSerialNumber);

  B2INFO("SN = " << aeroSerialNumber << "; n = " << aerogelParams.find("refractiveIndex")->second << "; transLen = " <<
         aerogelParams.find("transmissionLength")->second << "; thickness = " << aerogelParams.find("thickness")->second);
}


std::map<std::string, float> ARICHDatabaseImporter::getAerogelParams(std::string aeroSerialNumber)
{
  // Description:
  // This function loops over aerogel tiles and returns refractive index,
  // thickness and transmission length of aerogel for serial number

  std::map<std::string, float> aerogelParams;
  DBArray<ARICHAerogelInfo> elements("ARICHAerogelInfo");
  elements.getEntries();
  for (const auto& element : elements) {
    if ((element.getAerogelSerial()) == aeroSerialNumber) {
      aerogelParams = {
        { "refractiveIndex", element.getAerogelRefractiveIndex() },
        { "transmissionLength", element.getAerogelTransmissionLength() },
        { "thickness", element.getAerogelThickness() }
      };
    }
  }
  return aerogelParams;
}



void ARICHDatabaseImporter::importSensorModule()
{
  GearDir content = GearDir("/ArichData/AllData/hapdfebmapping");

  // define data array
//  TClonesArray moduleInfoConstants("Belle2::ARICHSensorModuleInfo");
  TClonesArray moduleMapConstants("Belle2::ARICHSensorModuleMap");

  int module = 0;

  // loop over xml files and extract the data
  for (const auto& sensor : content.getNodes("hapdfeb")) {
    int febSerial = sensor.getInt("febserial");
    string hapdSerial = sensor.getString("hapdserial");
    int sextant = sensor.getInt("sextant");
    int ring = sensor.getInt("ring");
    int column = sensor.getInt("column");
    int id = sensor.getInt("moduleID");

    // save data as an element of the array
    new(moduleMapConstants[module]) ARICHSensorModuleMap();
    auto* moduleMapConst = static_cast<ARICHSensorModuleMap*>(moduleMapConstants[module]);
    moduleMapConst->setSensorModuleSextantID(sextant);
    moduleMapConst->setSensorModuleRingID(ring);
    moduleMapConst->setSensorModuleColumnID(column);

    ARICHSensorModuleInfo* moduleInfoConstants = new ARICHSensorModuleInfo();
    auto* moduleInfoConst = static_cast<ARICHSensorModuleInfo*>(moduleInfoConstants);
    moduleInfoConst->setSensorModuleID(id);
    moduleInfoConst->setFEBserial(febSerial);
    moduleInfoConst->setHAPDserial(hapdSerial);

    DBArray<ARICHHapdInfo> elementsHapd("ARICHHapdInfo");
    elementsHapd.getEntries();
    for (const auto& element : elementsHapd) {
      if (element.getSerialNumber() == hapdSerial) {  moduleInfoConst->setHapdID(element); }
    }

    /*    DBArray<ARICHFEBoardInfo> elementsFeb("ARICHFEBoardInfo");
        elementsFeb.getEntries();
        for (const auto& element : elementsFeb) {
          if (element.getFEBoardSerial() == febSerial) {  moduleInfoConst->setFEBoardID(element); }
        }*/

    moduleMapConst->setSensorModuleId(*moduleInfoConst);


    module++;
  }

  // define interval of validity
  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

  // store under default name:
//  Database::Instance().storeData("ARICHSensorModuleInfo", &moduleInfoConstants, iov);
  Database::Instance().storeData("ARICHSensorModuleMap", &moduleMapConstants, iov);
}



void ARICHDatabaseImporter::exportSensorModule()
{
  DBArray<ARICHSensorModuleMap> elements("ARICHSensorModuleMap");
  elements.getEntries();

  for (const auto& element : elements) {
    B2INFO("Sextant = " << element.getSensorModuleSextantID() << ", ring = " << element.getSensorModuleRingID() << ", column = " <<
           element.getSensorModuleColumnID());
    ARICHSensorModuleInfo newelement = element.getSensorModuleId();
    B2INFO("module ID = " << newelement.getSensorModuleID() << ", feb = " << newelement.getFEBserial() << ", hapd = " <<
           newelement.getHAPDserial());
    ARICHHapdInfo newerelement = newelement.getHapdID();
    B2INFO("Hapd Serial = " << newerelement.getSerialNumber() << "; HV = " << newerelement.getHighVoltage() << "; qe400 = " <<
           newerelement.getQuantumEfficiency400());

  }
}

// classses for simulation/reconstruction software
void ARICHDatabaseImporter::importModulesInfo()
{

  // geometry configuration
  DBObjPtr<ARICHGeometryConfig> geoConfig;

  // QE 2D maps from DB
  DBArray<ARICHHapdQE> QEMaps("ARICHHapdQE");

  ARICHModulesInfo modInfo;

  // channel mapping used in QA tests (QE measurements, etc.)
  ARICHChannelMapping QAChMap;

  // read mapping from xml file
  GearDir content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content/ChannelMapping");
  istringstream chstream;
  int x, y, asic;
  chstream.str(content.getString("QAChannelMapping"));
  while (chstream >> x >> y >> asic) {
    QAChMap.mapXY2Asic(x, y, asic);
  }

  // get list of installed modules from xml
  content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content/InstalledModules");
  std::cout << "Installed modules" << std::endl;

  std::vector<std::string> installed;

  for (const GearDir& module : content.getNodes("Module")) {
    std::string hapdID = module.getString("@hapdID");

    unsigned sector = module.getInt("Sector");
    unsigned ring = module.getInt("Ring");
    unsigned azimuth = module.getInt("Azimuth");
    bool isActive = (bool)module.getInt("isActive");
    std::cout << " " << hapdID << ":  S " << sector << "  R " << ring << "  Z " << azimuth <<  ", isActive: " << isActive << std::endl;

    if (std::find(installed.begin(), installed.end(), hapdID) != installed.end()) {
      B2WARNING("ARICHDatabaseImporter::importModulesInfo: hapd " << hapdID << " installed multiple times!");
    } else installed.push_back(hapdID);

    unsigned moduleID = geoConfig->getDetectorPlane().getSlotIDFromSRF(sector, ring, azimuth);

    // get and set QE map
    std::vector<float> qs;
    qs.assign(144, 0);
    bool init = false;
    for (const auto& QEMap : QEMaps) {
      if (hapdID ==  QEMap.getHapdSerialNumber()) {
        TH2F* qe2d = QEMap.getQuantumEfficiency2D();
        for (int k = 1; k < 13; k++) {
          for (int l = 1; l < 13; l++) {
            int asicCh = QAChMap.getAsicFromXY(k - 1, l - 1);
            qs[asicCh] = qe2d->GetBinContent(k, l);
          }
        }
        init = true;
        std::cout << "  Channels QE map found and set." << std::endl;
      }
    }

    if (!init) {
      for (int k = 0; k < 144; k++) {
        qs[k] = 27.0;
      }
      B2WARNING("ARICHDatabaseImporter::importModulesInfo: QE map for hapd " << hapdID << " not found! Setting 27% QE for all channels!");
    }

    modInfo.addModule(moduleID, qs, isActive);

  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  DBImportObjPtr<ARICHModulesInfo> importObj;
  importObj.construct(modInfo);
  importObj.import(iov);

}


void ARICHDatabaseImporter::importChannelMask()
{

  DBObjPtr<ARICHGeometryConfig> geoConfig;

  // module test results from DB (we take list of dead channels from here)
  DBArray<ARICHModuleTest> moduleTest("ARICHModuleTest");

  ARICHChannelMask chanMask;

  // read mapping of HAPD channels to asic channels from xml file
  GearDir content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content/ChannelMapping");
  istringstream chstream2;
  int hapdCh, asic;
  chstream2.str(content.getString("HapdAsicChannelMapping"));
  std::vector<int> hapdChMap;
  hapdChMap.assign(144, -1);
  while (chstream2 >> hapdCh >> asic) {
    hapdChMap[hapdCh - 1] = asic;
  }

  for (auto ch : hapdChMap) if (ch == -1)
      B2ERROR("ARICHDatabaseImporter::importLWClasses: HAPD channel to asic channel mapping not set correctly!");

  // loop over installed modules (from xml file)
  content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content/InstalledModules");
  std::cout << "Installed modules" << std::endl;
  for (const GearDir& module : content.getNodes("Module")) {
    std::string hapdID = module.getString("@hapdID");
    unsigned sector = module.getInt("Sector");
    unsigned ring = module.getInt("Ring");
    unsigned azimuth = module.getInt("Azimuth");
    bool isActive = (bool)module.getInt("isActive");
    std::cout << " " << hapdID << ":  S " << sector << "  R " << ring << "  Z " << azimuth <<  ", isActive: " << isActive << std::endl;
    unsigned moduleID = geoConfig->getDetectorPlane().getSlotIDFromSRF(sector, ring, azimuth);

    // get and set channel mask (mask dead channels)
    bool init = false;
    for (const auto& test : moduleTest) {
      if (hapdID ==  test.getHapdSN()) {

        // loop over list of dead channels
        for (int i = 0; i < test.getDeadChsSize(); i++) {
          unsigned hapdCh = test.getDeadCh(i);
          chanMask.setActiveCh(moduleID, hapdChMap[hapdCh - 1], false);
        }
        init = true;
        std::cout << "  List of dead channels (from module test) found and set." << std::endl;
      }
    }

    if (!init) {
      B2WARNING("ARICHDatabaseImporter::importLWClasses: List of dead channels for hapd " << hapdID <<
                " not found! All channels set to active.");
      continue;
    }
  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  DBImportObjPtr<ARICHChannelMask> importObj;
  importObj.construct(chanMask);
  importObj.import(iov);

}


void ARICHDatabaseImporter::importSimulationParams()
{

  ARICHSimulationPar simPar;

  GearDir content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content/SimulationParameters");

  double qeScale = content.getDouble("qeScale");
  double winAbs = content.getDouble("windowAbsorbtion");
  double crossTalk = content.getDouble("chipNegativeCrosstalk");
  double colEff = content.getDouble("colEff");

  GearDir qeParams(content, "QE");
  float lambdaFirst = qeParams.getLength("LambdaFirst") / Unit::nm;
  float lambdaStep = qeParams.getLength("LambdaStep") / Unit::nm;
  double peakQE = qeParams.getDouble("peakQE");

  std::vector<float> qes;
  for (const auto& qeff : qeParams.getNodes("Qeffi")) {
    qes.push_back(qeff.getDouble("qe"));
  }

  simPar.setQECurve(lambdaFirst, lambdaStep, qes);
  simPar.setCollectionEff(colEff);
  simPar.setChipNegativeCrosstalk(crossTalk);
  simPar.setWindowAbsorbtion(winAbs);
  simPar.setQEScaling(qeScale);
  simPar.setPeakQE(peakQE);

  simPar.print();

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  DBImportObjPtr<ARICHSimulationPar> importObj;
  importObj.construct(simPar);
  importObj.import(iov);

}

void ARICHDatabaseImporter::importChannelMapping()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content");
  ARICHChannelMapping chMap;

  istringstream chstream;
  int x, y, asic;
  chstream.str(content.getString("ChannelMapping/SoftChannelMapping"));
  std::cout << "Importing channel x,y to asic channel map" << std::endl;
  std::cout << "  x   y   asic" << std::endl;
  while (chstream >> x >> y >> asic) {
    chMap.mapXY2Asic(x, y, asic);
    std::cout << " " << setw(2) << x << "  " << setw(2) << y << "   " << setw(3) << asic << std::endl;
  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  DBImportObjPtr<ARICHChannelMapping> importObjMap;
  importObjMap.construct(chMap);
  importObjMap.import(iov);

}

void ARICHDatabaseImporter::importFEMappings()
{

  GearDir content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content");

  DBObjPtr<ARICHGeometryConfig> geoConfig;

  ARICHMergerMapping mergerMap;
  ARICHCopperMapping copperMap;
  GearDir mapping(content, "FrontEndMapping");

  for (const GearDir& merger : mapping.getNodes("Merger")) {
    unsigned mergerID = (unsigned) merger.getInt("@id");
    std::cout << std::endl << "Mapping of modules to merger no. " << mergerID << std::endl;
    for (const GearDir& module : merger.getNodes("Modules/Module")) {
      unsigned sector = module.getInt("Sector");
      unsigned ring = module.getInt("Ring");
      unsigned azimuth = module.getInt("Azimuth");
      unsigned moduleID = geoConfig->getDetectorPlane().getSlotIDFromSRF(sector, ring, azimuth);
      unsigned slot = (unsigned) module.getInt("@FEBSlot");
      mergerMap.addMapping(moduleID, mergerID, slot);
      std::cout << std::endl << " FEB slot: " << slot << ", module position: S" << sector << " R" << ring << " Z" << azimuth <<
                ", module ID: " << moduleID << std::endl;
      std::cout << " crosscheck:  mergerMap.getMergerID(" << moduleID << ") = " <<  mergerMap.getMergerID(
                  moduleID) << ", mergerMap.getFEBSlot(" << moduleID << ") = " << mergerMap.getFEBSlot(moduleID) << ", mergerMap.getModuleID(" <<
                mergerID << "," << slot << ") = " <<  mergerMap.getModuleID(mergerID, slot) << std::endl;
    }
    std::cout << std::endl;

    unsigned copperID = (unsigned) merger.getInt("COPPERid");
    string finesseSlot = merger.getString("FinesseSlot");
    int finesse = 0;
    if (finesseSlot == "A") {finesse = 0;}
    else if (finesseSlot == "B") {finesse = 1;}
    else if (finesseSlot == "C") {finesse = 2;}
    else if (finesseSlot == "D") {finesse = 3;}
    else {
      B2ERROR(merger.getPath() << "/FinesseSlot " << finesseSlot <<
              " ***invalid slot (valid are A, B, C, D)");
      continue;
    }
    copperMap.addMapping(mergerID, copperID, finesse);
    std::cout << "Merger " << mergerID << " connected to copper " << copperID << ", finesse " << finesse << std::endl;

  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  DBImportObjPtr<ARICHMergerMapping> importObjMerger;
  importObjMerger.construct(mergerMap);
  importObjMerger.import(iov);

  DBImportObjPtr<ARICHCopperMapping> importObjCopper;
  importObjCopper.construct(copperMap);
  importObjCopper.import(iov);

}

void ARICHDatabaseImporter::importGeometryConfig()
{

  GearDir content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content");
  ARICHGeometryConfig arichGeometryConfig(content);

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  DBImportObjPtr<ARICHGeometryConfig> importObj;
  importObj.construct(arichGeometryConfig);
  importObj.import(iov);

}

void ARICHDatabaseImporter::importCosmicTestGeometry()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content");
  GearDir cosmic(content, "CosmicTest");
  DBObjPtr<ARICHGeometryConfig> geoConfig;

  GearDir masterDir(cosmic, "MasterVolume");
  ARICHGeoMasterVolume master = geoConfig->getMasterVolume();
  master.setPlacement(masterDir.getLength("Position/x"), masterDir.getLength("Position/y"), masterDir.getLength("Position/z"),
                      masterDir.getAngle("Rotation/x"), masterDir.getAngle("Rotation/y"), masterDir.getAngle("Rotation/z"));
  master.setVolume(master.getInnerRadius(), master.getOuterRadius(), 100., master.getMaterial());
  geoConfig->setMasterVolume(master);


  GearDir aerogel(cosmic, "Aerogel");
  std::vector<double> par = {aerogel.getLength("xSize"), aerogel.getLength("ySize"), aerogel.getLength("xPosition"), aerogel.getLength("yPosition"), aerogel.getAngle("zRotation")};
  ARICHGeoAerogelPlane plane = geoConfig->getAerogelPlane();
  plane.setSimple(par);
  geoConfig->setAerogelPlane(plane);

  GearDir scints(cosmic, "Scintilators");
  double size[3] = {scints.getLength("xSize"), scints.getLength("ySize"), scints.getLength("zSize")};
  std::string scintMat = scints.getString("Material");

  ARICHGeoSupport support = geoConfig->getSupportStructure();
  support.clearBoxes();
  for (const GearDir& scint : scints.getNodes("Scintilator")) {
    std::string name = scint.getString("@name");
    double position[3] = {scint.getLength("Position/x"), scint.getLength("Position/y"), scint.getLength("Position/z")};
    double rotation[3] = {scint.getAngle("Rotation/x"), scint.getAngle("Rotation/y"), scint.getAngle("Rotation/z")};
    support.addBox(name, scintMat, size, position, rotation);
  }

  geoConfig->setSupportStructure(support);

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  DBImportObjPtr<ARICHGeometryConfig> geoImport;
  geoImport.construct(*geoConfig);
  geoImport.import(iov);

}

void ARICHDatabaseImporter::printSimulationPar()
{
  DBObjPtr<ARICHSimulationPar> simPar;
  simPar->print();
}

void ARICHDatabaseImporter::printGeometryConfig()
{
  DBObjPtr<ARICHGeometryConfig> geoConfig;
  geoConfig->print();

}

void ARICHDatabaseImporter::printChannelMapping()
{
  DBObjPtr<ARICHChannelMapping> chMap;
  chMap->print();
}

void ARICHDatabaseImporter::printFEMappings()
{
  DBObjPtr<ARICHMergerMapping> mrgMap;
  DBObjPtr<ARICHCopperMapping> copMap;
  mrgMap->print();
  copMap->print();
}

void ARICHDatabaseImporter::printModulesInfo()
{
  DBObjPtr<ARICHModulesInfo> modinfo;
  modinfo->print();
}

void ARICHDatabaseImporter::printChannelMask()
{
  DBObjPtr<ARICHChannelMask> chMask;
  chMask->print();
}

void ARICHDatabaseImporter::dumpModuleNumbering()
{

  ARICHChannelHist* hist = new ARICHChannelHist("moduleNum", "HAPD module slot numbering", 1);
  for (int hapdID = 1; hapdID < 421; hapdID++) {
    hist->setBinContent(hapdID, hapdID);
  }
  hist->SetOption("TEXT");
  hist->SaveAs("ModuleNumbering.root");

}

void ARICHDatabaseImporter::dumpQEMap(bool simple)
{
  DBObjPtr<ARICHModulesInfo> modInfo;
  DBObjPtr<ARICHGeometryConfig> geoConfig;
  DBObjPtr<ARICHChannelMapping> chMap;

  if (simple) {
    TGraph2D* gr = new TGraph2D();
    int point = 0;
    for (int i = 1; i < 421; i++) {
      if (modInfo->isInstalled(i)) {
        for (int j = 0; j < 144; j++) {
          int xCh, yCh;
          chMap->getXYFromAsic(j, xCh, yCh);
          TVector2 chPos = geoConfig->getChannelPosition(i, xCh, yCh);
          gr->SetPoint(point, chPos.X(), chPos.Y(), modInfo->getChannelQE(i, j));
          point++;
        }
      }
    }
    gr->SaveAs("QEMap.root");
  } else {
    ARICHChannelHist* hist = new ARICHChannelHist("hapdQE", "hapd QE map");
    for (int hapdID = 1; hapdID < 421; hapdID++) {
      if (!modInfo->isInstalled(hapdID)) continue;
      for (int chID = 0; chID < 144; chID++) {
        hist->setBinContent(hapdID, chID, modInfo->getChannelQE(hapdID, chID));
      }
    }
    hist->SaveAs("QEMap.root");
  }
}

