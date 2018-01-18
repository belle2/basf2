/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <pxd/calibration/PXDDatabaseImporter.h>

// framework - core
#include <framework/core/RandomGenerator.h>

// framework - Database
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportArray.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

// DB objects
//#include <pxd/dbobjects/PXDNoiseCalMap.h>
//#include <pxd/dbobjects/PXDDeadPixelCalMap.h>
//#include <pxd/dbobjects/PXDPedestalCalMap.h>
//#include <pxd/dbobjects/PXDGainCalMap.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <set>


using namespace std;

namespace Belle2 {

  void PXDDatabaseImporter::importPXDNoiseCalibrations(string fName)
  {
    /*
    // Create an empty payload
    PXDNoiseCalMap noiseMap;

    // Set nominal noise level (ADU)
    noiseMap.setNominalNoise(0.7);

    // Read constants from file and put them to DB object
    string fullName = FileSystem::findFile(fName);
    if (fullName == "") {
      B2FATAL("openFile: Can't locate " << fName);
      return;
    }
    ifstream inputfile;
    inputfile.open(fullName.c_str(), ifstream::in);
    if (!inputfile.good()) {
      B2FATAL("openFile: Error opening file " << fName);
      return;
    }
    B2INFO(fName << ": open for reading");

    int i = 0;
    string line;
    while (getline(inputfile, line)) {
      istringstream iss(line);
      float noiseLevel = 0.7;
      string stringID;
      if (!(iss >> stringID >> noiseLevel)) {
        B2ERROR("Error reading noise calibration data from " << fullName.c_str());
        continue;
      }
      // Add calibration to noiseMap
      noiseMap.getNoiseMap()[VxdID(stringID)] = noiseLevel;
      i++;
    }
    inputfile.close();
    B2INFO("file closed");

    // Import constants
    DBImportObjPtr<PXDNoiseCalMap> importObj;
    importObj.construct(noiseMap);
    IntervalOfValidity iov(0, 0, -1, -1);
    importObj.import(iov);

    B2RESULT("PXDNoiseCalibreation payload imported");
    */
  }

} // end Belle2 namespace

