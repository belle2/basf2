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
#include <framework/utilities/ClockDatabaseImporter.h>


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
  }

  Const::EDetector  ClockDatabaseImporter::parseDetectors(std::string const val)
  {
    if (val == "svd")        return Const::SVD;
    else if (val == "cdc")   return Const::CDC;
    else if (val == "top")   return Const::TOP;
    else if (val == "arich") return Const::ARICH;
    else if (val == "ecl")   return Const::ECL;
    else if (val == "klm")   return Const::KLM;
    else B2ERROR("Unknown detector component: " << val);
  }

  void ClockDatabaseImporter::setAcceleratorRF(Float_t val)
  {
    clockbase->setAcceleratorRF(val); /** MHz */
  }

  void ClockDatabaseImporter::setClockPrescale(std::string const detector, std::string type, Int_t val)
  {
    clockbase->setClockPrescale(parseDetectors(detector), type, val);
  }
}
