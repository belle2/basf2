/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/Configuration.h>
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
    std::cout << "incorrect number of arguments for eclDumpECLCrystalCalib" << std::endl;
    return -1;
  }
  std::string payloadName = argv[1];
  std::string gtName = argv[2];
  int experiment = std::stoi(argv[3]);
  int run = std::stoi(argv[4]);
  std::cout << "eclDumpECLCrystalCalib called with arguments " << payloadName << " " << gtName << " " << experiment << " " << run <<
            " " <<
            std::endl;

  //------------------------------------------------------------------------
  //..Specify database
  auto& conf = Conditions::Configuration::getInstance();
  conf.prependGlobalTag(gtName);
  conf.prependTestingPayloadLocation("localdb/database.txt");

  //..Populate database contents
  std::cout << "calling setupDatabase " << std::endl;
  setupDatabase(experiment, run);

  //------------------------------------------------------------------------
  //..Read payloads from database
  DBObjPtr<Belle2::ECLCrystalCalib> existingObject(payloadName);
  std::cout << "Dumping " << payloadName << std::endl;
  existingObject->Dump();

  //..Get vectors of values from the payloads
  std::vector<float> currentValues = existingObject->getCalibVector();
  std::vector<float> currentUnc = existingObject->getCalibUncVector();

  //..Print out a few values for quality control
  std::cout << std::endl << "Values read from database " << std::endl;
  for (int ic = 0; ic < 9000; ic += 1000) {
    std::cout << "cellID " << ic + 1 << " " << currentValues[ic] << " +/- " << currentUnc[ic] << std::endl;
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
  htitle += " values;cellID";
  TH1F* existingPayload = new TH1F("existingPayload", htitle, 8736, 1, 8737);

  for (int cellID = 1; cellID <= 8736; cellID++) {
    existingPayload->SetBinContent(cellID, currentValues[cellID - 1]);
    existingPayload->SetBinError(cellID, currentUnc[cellID - 1]);
  }

  hfile.cd();
  hfile.Write();
  hfile.Close();
  std::cout << std::endl << "Values written to " << fname << std::endl;
}
