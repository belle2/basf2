/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty   hearty@physics.ubc.ca               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *                                                                        *
 * - Read in payloads energy calibration payloads from specified          *
 *   global tag (default ECL_energy_calibrations) or localdb, if present. *
 * - Combine to create new ECLCrystalEnergy                               *
 * - Write new ECLCrystalEnergy to localdb, with iov exp,run,-1,-1        *
 * - Compare new version to existing version and store histograms to      *
 *   ECLCrystalEnergy_exp_run.root                                        *
 *                                                                        *
 *   Usage: eclCrystalEnergy exp run [globaltag [writeToDB]]              *
 *   Set option argument writeToDB = 0 to not write output to database    *
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
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <iostream>
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
  if (argc < 3 || argc > 5) {
    std::cout << "incorrect number of arguments for eclCrystalEnergy" << std::endl;
    return -1;
  }
  int experiment = std::stoi(argv[1]);
  int run = std::stoi(argv[2]);
  std::string gtName = "ECL_energy_calibrations";
  if (argc >= 4) {gtName = argv[3];}
  bool writeOutput = true;
  if (argc == 5) {writeOutput = std::stoi(argv[4]);}

  std::cout << "eclCrystalEnergy called with arguments exp = " << experiment << " run = " << run << " GT =  " << gtName <<
            " writeOutput = "  << writeOutput << std::endl;


  //------------------------------------------------------------------------
  /** Specify database. Last one specified is first one used */
  Database::reset();
  bool resetIovs = false;
  DatabaseChain::createInstance(resetIovs);
  ConditionsDatabase::createDefaultInstance(gtName, LogConfig::c_Debug);
  LocalDatabase::createInstance("localdb/database.txt", "", LogConfig::c_Debug);

  /** Create the DBObjPtr for the payloads that we want to read from the DB */
  DBObjPtr<Belle2::ECLCrystalCalib> Cosmic("ECLCrystalEnergyCosmic");
  DBObjPtr<Belle2::ECLCrystalCalib> MuMu("ECLCrystalEnergyMuMu");
  DBObjPtr<Belle2::ECLCrystalCalib> GammaGamma("ECLCrystalEnergyGammaGamma");
  DBObjPtr<Belle2::ECLCrystalCalib> Bhabha5x5("ECLCrystalEnergyee5x5");
  DBObjPtr<Belle2::ECLCrystalCalib> Existing("ECLCrystalEnergy");

  /** Populate database contents */
  setupDatabase(experiment, run);

  //------------------------------------------------------------------------
  /** Get the vectors from the input payload */
  std::vector<float> CosmicCalib;
  std::vector<float> CosmicCalibUnc;
  CosmicCalib = Cosmic->getCalibVector();
  CosmicCalibUnc = Cosmic->getCalibUncVector();

  std::vector<float> MuMuCalib;
  std::vector<float> MuMuCalibUnc;
  MuMuCalib = MuMu->getCalibVector();
  MuMuCalibUnc = MuMu->getCalibUncVector();

  std::vector<float> GammaGammaCalib;
  std::vector<float> GammaGammaCalibUnc;
  GammaGammaCalib = GammaGamma->getCalibVector();
  GammaGammaCalibUnc = GammaGamma->getCalibUncVector();

  std::vector<float> Bhabha5x5Calib;
  std::vector<float> Bhabha5x5CalibUnc;
  Bhabha5x5Calib = Bhabha5x5->getCalibVector();
  Bhabha5x5CalibUnc = Bhabha5x5->getCalibUncVector();

  std::vector<float> ExistingCalib;
  std::vector<float> ExistingCalibUnc;
  ExistingCalib = Existing->getCalibVector();
  ExistingCalibUnc = Existing->getCalibUncVector();

  //------------------------------------------------------------------------
  /** Calculate new ECLCrystalEnergy from existing payloads */
  //..For now, use Gamma Gamma if available; otherwise, use existing value
  std::vector<float> NewCalib;
  std::vector<float> NewCalibUnc;
  NewCalib.resize(8736);
  NewCalibUnc.resize(8736);
  for (int ic = 0; ic < 8736; ic++) {
    if (GammaGammaCalib[ic] > 0.) {
      NewCalib[ic] = GammaGammaCalib[ic];
      NewCalibUnc[ic] = GammaGammaCalibUnc[ic];
    } else {
      NewCalib[ic] = ExistingCalib[ic];
      NewCalibUnc[ic] = ExistingCalibUnc[ic];
    }
  }

  //------------------------------------------------------------------------
  /** Write out a few values for quality control purposes */
  for (int ic = 0; ic < 9000; ic += 1000) {
    std::cout << ic + 1 << " " << ExistingCalib[ic] << " " << ExistingCalibUnc[ic] << " " << GammaGammaCalib[ic] << " " <<
              GammaGammaCalibUnc[ic] << " " << NewCalib[ic] << " " << NewCalibUnc[ic] << std::endl;
  }

  //------------------------------------------------------------------------
  //..Histograms of existing calibration, new calibration, and ratio new/old
  TString payloadTitle = "ECLCrystalEnergy";
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

    newPayload->SetBinContent(cellID, NewCalib[cellID - 1]);
    newPayload->SetBinError(cellID, NewCalibUnc[cellID - 1]);

    float ratio = 1.;
    float ratioUnc = 0.;
    if (abs(ExistingCalib[cellID - 1]) > 1.0e-12) {
      ratio = NewCalib[cellID - 1] / ExistingCalib[cellID - 1];
      float rUnc0 = ExistingCalibUnc[cellID - 1] / ExistingCalib[cellID - 1];
      float rUnc1 = 0.;
      if (abs(NewCalib[cellID - 1]) > 1.0e-12) {rUnc1 = NewCalibUnc[cellID - 1] / NewCalib[cellID - 1];}
      ratioUnc = ratio * sqrt(rUnc0 * rUnc0 + rUnc1 * rUnc1);
    }

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
    Belle2::DBImportObjPtr<Belle2::ECLCrystalCalib> importer("ECLCrystalEnergy");
    importer.construct();
    importer->setCalibVector(NewCalib, NewCalibUnc);
    importer.import(Belle2::IntervalOfValidity(experiment, run, -1, -1));
    std::cout << "Successfully wrote payload ECLCrystalEnergy with iov " << experiment << "," << run << ",-1,-1" << std::endl;
  } else {
    std::cout << "Payload not written to database" << std::endl;
  }
}
