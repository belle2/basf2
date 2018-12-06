/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty   hearty@physics.ubc.ca               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/LogSystem.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TH1F.h>

using namespace Belle2;

/** Set experiment, run, and event numbers before reading a payload from the DB */
namespace {
  void setupDatabase(int exp, int run, int eventNr = 1)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    // simulate the initialize() phase where we can register objects in the DataStore
    DataStore::Instance().setInitializeActive(true);
    evtPtr.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);
    // now construct the event metadata
    evtPtr.construct(eventNr, run, exp);
    // and update the database contents
    DBStore& dbstore = DBStore::Instance();
    dbstore.update();
    // this is only needed it the payload might be intra-run dependent,
    // that is if it might change during one run as well
    dbstore.updateEvent();
  }
}


int main()
{

  /** Manually create a set of calibration constants for ECL crystals, class ECLCrystalCalib
   * Usage:
   *    - edit as needed, including specifying payload name and interval of validity
   *    - scons ecl
   *    - execute (eclWriteLocalDB) in appropriate working directory
   *    - payload is written to localdb subdirectory, and added to localdb/database.txt
   *    - you can then add payload to development global tag if desired:
   *        cd localdb
   *        conditionsdb upload development ./database.txt
   */

  std::vector<float> tempCalib;
  std::vector<float> tempCalibUnc;

  /** Sample code to fill payload contents with a constant value */
  /*
   float calib = 0.;
   float calibUnc = 0.;
   for (int cellID = 1; cellID <= 8736; cellID++) {
   tempCalib.push_back(calib);
   tempCalibUnc.push_back(calibUnc);
   } */

  /** Sample code to read text file of values */
  /* std::ifstream calibFile("refref1.txt");
   float cellIDf, amplitude, toff;
   for(int cellID=1; cellID<=8736; cellID++) {
   calibFile >> cellIDf >> amplitude >> toff;
   if(cellID<=1152) {
   calib = 1.;
   } else {
   calib = 17800./amplitude;
   }
   calibUnc = 0.;
   tempCalib.push_back(calib);
   tempCalibUnc.push_back(calibUnc);
   if(cellID%100==0) {std::cout << cellID << " ampl = " << amplitude << " calib = " << calib << " " << calibUnc << std::endl;}
   } */

  /** Sample code to read in a histogram */
  /* TFile f("eclCosmicEAlgorithm.root");
   TH1F *CalibvsCrys = (TH1F*)f.Get("CalibvsCrys");
   for(int cellID=1; cellID<=8736; cellID++) {
   if(cellID<1153) {
   calib = 4.06902e-05;
   calibUnc = 0.;
   } else {
   calib = CalibvsCrys->GetBinContent(cellID);
   calibUnc = CalibvsCrys->GetBinError(cellID);
   }
   tempCalib.push_back(calib);
   tempCalibUnc.push_back(calibUnc);
   if(cellID%100==0) {std::cout << "cellID = " << cellID << " calib = " << calib << " +/- " << calibUnc << std::endl; }
   } */

  /** Sample code to read vectors from an existing payload
   Set the desired global tag via environmental variable
   export BELLE2_CONDB_GLOBALTAG="development" */

  /** set debug level so that we see which databases are created */
  LogConfig* logging = LogSystem::Instance().getLogConfig();
  logging->setLogLevel(LogConfig::c_Debug);
  logging->setDebugLevel(10);

  /** Create the DBObjPtr for the payload that we want to read from the DB */
  DBObjPtr<Belle2::ECLCrystalCalib> InputCalib("ECLCrystalEnergy");

  /** Populate database contents */
  int experiment = 1002;
  int run = 1;
  setupDatabase(experiment, run);

  /** Print out some information about the input payload */
  InputCalib->Dump();

  /** Get the vectors from the input payload */
  std::vector<float> calibIn;
  std::vector<float> calibInUnc;
  calibIn = InputCalib->getCalibVector();
  calibInUnc = InputCalib->getCalibUncVector();

  /** Do something with them */
  tempCalib = calibIn;
  tempCalibUnc = calibInUnc;

  /** Write out a few values for quality control purposes */
  for (int ic = 0; ic < 9000; ic += 1000) {
    std::cout << "crysID " << ic << " calib = " << tempCalib[ic] << " +/- " << tempCalibUnc[ic] << std::endl;
  }

  //..Write out to localdb
  std::cout << "Creating importer" << std::endl;
  Belle2::DBImportObjPtr<Belle2::ECLCrystalCalib> importer("ECLCrystalEnergyGammaGamma");
  importer.construct();
  importer->setCalibVector(tempCalib, tempCalibUnc);
  importer.import(Belle2::IntervalOfValidity(0, 0, -1, -1));
  std::cout << "Successfully wrote payload ECLCrystalEnergyGammaGamma" << std::endl;
}
