/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/calibration/TOPDatabaseImporter.h>
#include <top/geometry/TOPGeometryPar.h>

// framework - Database
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportArray.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>

// framework - xml
#include <framework/gearbox/GearDir.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DB objects
#include <top/dbobjects/TOPCalTimebase.h>
#include <top/dbobjects/TOPCalChannelT0.h>
#include <top/dbobjects/TOPCalChannelMask.h>
#include <top/dbobjects/TOPPmtGainPar.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <random>

#include "TFile.h"


using namespace std;
using namespace Belle2;

void TOPDatabaseImporter::importSampleTimeCalibration(std::string fileName)
{

  auto& chMapper = TOP::TOPGeometryPar::Instance()->getChannelMapper();
  auto& feMapper = TOP::TOPGeometryPar::Instance()->getFrontEndMapper();
  const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
  auto syncTimeBase = geo->getNominalTDC().getSyncTimeBase();

  std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportObjPtr<TOPCalTimebase> timeBase;
  timeBase.construct(syncTimeBase);

  // this is how to import from one of Gary's text files (module 3 and 4)

  for (int moduleID = 3; moduleID < 5; moduleID++) {
    for (int boardStack = 0; boardStack < 4; boardStack++) {
      auto* femap = feMapper.getMap(moduleID, boardStack);
      if (!femap) {
        B2ERROR("No FrontEnd map available for boardstack " << boardStack <<
                " of module " << moduleID);
        continue;
      }
      auto scrodID = femap->getScrodID();
      for (int carrierBoard = 0; carrierBoard < 4; carrierBoard++) {
        for (int asic = 0; asic < 4; asic++) {
          for (int chan = 0; chan < 8; chan++) {
            std::vector<double> sampleTimes;
            for (int sample = 0; sample < 256; sample++) {
              double data = 0;
              stream >> data;
              if (!stream.good()) {
                B2ERROR("Error reading sample time calibration constants");
                return;
              }
              sampleTimes.push_back(data);
            }
            auto channel = chMapper.getChannel(boardStack, carrierBoard, asic, chan);
            timeBase->append(scrodID, channel % 128, sampleTimes);
          }
        }
      }
    }
  }

  stream.close();
  int ndataRead = timeBase->getSampleTimes().size() * 256;
  B2INFO(fileName << ": file closed, " << ndataRead << " constants read from file");

  IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
  timeBase.import(iov);

  B2RESULT("Sample time calibration constants imported to database");

}


void TOPDatabaseImporter::importSampleTimeCalibrationKichimi(string fNames)
{
  vector<string> fileNames;
  stringstream ss(fNames);
  string fName;
  while (ss >> fName) {
    fileNames.push_back(fName);
  }

  auto& feMapper = TOP::TOPGeometryPar::Instance()->getFrontEndMapper();
  const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
  auto syncTimeBase = geo->getNominalTDC().getSyncTimeBase();

  DBImportObjPtr<TOPCalTimebase> timeBase;
  timeBase.construct(syncTimeBase);

  for (const auto& fileName : fileNames) {
    TFile* file = new TFile(fileName.c_str(), "r");
    if (!file) {
      B2ERROR("openFile: " << fileName << " *** failed to open");
      continue;
    }
    B2INFO(fileName << ": open for reading");

    // parse module ID from the file name
    auto i = fileName.rfind("/");
    if (i != string::npos) {
      i++;
    } else {
      i = 0;
    }
    if (fileName.substr(i, 1) != "s") {
      B2ERROR("No 's' found in the file name");
      continue;
    }
    string slot = fileName.substr(i + 1, 2);
    int moduleID = std::stoi(slot);
    if (!geo->isModuleIDValid(moduleID)) {
      B2ERROR("Module ID is not valid (incorrectly parsed from file name?): " << moduleID);
      continue;
    }
    B2INFO("--> importing constats for slot " << moduleID);

    string hname = "h_qasic[" +  std::to_string(moduleID) + "];1";
    TH1D* quality = (TH1D*) file->Get(hname.c_str());
    if (!quality) {
      B2ERROR("Quality histogram '" << hname << "' not found");
      continue;
    }

    int goodChannels = 0;
    for (int as = 0; as < 64; as++) { // as = ASIC + carrier * 4 + BS * 16

      string hname = "tbcval[" +  std::to_string(as) + "];1";
      TH1D* tbcval = (TH1D*) file->Get(hname.c_str());
      if (!tbcval) {
        B2ERROR("Histogram '" << hname << "' with calibration constants not found");
        continue;
      }

      std::vector<double> sampleTimes;
      if (quality->GetBinContent(as + 1) == 0 and tbcval->GetEntries() > 0) {
        double rescale = 1;
        if (tbcval->GetBinContent(257) > 0)
          rescale = 2 * syncTimeBase / tbcval->GetBinContent(257);
        for (int isamp = 0; isamp < 256; isamp++) {
          sampleTimes.push_back(tbcval->GetBinContent(isamp + 1) * rescale);
        }
        goodChannels++;
      }

      auto boardStack = as / 16;
      auto* femap = feMapper.getMap(moduleID, boardStack);
      if (!femap) {
        B2ERROR("No FrontEnd map available for boardstack " << boardStack <<
                " of module " << moduleID);
        continue;
      }
      auto scrodID = femap->getScrodID();
      for (int ch = 0; ch < 8; ch++) {
        auto channel = as * 8 + ch;
        if (sampleTimes.empty()) {
          timeBase->append(scrodID, channel % 128);
        } else {
          timeBase->append(scrodID, channel % 128, sampleTimes);
        }
      }
    }
    file->Close();
    B2INFO("--> number of calibrated asics: " << goodChannels << "/64");
    B2INFO("file closed");
  }

  int nall = timeBase->getSampleTimes().size();
  int ncal = 0;
  for (const auto& sampleTimes : timeBase->getSampleTimes()) {
    if (sampleTimes.isCalibrated()) ncal++;
  }

  IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
  timeBase.import(iov);

  B2RESULT("Sample time calibration constants imported to database, calibrated channels: "
           << ncal << "/" << nall);

}



void TOPDatabaseImporter::importChannelT0CalibrationKichimi(string fNames)
{
  vector<string> fileNames;
  stringstream ss(fNames);
  string fName;
  while (ss >> fName) {
    fileNames.push_back(fName);
  }

  auto& chMapper = TOP::TOPGeometryPar::Instance()->getChannelMapper();
  const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();

  DBImportObjPtr<TOPCalChannelT0> channelT0;
  channelT0.construct();

  int nall = 0;
  int ncal = 0;
  for (const auto& fileName : fileNames) {
    TFile* file = new TFile(fileName.c_str(), "r");
    if (!file) {
      B2ERROR("openFile: " << fileName << " *** failed to open");
      continue;
    }
    B2INFO(fileName << ": open for reading");

    // parse module ID from the file name
    auto i = fileName.rfind("/");
    if (i != string::npos) {
      i++;
    } else {
      i = 0;
    }
    if (fileName.substr(i, 1) != "s") {
      B2ERROR("No 's' found in the file name");
      continue;
    }
    string slot = fileName.substr(i + 1, 2);
    int moduleID = std::stoi(slot);
    if (!geo->isModuleIDValid(moduleID)) {
      B2ERROR("Module ID is not valid (incorrectly parsed from file name?): " << moduleID);
      continue;
    }
    B2INFO("--> importing constats for slot " << moduleID);

    string qname = "t0good[" +  std::to_string(moduleID) + "];1";
    TH1D* quality = (TH1D*) file->Get(qname.c_str());
    if (!quality) {
      B2ERROR("Quality histogram '" << qname << "' not found");
      continue;
    }

    string hname = "t0val[" +  std::to_string(moduleID) + "];1";
    TH1D* t0val = (TH1D*) file->Get(hname.c_str());
    if (!t0val) {
      B2ERROR("Histogram '" << hname << "' with calibration constants not found");
      continue;
    }

    int goodChannels = 0;
    for (int pixel = 1; pixel <= 512; pixel++) {
      auto channel = chMapper.getChannel(pixel);
      double err = 50e-3 * quality->GetBinContent(pixel); // 0 for bad constant
      channelT0->setT0(moduleID, channel, t0val->GetBinContent(pixel), err);
      nall++;
      if (err != 0) {goodChannels++; ncal++;}
    }
    file->Close();
    B2INFO("--> number of calibrated channels: " << goodChannels << "/512");
    B2INFO("file closed");
  }

  IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
  channelT0.import(iov);

  B2RESULT("Channel T0 calibration constants imported to database, calibrated channels: "
           << ncal << "/" << nall);

}



void TOPDatabaseImporter::printSampleTimeCalibration()
{

  DBObjPtr<TOPCalTimebase> timeBase;

  for (const auto& sampleTimes : timeBase->getSampleTimes()) {
    cout << sampleTimes.getScrodID() << " " << sampleTimes.getChannel() << endl;
    for (const auto& time : sampleTimes.getTimeAxis()) {
      cout << time << " ";
    }
    cout << endl;
  }

}



void TOPDatabaseImporter::generateFakeChannelMask(double fractionDead, double fractionHot)
{
  // declare db object to be imported -- and construct it
  DBImportObjPtr<TOPCalChannelMask> channelMask;
  channelMask.construct();

  // set up for loop channel maper and random number generator
  auto& chMapper = TOP::TOPGeometryPar::Instance()->getChannelMapper();
  const size_t nModules = TOP::TOPGeometryPar::Instance()->getGeometry()->getNumModules();
  // http://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
  std::random_device rd;
  std::mt19937 gen(rd());  // std generator
  std::uniform_real_distribution<> dist(0, 1);
  unsigned ncall = 0;
  unsigned nall = 0;

  // loop over module (1-based)
  for (size_t moduleID = 1; moduleID <= nModules; moduleID++) {

    // loop over boardStack*carrierBoard*assic*channel to get channel (0 to 512)
    // TODO: get these loop limits from some sensible enum somewhere
    for (int boardStack = 0; boardStack < 4; boardStack++) {
      for (int carrierBoard = 0; carrierBoard < 4; carrierBoard++) {
        for (int asic = 0; asic < 4; asic++) {
          for (int chan = 0; chan < 8; chan++) {
            auto channel = chMapper.getChannel(boardStack, carrierBoard, asic, chan);
            nall++;
            if (dist(gen) < fractionDead) {
              channelMask->setDead(moduleID, channel);
              ncall++;
            }
            if (dist(gen) < fractionHot) {
              channelMask->setNoisy(moduleID, channel);
              ncall++;
            }
          }
        }
      }
    }
  } // module

  // declare interval of validity
  IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
  channelMask.import(iov);

  B2WARNING("Generated and imported a fake channel mask to database for testing: "
            << ncall << "/" << nall);
  return;
}



//---- for testing only -- will be removed --------------------------------

void TOPDatabaseImporter::importTest(int runNumber, double syncTimeBase)
{

  DBImportObjPtr<TOPCalTimebase> timeBase;

  timeBase.construct(syncTimeBase);
  for (unsigned scrodID = 0; scrodID < 64; scrodID++) {
    for (unsigned channel = 0; channel < 128; channel++) {
      timeBase->append(scrodID, channel);
    }
  }

  if (runNumber == 3) {
    timeBase.addEventDependency(10);
    timeBase.construct(syncTimeBase + 100);
    for (unsigned scrodID = 0; scrodID < 64; scrodID++) {
      for (unsigned channel = 0; channel < 128; channel++) {
        timeBase->append(scrodID, channel);
      }
    }
  }

  IntervalOfValidity iov(1, runNumber, 1, runNumber);
  timeBase.import(iov);

}


void TOPDatabaseImporter::importTest()
{

  DBImportArray<TOPPmtGainPar> pmtGains;

  auto* pmtGain = pmtGains.appendNew("JT00123");
  pmtGain->setNominalHV(3520);
  for (unsigned channel = 1; channel <= 16; channel++) {
    pmtGain->setChannelData(channel, -13.77, 0.0042, 0.4);
  }

  pmtGain = pmtGains.appendNew("JT02135");
  pmtGain->setNominalHV(3450);
  for (unsigned channel = 1; channel <= 16; channel++) {
    pmtGain->setChannelData(channel, -12.77, 0.0045, 0.4);
  }

  for (const auto& pmtGain : pmtGains) pmtGain.print();

  //  IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
  //  pmtGains.import(iov);


}
