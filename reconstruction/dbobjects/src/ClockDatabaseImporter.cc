/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2016 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Gian Luca Pinna Angioni                                  *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

// Own include
#include <reconstruction/dbobjects/ClockDatabaseImporter.h>


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
#include <framework/dbobjects/Clocks.h> //del


using namespace std;

namespace Belle2 {


  void ClockDatabaseImporter::importClock(int firstExp, int firstRun,
                                          int lastExp, int lastRun)
  {
    IntervalOfValidity iov(firstExp, firstRun, lastExp, lastRun);
    DBImportObjPtr<Clocks> clockbase;
    clockbase.construct();

    clockbase->setGlobalClock(127.222); // MHz
    clockbase->setClock(Const::EDetector::TOP, "sampling", 6);
    clockbase->setClock(Const::EDetector::SVD, "sampling", 4);
    clockbase->setClock(Const::EDetector::ECL, "sampling", 3 * 24);
    clockbase->setClock(Const::EDetector::ECL, "fitting", 3);

    clockbase.import(iov);
    B2INFO("Dummy clock importer");
    return;
  }
}
