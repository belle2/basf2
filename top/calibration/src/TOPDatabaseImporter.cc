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

// framework - Database
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>

// framework - xml
#include <framework/gearbox/GearDir.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// db objects
#include <top/dbobjects/TOPSampleTime.h>

// root
#include <TClonesArray.h>

#include <iostream>
#include <fstream>

using namespace std;
using namespace Belle2;

void TOPDatabaseImporter::importSampleTimeCalibration(std::string fileName)
{

  if (!m_topgp->isInitialized()) {
    B2ERROR("TOPGeometryPar was not initialized");
    return;
  }

  auto& mapper = m_topgp->getChannelMapper();
  auto syncTimeBase = m_topgp->getSyncTimeBase();

  std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  TClonesArray constants("Belle2::TOPSampleTime");
  int index = 0;

  for (int moduleID = 3; moduleID < 5; moduleID++) {
    for (int boardStack = 0; boardStack < 4; boardStack++) {
      for (int carrierBoard = 0; carrierBoard < 4; carrierBoard++) {
        for (int asic = 0; asic < 4; asic++) {
          for (int chan = 0; chan < 8; chan++) {
            auto channel = mapper.getChannel(boardStack, carrierBoard, asic, chan);
            new(constants[index]) TOPSampleTime(moduleID, channel, syncTimeBase);
            auto* topSampleTimes = static_cast<TOPSampleTime*>(constants[index]);
            index++;
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
            topSampleTimes->setTimeAxis(sampleTimes, syncTimeBase);
          }
        }
      }
    }
  }

  stream.close();
  int ndata = index * 256;
  B2INFO(fileName << ": file closed, " << ndata << " constants read from file");

  const auto name = DBStore::arrayName<TOPSampleTime>("");
  IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
  Database::Instance().storeData(name, &constants, iov);

  B2RESULT("Sample time calibration constants imported to database");

}

