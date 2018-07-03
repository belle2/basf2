/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty   hearty@physics.ubc.ca               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *                                                                        *
 * Read payloads ECLRefAmpl and ECLRefTime and use them to derive         *
 * payloads ECLCrystalElectronics or ECLCrystalElectronicsTime            *
 *                                                                        *
 * Also performs a comparison of new and existing calibration values and  *
 * writes these to a root file.                                           *
 *                                                                        *
 * Payloads are read from localdb if present, otherwise from              *
 * Calibration_Offline_Development, or Data_Taking_HLT.                   *
 * They are written to localdb with iov = exp,run,-1,-1                   *
 *                                                                        *
 * Usage:                                                                 *
 * eclElectronicsPayloads payloadName exp run [writeToDB]                 *
 * where payloadName = ECLCrystalElectronics or ECLCrystalElectronicsName *
 * exp and run specify the start of the iov, and are used to read         *
 * the reference amplitudes and times                                     *
 * Option argument writeToDB = 0 to not write output to database          *
 *                                                                        *
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/Database.h>
#include <framework/database/LocalDatabase.h>
#include <framework/database/DatabaseChain.h>
#include <framework/database/ConditionsDatabase.h>
#include <framework/logging/LogSystem.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TH1F.h>

using namespace Belle2;

//------------------------------------------------------------------------
//..Set experiment, run, and event numbers before reading a payload from the DB
namespace {
  void setupDatabase(int exp, int run, int eventNr = 1)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    // simulate the initialize() phase where we can register objects in the DataStore
    DataStore::Instance().setInitializeActive(true);
    evtPtr.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);
    std::cout << "about to construct EventMetaData, exp = " << exp << " run = " << run << " eventNr = " << eventNr << std::endl;
    evtPtr.construct(eventNr, run, exp);
    DBStore& dbstore = DBStore::Instance();
    dbstore.update();
    dbstore.updateEvent();
  }
}

//------------------------------------------------------------------------
int main(int argc, char** argv)
{
  if (argc < 4 || argc > 5) {
    std::cout << "insufficient arguments for eclElectronicsPayloads" << std::endl;
    return -1;
  }
  std::string payloadName = argv[1];
  bool findAmpl = false;
  if (payloadName == "ECLCrystalElectronics") {
    findAmpl = true;
  } else if (payloadName == "ECLCrystalElectronicsTime") {
    findAmpl = false;
  } else {
    std::cout << "First argument must be ECLCrystalElectronics or ECLCrystalElectronicsTime" << std::endl;
    return -1;
  }
  int experiment = std::stoi(argv[2]);
  int run = std::stoi(argv[3]);
  bool writeOutput = true;
  if (argc == 5) writeOutput = std::stoi(argv[4]);
  std::cout << "eclElectronicsPayloads called with arguments " << payloadName << " " << experiment << " " << run << " " << writeOutput
            << std::endl;

  //------------------------------------------------------------------------
  //..Specify database
  Database::reset();
  bool resetIovs = false;
  DatabaseChain::createInstance(resetIovs);
  ConditionsDatabase::createDefaultInstance("Data_Taking_HLT", LogConfig::c_Debug);
  ConditionsDatabase::createInstance("Calibration_Offline_Development", "", "", "", LogConfig::c_Debug);
  LocalDatabase::createInstance("localdb/database.txt", "", LogConfig::c_Debug);

  //..set debug level
  LogConfig* logging = LogSystem::Instance().getLogConfig();
  logging->setLogLevel(LogConfig::c_Debug);
  logging->setDebugLevel(10);

  //..Populate database contents
  std::cout << "calling setupDatabase " << std::endl;
  setupDatabase(experiment, run);

  //------------------------------------------------------------------------
  //..Read input and existing output payloads from database
  DBObjPtr<Belle2::ECLCrystalCalib> existingObject(payloadName);
  DBObjPtr<Belle2::ECLCrystalCalib> InputAmpl("ECLRefAmpl");
  DBObjPtr<Belle2::ECLCrystalCalib> InputTime("ECLRefTime");

  //..Print out some information about the existing payload
  std::cout << "Reading ECLRefAmpl, ECLRefTime, and " << payloadName << std::endl;
  existingObject->Dump();

  //..Get vectors of values from the payloads
  std::vector<float> currentValues;
  std::vector<float> currentUnc;
  currentValues = existingObject->getCalibVector();
  currentUnc = existingObject->getCalibUncVector();

  std::vector<float> refAmpl;
  std::vector<float> refAmplUnc;
  refAmpl = InputAmpl->getCalibVector();
  refAmplUnc = InputAmpl->getCalibUncVector();

  std::vector<float> refTime;
  std::vector<float> refTimeUnc;
  refTime = InputTime->getCalibVector();
  refTimeUnc = InputTime->getCalibUncVector();

  //..Print out a few values for quality control
  std::cout << std::endl << "Reference amplitudes and times read from database " << std::endl;
  for (int ic = 0; ic < 9000; ic += 1000) {
    std::cout << "cellID " << ic + 1 << " ref amplitude = " << refAmpl[ic] << " +/- " << refAmplUnc[ic] << " ref time = " << refTime[ic]
              << " +/- " << refTimeUnc[ic] << std::endl;
  }

  //------------------------------------------------------------------------
  //..Calculate the new values for ECLCrystalElectronics or ECLCrystalElectronicsTime
  std::vector<float> newValues;
  std::vector<float> newUnc;
  for (int ic = 0; ic < 8736; ic++) {
    if (findAmpl) {
      newValues.push_back(17750. / refAmpl[ic]);
      newUnc.push_back(newValues[ic]*refAmplUnc[ic] / refAmpl[ic]);
    } else {
      newValues.push_back(refTime[ic]);
      newUnc.push_back(refTimeUnc[ic]);
    }
  }

  //------------------------------------------------------------------------
  //..Compare current values to new ones
  std::cout << std::endl << "Comparison of existing and new values for " << payloadName << std::endl;
  for (int ic = 0; ic < 9000; ic += 1000) {
    std::cout << "cellID " << ic + 1 << " existing = " << currentValues[ic] << " +/- " << currentUnc[ic] << " new = " << newValues[ic]
              << " +/- " << newUnc[ic] << std::endl;
  }

  TString payloadTitle = payloadName;
  payloadTitle += "_";
  payloadTitle += experiment;
  payloadTitle += "_";
  payloadTitle += run;
  TString fname = payloadTitle;
  fname += ".root";
  TFile hfile(fname, "recreate");
  TString htitle = payloadTitle;
  htitle += " existing calibration values;cellID";
  TH1F* existingCalib = new TH1F("existingCalib", htitle, 8736, 1, 8737);
  htitle = payloadTitle;
  htitle += " new calibration values;cellID";
  TH1F* newCalib = new TH1F("newCalib", htitle, 8736, 1, 8737);
  htitle = payloadTitle;
  htitle += " ratio";
  TH1F* calibRatio = new TH1F("calibRatio", htitle, 200, 0.9, 1.1);
  htitle = payloadTitle;
  htitle += " difference";
  TH1F* calibDiff = new TH1F("calibDiff", htitle, 200, -100, 100);

  for (int cellID = 1; cellID <= 8736; cellID++) {
    float oldValue = currentValues[cellID - 1];
    float newValue = newValues[cellID - 1];
    float ratio = 9999.;
    if (oldValue != 0.) {
      ratio = newValue / oldValue;
    } else if (newValue != 0.) {
      ratio = ratio * newValue / fabs(newValue);
    }

    existingCalib->SetBinContent(cellID, oldValue);
    existingCalib->SetBinError(cellID, currentUnc[cellID - 1]);
    newCalib->SetBinContent(cellID, newValue);
    newCalib->SetBinError(cellID, newUnc[cellID - 1]);
    calibRatio->Fill(ratio);
    calibDiff->Fill(newValue - oldValue);
  }

  hfile.cd();
  hfile.Write();
  hfile.Close();
  std::cout << "Comparison of existing and new calibration values written to " << fname << std::endl;

  //------------------------------------------------------------------------
  //..Write out to localdb if requested
  if (writeOutput) {
    std::cout << "Creating importer" << std::endl;
    Belle2::DBImportObjPtr<Belle2::ECLCrystalCalib> importer(payloadName);
    importer.construct();
    importer->setCalibVector(newValues, newUnc);
    importer.import(Belle2::IntervalOfValidity(experiment, run, -1, -1));
    std::cout << "Successfully wrote payload " << payloadName << " with iov " << experiment << "," << run << ",-1,-1" << std::endl;
  }
}
