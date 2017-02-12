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
#include <top/dbobjects/TOPPmtGainPar.h>

#include <iostream>
#include <fstream>
#include <sstream>

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
    std::ifstream stream;
    stream.open(fileName.c_str());
    if (!stream) {
      B2ERROR("openFile: " << fileName << " *** failed to open");
      continue;
    }
    B2INFO(fileName << ": opened for reading");

    int numChannels = 0;
    stream >> numChannels;
    if (!stream.good()) {
      B2ERROR("Error reading sample time calibration constants: numChannels");
      stream.close();
      continue;
    }
    for (int ich = 0; ich < numChannels; ich++) {
      int moduleID = 0;
      unsigned ch = 0; // ASIC channel
      unsigned as = 0; // as = ASIC + carrier * 4 + BS * 16
      int numSamples = 0;
      stream >> moduleID >> ch >> as >> numSamples;
      if (!stream.good()) {
        B2ERROR("Error reading sample time calibration constants: moduleID");
        stream.close();
        break;
      }

      std::vector<double> sampleTimes;
      for (int isamp = 0; isamp < numSamples; isamp++) {
        int iter = 0;
        int samp = 0;
        double data = 0;
        stream >> iter >> samp >> data;
        if (!stream.good()) {
          B2ERROR("Error reading sample time calibration constants: samples");
          stream.close();
          break;
        }
        sampleTimes.push_back(data);
      }

      if (sampleTimes.size() == 257) {
        double correction = 2 * syncTimeBase / sampleTimes[256];
        sampleTimes.pop_back();
        for (auto& t : sampleTimes) t *= correction;
      }
      if (sampleTimes.size() != 256) {
        B2ERROR("Error: invalid vector size " << sampleTimes.size());
        continue;
      }

      auto boardStack = as / 16;
      auto* femap = feMapper.getMap(moduleID, boardStack);
      if (!femap) {
        B2ERROR("No FrontEnd map available for boardstack " << boardStack <<
                " of module " << moduleID);
        continue;
      }
      auto scrodID = femap->getScrodID();
      auto channel = as * 8 + ch;
      timeBase->append(scrodID, channel % 128, sampleTimes);

    }
    stream.close();
    B2INFO(fileName << ": closed");
  }

  int ndataRead = timeBase->getSampleTimes().size() * 256;
  B2INFO(ndataRead << " constants read from files");

  IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
  timeBase.import(iov);

  B2RESULT("Sample time calibration constants imported to database");

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
