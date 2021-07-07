/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/Configuration.h>
#include <framework/logging/LogSystem.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <iostream>
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
  if (payloadName != "ECLCrystalElectronics" and payloadName != "ECLCrystalElectronicsTime" and payloadName != "ECLRefAmplNom"
      and payloadName != "ECLRefTimeNom") {
    std::cout << "First argument must be ECLCrystalElectronics, ECLCrystalElectronicsTime, ECLRefAmplNom, or ECLRefTimeNom" <<
              std::endl;
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
  auto& conf = Conditions::Configuration::getInstance();
  auto states = conf.getUsableTagStates();
  states.insert("OPEN");
  conf.setUsableTagStates(states);
  conf.prependGlobalTag("ECL_localrun_data");
  conf.prependTestingPayloadLocation("localdb/database.txt");

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
  DBObjPtr<Belle2::ECLCrystalCalib> InputAmplNom("ECLRefAmplNom");
  DBObjPtr<Belle2::ECLCrystalCalib> InputTime("ECLRefTime");
  DBObjPtr<Belle2::ECLCrystalCalib> InputTimeNom("ECLRefTimeNom");
  DBObjPtr<Belle2::ECLCrystalCalib> CurrentElec("ECLCrystalElectronics");
  DBObjPtr<Belle2::ECLCrystalCalib> CurrentTime("ECLCrystalElectronicsTime");

  //..Print out some information about the existing payload
  std::cout << "Reading ECLRefAmpl, ECLRefAmplNom, ECLRefTime, ECLRefTimeNom, ECLCrystalElectronics, and ECLCrystalElectronicsTime" <<
            std::endl;
  std::cout << "Dumping " << payloadName << std::endl;
  existingObject->Dump();

  //..Get vectors of values from the payloads
  std::vector<float> currentValues = existingObject->getCalibVector();
  std::vector<float> currentUnc = existingObject->getCalibUncVector();

  std::vector<float> refAmpl = InputAmpl->getCalibVector();
  std::vector<float> refAmplUnc = InputAmpl->getCalibUncVector();

  std::vector<float> refAmplNom = InputAmplNom->getCalibVector();

  std::vector<float> refTime = InputTime->getCalibVector();
  std::vector<float> refTimeUnc = InputTime->getCalibUncVector();

  std::vector<float> refTimeNom = InputTimeNom->getCalibVector();

  std::vector<float> crysElec = CurrentElec->getCalibVector();

  std::vector<float> crysTime = CurrentTime->getCalibVector();


  //..Print out a few values for quality control
  std::cout << std::endl << "Reference amplitudes and times read from database " << std::endl;
  for (int ic = 0; ic < 9000; ic += 1000) {
    std::cout << "cellID " << ic + 1 << " ref amplitude = " << refAmpl[ic] << " +/- " << refAmplUnc[ic] << " nom = " << refAmplNom[ic]
              << " ref time = " << refTime[ic]
              << " +/- " << refTimeUnc[ic] << " nom = " << refTimeNom[ic] << std::endl;
  }

  //------------------------------------------------------------------------
  //..Calculate the new values for requested payload
  std::vector<float> newValues;
  std::vector<float> newUnc;
  for (int ic = 0; ic < 8736; ic++) {
    if (payloadName == "ECLCrystalElectronics") {
      newValues.push_back(refAmplNom[ic] / refAmpl[ic]);
      newUnc.push_back(newValues[ic]*refAmplUnc[ic] / refAmpl[ic]);
    } else if (payloadName == "ECLCrystalElectronicsTime") {
      newValues.push_back(refTime[ic] - refTimeNom[ic]);
      newUnc.push_back(refTimeUnc[ic]);
    } else if (payloadName == "ECLRefAmplNom") {
      newValues.push_back(crysElec[ic]*refAmpl[ic]);
      newUnc.push_back(0.);
    } else if (payloadName == "ECLRefTimeNom") {
      newValues.push_back(refTime[ic] - crysTime[ic]);
      newUnc.push_back(0.);
    }

  }

  //------------------------------------------------------------------------
  //..Compare current values to new ones
  std::cout << std::endl << "Comparison of existing and new values for " << payloadName << std::endl;
  for (int ic = 0; ic < 9000; ic += 1000) {
    std::cout << "cellID " << ic + 1 << " existing = " << currentValues[ic] << " +/- " << currentUnc[ic] << " new = " << newValues[ic]
              << " +/- " << newUnc[ic] << std::endl;
  }
  std::cout << std::endl;

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

  htitle = payloadTitle;
  htitle += " reference";
  TH1F* refValues = new TH1F("refValues", htitle, 8736, 1, 8737);

  htitle = payloadTitle;
  htitle += " ratio vs cellID;cellID;new/old";
  TH1F* ratioVsCellID = new TH1F("ratioVsCellID", htitle, 8736, 1, 8737);

  htitle = payloadTitle;
  htitle += " diff vs cellID;cellID;new - old";
  TH1F* diffVsCellID = new TH1F("diffVsCellID", htitle, 8736, 1, 8737);

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
    ratioVsCellID->SetBinContent(cellID, ratio);
    ratioVsCellID->SetBinError(cellID, 0);
    calibDiff->Fill(newValue - oldValue);
    diffVsCellID->SetBinContent(cellID, newValue - oldValue);
    diffVsCellID->SetBinError(cellID, 0);
    if (payloadName == "ECLCrystalElectronics" or payloadName == "ECLRefAmplNom") {
      refValues->SetBinContent(cellID, refAmpl[cellID - 1]);
      refValues->SetBinError(cellID, refAmplUnc[cellID - 1]);
    } else {
      refValues->SetBinContent(cellID, refTime[cellID - 1]);
      refValues->SetBinError(cellID, refTimeUnc[cellID - 1]);
    }

    //..Note any large changes
    if ((payloadName == "ECLCrystalElectronics" or payloadName == "ECLRefAmplNom") and (ratio<0.99 or ratio>1.01)) {
      std::cout << "Ratio = " << ratio << " for cellID = " << cellID << " refAmpl = " << refAmpl[cellID - 1] << " refAmplNom = " <<
                refAmplNom[cellID - 1] << std::endl;
    } else if (abs(newValue - oldValue) > 20.) {
      std::cout << "Difference = " << newValue - oldValue << " for cellID = " << cellID << " refTime = " << refTime[cellID - 1] <<
                " refTimeNom = " << refTimeNom[cellID - 1] << std::endl;
    }
  }

  hfile.cd();
  hfile.Write();
  hfile.Close();
  std::cout << std::endl << "Comparison of existing and new calibration values written to " << fname << std::endl;

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
