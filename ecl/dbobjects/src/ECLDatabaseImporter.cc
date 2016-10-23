/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber   (ferber@physics.ubc.ca)                  *
 *               Alon Hershehorn (hershen@phas.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// ECL
#include <ecl/dbobjects/ECLDatabaseImporter.h>
#include <ecl/dbobjects/ECLDigitEnergyConstants.h>
#include <ecl/dbobjects/ECLDigitTimeConstants.h>
#include <ecl/modules/eclShowerShape/ECLShowerShapeModule.h>
#include <ecl/dbobjects/ECLShowerShapeSecondMomentCorrection.h>
#include <ecl/dbobjects/ECLShowerCorrectorLeakageCorrection.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>

// FRAMEWORK
#include <framework/gearbox/GearDir.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/DBImportArray.h>
#include <framework/database/DBImportObjPtr.h>

// ROOT
#include <TH1.h>
#include <TKey.h>
#include <string>
#include <TClonesArray.h>
#include <TTree.h>
#include <TDirectory.h>


// NAMESPACES
using namespace std;
using namespace Belle2;

ECLDatabaseImporter::ECLDatabaseImporter(vector<string> inputFileNames, std::string name)
{
  //input file names
  for (auto& inputFileName : inputFileNames)
    m_inputFileNames.push_back(inputFileName);

  //output file name
  m_name = name;
}

void ECLDatabaseImporter::importDigitEnergyCalibration()
{

  TClonesArray digitCalibrationConstants("Belle2::ECLDigitEnergyConstants");

  TH1F* energy = 0;
  TH1F* amplitude = 0;
  int nFiles = 0;

  for (const string& inputFile : m_inputFileNames) {

    TFile* f = TFile::Open(inputFile.c_str(), "READ");

    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*) next())) {

      string histconstants = key->GetName();

      if (histconstants.compare("energy") == 0) {
        energy = (TH1F*)f->Get(histconstants.c_str());
      } else  if (histconstants.compare("amplitude") == 0) {
        amplitude = (TH1F*)f->Get(histconstants.c_str());
      }

      else { B2FATAL("Key name does not match any of the following: energy, amplitude!"); }
    }

    nFiles++;
  }

  if (nFiles != 1) { B2FATAL("Sorry, you must only import one file at a time for now!"); }

  // loop over the histogram to fill the TClonesArray
  int cell = 0;
  for (int bin = 1; bin <= amplitude->GetNbinsX(); ++bin) {
    float amplitudeval = amplitude->GetBinContent(bin);
    float energyval = energy->GetBinContent(bin);
    new(digitCalibrationConstants[cell]) ECLDigitEnergyConstants(bin, amplitudeval, energyval);
    cell++;
  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData(m_name, &digitCalibrationConstants, iov);
}

void ECLDatabaseImporter::importDigitTimeCalibration()
{

  TClonesArray digitCalibrationConstants("Belle2::ECLDigitTimeConstants");

  TH1F* offset = 0;
  int nFiles = 0;

  for (const string& inputFile : m_inputFileNames) {

    TFile* f = TFile::Open(inputFile.c_str(), "READ");

    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*) next())) {

      string histconstants = key->GetName();

      if (histconstants.compare("constantB") == 0) {
        offset = (TH1F*)f->Get(histconstants.c_str());
      } else { B2FATAL("Key name does not match any of the following: constantC!"); }
    }

    nFiles++;
  }

  if (nFiles != 1) { B2FATAL("Sorry, you must only import one file at a time for now!"); }

  // loop over the histogram to fill the TClonesArray
  int cell = 0;
  for (int bin = 1; bin <= offset->GetNbinsX(); ++bin) {
    float offsetval = offset->GetBinContent(bin);
    new(digitCalibrationConstants[cell]) ECLDigitTimeConstants(bin, offsetval);
    cell++;
  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData(m_name, &digitCalibrationConstants, iov);
}

void ECLDatabaseImporter::importShowerCorrectorLeakageCorrections()
{
  if (m_inputFileNames.size() > 1)
    B2FATAL("Sorry, you must only import one file at a time for now!");

  //Open file
  TFile* inputFile = new TFile(m_inputFileNames[0].data(), "READ");

  if (!inputFile || inputFile->IsZombie())
    B2FATAL("Could not open file " << m_inputFileNames[0]);

  //Get trees
  TTree* correctionTree = getRootObjectFromFile<TTree*>(inputFile, "ParameterNtuple");
  TTree* helperTree = getRootObjectFromFile<TTree*>(inputFile, "ConstantNtuple");
  B2WARNING("line = " << __LINE__);
  //Construct DB object
  DBImportObjPtr<ECLShowerCorrectorLeakageCorrection> dbPtr("ecl_shower_corrector_leakage_corrections");
  B2WARNING("line = " << __LINE__);
  TDirectory* directory = new TDirectory();
  if (correctionTree) correctionTree->SetDirectory(directory);
  if (helperTree) helperTree->SetDirectory(directory);
  dbPtr.construct(directory, helperTree, correctionTree);
  B2WARNING("line = " << __LINE__);

  //Create IOV object
  int startExp = 0;
  int startRun = 0;
  int endExp = -1;
  int endRun = -1;
  IntervalOfValidity iov(startExp, startRun, endExp, endRun);
  B2WARNING("line = " << __LINE__);
  //Import into local db
  dbPtr.import(iov);
  B2WARNING("line = " << __LINE__);
  delete inputFile;

}

void ECLDatabaseImporter::importShowerShapesSecondMomentCorrections()
{
  if (m_inputFileNames.size() > 1)
    B2FATAL("Sorry, you must only import one file at a time for now!");

  //Extract TGraphs from file
  DBImportArray<ECLShowerShapeSecondMomentCorrection> dbArray("ecl_shower_shape_second_moment_corrections");
  TFile* inputFile = new TFile(m_inputFileNames[0].data(), "READ");

  if (!inputFile || inputFile->IsZombie())
    B2FATAL("Could not open file " << m_inputFileNames[0]);

  //N1 theta
  TGraph* theta_N1_graph = getRootObjectFromFile<TGraph*>(inputFile, "SecondMomentCorrections_theta_N1");
  dbArray.appendNew(ECLConnectedRegion::c_N1, ECL::ECLShowerShapeModule::c_thetaType , *theta_N1_graph);

  //N1 phi
  TGraph* phi_N1_graph = getRootObjectFromFile<TGraph*>(inputFile, "SecondMomentCorrections_phi_N1");
  dbArray.appendNew(ECLConnectedRegion::c_N1, ECL::ECLShowerShapeModule::c_phiType , *phi_N1_graph);

  //N2 theta
  TGraph* theta_N2_graph = getRootObjectFromFile<TGraph*>(inputFile, "SecondMomentCorrections_theta_N2");
  dbArray.appendNew(ECLConnectedRegion::c_N2, ECL::ECLShowerShapeModule::c_thetaType , *theta_N2_graph);

  //N2 phi
  TGraph* phi_N2_graph = getRootObjectFromFile<TGraph*>(inputFile, "SecondMomentCorrections_phi_N2");
  dbArray.appendNew(ECLConnectedRegion::c_N2, ECL::ECLShowerShapeModule::c_phiType , *phi_N2_graph);


  //Import to DB
  int startExp = 0;
  int startRun = 0;
  int endExp = -1;
  int endRun = -1;
  IntervalOfValidity iov(startExp, startRun, endExp, endRun);

  //Import into local db
  dbArray.import(iov);

  delete inputFile;
}
