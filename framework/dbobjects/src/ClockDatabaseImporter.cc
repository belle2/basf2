/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2020 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Gian Luca Pinna Angioni                                  *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

// Own include
#include <framework/dbobjects/ClockDatabaseImporter.h>


// framework - core
#include <framework/core/RandomGenerator.h> // gRandom

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
#include <framework/dbobjects/Clocks.h>


using namespace std;

namespace Belle2 {

  ClockDatabaseImporter::ClockDatabaseImporter()
  {
    clockbase.construct();
  }

  void ClockDatabaseImporter::importClock(int firstExp, int firstRun,
                                          int lastExp, int lastRun)
  {
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    clockbase.import(iov);
    B2INFO("Clock importer");
    return;
  }


  Const::EDetector  ClockDatabaseImporter::parseDetectors(const std::string val)
  {
    Const::EDetector result;

    if (val == "svd")        result = Const::SVD;
    else if (val == "cdc")   result = Const::CDC;
    else if (val == "top")   result = Const::TOP;
    else if (val == "arich") result = Const::ARICH;
    else if (val == "ecl")   result = Const::ECL;
    else if (val == "klm")   result = Const::KLM;
    else B2ERROR("Unknown detector component: " << val);

    return result;
  }

  void ClockDatabaseImporter::setGlobalClock(Float_t val)
  {
    clockbase->setGlobalClock(val); // MHz
  }

  void ClockDatabaseImporter::setClock(string detector, string type, Int_t val)
  {
    clockbase->setClock(parseDetectors(detector), type, val);
  }
}