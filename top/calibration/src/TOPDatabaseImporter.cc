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

#include <iostream>
#include <fstream>

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
