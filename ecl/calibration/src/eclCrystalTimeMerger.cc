/******************************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                                     *
 * Copyright(C) 2019 - Belle II Collaboration                                             *
 *                                                                                        *
 * Author: The Belle II Collaboration                                                     *
 * Contributors: Christopher Hearty   hearty@physics.ubc.ca                               *
 *                                                                                        *
 * This software is provided "as is" without any warranty.                                *
 *                                                                                        *
 * - Read in payloads energy calibration payloads from specified                          *
 *   global tag (default ECL_energy_calibrations) or localdb, if present.                 *
 * - Combine to create new ECLCrystalEnergy                                               *
 * - Write new ECLCrystalEnergy to localdb, with iov exp,run,-1,-1                        *
 * - Compare new version to existing version and store histograms to                      *
 *   ECLCrystalEnergy_exp_run.root                                                        *
 *                                                                                        *
 *   Usage: eclCrystalTimeMerger exp run [globaltag [[writeToDB] [timeShiftInTicks]]]     *
 *   Set option argument writeToDB = 0 to not write output to database                    *
 *                                                                                        *
 *****************************************************************************************/

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


int main(int argc, char** argv)
{
  //------------------------------------------------------------------------
  //..Check the arguments
  if (argc < 4 || argc > 6) {
    std::cout << "incorrect number of arguments for eclCrystalEnergy" << std::endl;
    return -1;
  }
  int experiment = std::stoi(argv[1]);
  int run = std::stoi(argv[2]);
  std::string gtName = "data_reprocessing_proc8";
  if (argc >= 4) {gtName = argv[3];}
  bool writeOutput = true;
  double timeShiftInTicks = 0 ;
  if (argc == 5) {writeOutput = std::stoi(argv[4]);}
  if (argc == 6) {
    writeOutput = std::stoi(argv[4]);
    timeShiftInTicks = std::stod(argv[5]);
  }

  std::cout << "eclCrystalEnergy called with arguments exp = " << experiment
            << ", run = " << run << ", GT =  " << gtName
            << ", writeOutput = "  << writeOutput
            << ", timeShiftInTicks = "  << timeShiftInTicks << std::endl;


  //------------------------------------------------------------------------
  /** Specify database. Last one specified is first one used */
  Database::reset();
  bool resetIovs = false;
  DatabaseChain::createInstance(resetIovs);
  ConditionsDatabase::createDefaultInstance(gtName, LogConfig::c_Debug);
  LocalDatabase::createInstance("localdb_insideCDCPayloadAndOutsideCDCPayload/database.txt", "", LogConfig::c_Debug);

  /** Create the DBObjPtr for the payloads that we want to read from the DB */
  DBObjPtr<Belle2::ECLCrystalCalib> Existing("ECLCrystalTimeOffsetBhabhaGamma");
  DBObjPtr<Belle2::ECLCrystalCalib> Bhabha("ECLCrystalTimeOffset");

  /** Populate database contents */
  setupDatabase(experiment, run);

  //------------------------------------------------------------------------
  /** Get the vectors from the input payload */
  std::vector<float> BhabhaCalib;
  std::vector<float> BhabhaCalibUnc;
  BhabhaCalib = Bhabha->getCalibVector();
  BhabhaCalibUnc = Bhabha->getCalibUncVector();

  std::vector<float> ExistingCalib;
  std::vector<float> ExistingCalibUnc;
  ExistingCalib = Existing->getCalibVector();
  ExistingCalibUnc = Existing->getCalibUncVector();

  //------------------------------------------------------------------------
  /** Calculate new ECLCrystalTimeOffset from existing payloads */
  //..For now, use Gamma Gamma if available; otherwise, use existing value
  std::vector<float> NewCalib;
  std::vector<float> NewCalibUnc;
  NewCalib.resize(8736);
  NewCalibUnc.resize(8736);

  std::cout <<
            "cid    ExistingCalib[ic]   ExistingCalibUnc[ic]     BhabhaCalib[ic]   BhabhaCalibUnc[ic]     NewCalib[ic]    NewCalibUnc[ic]\n" ;

  for (int ic = 0; ic < 8736; ic++) {
    std::cout << ic + 1 << " " << ExistingCalib[ic] << " " << ExistingCalibUnc[ic] << " " << BhabhaCalib[ic] << " " <<
              BhabhaCalibUnc[ic] << " " ;

    /* If there is no new bhabha calibration constant, use the cosmic ones.
       If looking at crystals near the edge of the CDC acceptance and
       if the bhabha calibration constant uncertainty is larger than 2 ticks (0.4931 ticks/ns * 2 ticks = 0.9862 ns), then
       use the cosmic ones.
    */
    if ((BhabhaCalib[ic] == 0)    ||
        ((BhabhaCalibUnc[ic] > 2) &&
         ((ic <= 288) || (ic >= 8545)))) {
      NewCalib[ic] = ExistingCalib[ic];
      NewCalibUnc[ic] = ExistingCalibUnc[ic];

      std::cout << NewCalib[ic] << " " << NewCalibUnc[ic] << " ticks, not ns !! ---- using existing calib, not new calib" <<  std::endl;


    } else {
      NewCalib[ic] = BhabhaCalib[ic] ;
      NewCalibUnc[ic] = BhabhaCalibUnc[ic] ;
      std::cout << NewCalib[ic] << " " << NewCalibUnc[ic] << " ticks, not ns !!" <<  std::endl;

    }

  }

  //------------------------------------------------------------------------
  /** Write out a few values for quality control purposes */
  for (int ic = 0; ic < 8736; ic += 100) {
    std::cout << ic + 1 << " " << ExistingCalib[ic] << " " << ExistingCalibUnc[ic] << " " << BhabhaCalib[ic] << " " <<
              BhabhaCalibUnc[ic] << " " << NewCalib[ic] << " " << NewCalibUnc[ic] << std::endl;
  }

  //------------------------------------------------------------------------
  //..Histograms of existing calibration, new calibration, and ratio new/old
  TString payloadTitle = "ECLCrystalTimeOffset";
  payloadTitle += "_";
  payloadTitle += experiment;
  payloadTitle += "_";
  payloadTitle += run;
  TString fname = payloadTitle;
  fname += ".root";
  TFile hfile(fname, "recreate");

  TString htitle = payloadTitle;
  htitle += " existing values;cellID";
  TH1F* existingPayload = new TH1F("existingPayload", htitle, 8736, 1, 8737);

  htitle = payloadTitle;
  htitle += " newer bhabha values;cellID";
  TH1F* bhabhaPayload = new TH1F("newerBhabhaPayload", htitle, 8736, 1, 8737);

  htitle = payloadTitle;
  htitle += " new values;cellID";
  TH1F* newPayload = new TH1F("newPayload", htitle, 8736, 1, 8737);

  htitle = payloadTitle;
  htitle += " ratio new/old;cellID";
  TH1F* payloadRatioVsCellID = new TH1F("payloadRatioVsCellID", htitle, 8736, 1, 8737);

  htitle = payloadTitle;
  htitle += " ratio new/old";
  TH1F* payloadRatio = new TH1F("payloadRatio", htitle, 200, 0.95, 1.05);

  for (int cellID = 1; cellID <= 8736; cellID++) {
    existingPayload->SetBinContent(cellID, ExistingCalib[cellID - 1]);
    existingPayload->SetBinError(cellID, ExistingCalibUnc[cellID - 1]);

    bhabhaPayload->SetBinContent(cellID, BhabhaCalib[cellID - 1]);
    bhabhaPayload->SetBinError(cellID, BhabhaCalibUnc[cellID - 1]);


    if (timeShiftInTicks != 0) {
      NewCalib[cellID - 1] += timeShiftInTicks ;
      std::cout << "Hacking calib to be +" << timeShiftInTicks << " in TICKS for hadronic events !!!!!!!!!!!!!\n" ;
    } else {
      std::cout << "NO offset being applied to crystal times during merger\n" ;
    }

    newPayload->SetBinContent(cellID, NewCalib[cellID - 1]);
    newPayload->SetBinError(cellID, NewCalibUnc[cellID - 1]);

    float ratio = NewCalib[cellID - 1] / ExistingCalib[cellID - 1];
    float rUnc0 = ExistingCalibUnc[cellID - 1] / ExistingCalib[cellID - 1];
    float rUnc1 = NewCalibUnc[cellID - 1] / NewCalib[cellID - 1];
    float ratioUnc = ratio * sqrt(rUnc0 * rUnc0 + rUnc1 * rUnc1);

    payloadRatioVsCellID->SetBinContent(cellID, ratio);
    payloadRatioVsCellID->SetBinError(cellID, ratioUnc);

    payloadRatio->Fill(ratio);
  }

  hfile.cd();
  hfile.Write();
  hfile.Close();
  std::cout << std::endl << "Values written to " << fname << std::endl;

  //------------------------------------------------------------------------
  //..Write new payload to localdb if requested
  if (writeOutput) {
    std::cout << "Creating importer" << std::endl;
    Belle2::DBImportObjPtr<Belle2::ECLCrystalCalib> importer("ECLCrystalTimeOffset");
    importer.construct();
    importer->setCalibVector(NewCalib, NewCalibUnc);
    importer.import(Belle2::IntervalOfValidity(experiment, run, -1, -1));
    std::cout << "Successfully wrote payload ECLCrystalEnergy with iov " << experiment << "," << run << ",-1,-1" << std::endl;
  } else {
    std::cout << "Payload not written to database" << std::endl;
  }
}

