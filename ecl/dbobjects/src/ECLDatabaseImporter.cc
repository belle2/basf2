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
#include <ecl/dbobjects/ECLDatabaseImporter.h>

//STL
#include <sstream>
#include <fstream>
#include <string>

//Boost
#include <boost/filesystem.hpp>

// ECL
#include <ecl/dbobjects/ECLDigitEnergyConstants.h>
#include <ecl/dbobjects/ECLDigitTimeConstants.h>
#include <ecl/modules/eclShowerShape/ECLShowerShapeModule.h>
#include <ecl/dbobjects/ECLShowerShapeSecondMomentCorrection.h>
#include <ecl/dbobjects/ECLShowerCorrectorLeakageCorrection.h>
#include <ecl/dbobjects/ECLShowerEnergyCorrectionTemporary.h>
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
#include <TGraph2D.h>


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

  //----------------------------------------------------------------------------------------------
  //Fill ParameterNtuple vectors
  //----------------------------------------------------------------------------------------------

  int bgFractionBinNum;
  int regNum;
  int phiBinNum;
  int thetaBinNum;
  int energyBinNum;
  float correctionFactor;

  //Set Branch Addresses
  correctionTree->SetBranchAddress(m_bgFractionBinNumBranchName.c_str(), &bgFractionBinNum);
  correctionTree->SetBranchAddress(m_regNumBranchName.c_str(), &regNum);
  correctionTree->SetBranchAddress(m_phiBinNumBranchName.c_str(), &phiBinNum);
  correctionTree->SetBranchAddress(m_thetaBinNumBranchName.c_str(), &thetaBinNum);
  correctionTree->SetBranchAddress(m_energyBinNumBranchName.c_str(), &energyBinNum);
  correctionTree->SetBranchAddress(m_correctionFactorBranchName.c_str(), &correctionFactor);

  //Fill vectors
  std::vector<int> m_bgFractionBinNum;
  std::vector<int> m_regNum;
  std::vector<int> m_phiBinNum;
  std::vector<int> m_thetaBinNum;
  std::vector<int> m_energyBinNum;
  std::vector<float> m_correctionFactor;

  for (long iEntry = 0; iEntry < correctionTree->GetEntries(); ++iEntry) {
    correctionTree->GetEntry(iEntry);

    m_bgFractionBinNum.push_back(bgFractionBinNum);
    m_regNum.push_back(regNum);
    m_phiBinNum.push_back(phiBinNum);
    m_thetaBinNum.push_back(thetaBinNum);
    m_energyBinNum.push_back(energyBinNum);
    m_correctionFactor.push_back(correctionFactor);
  }

  //----------------------------------------------------------------------------------------------
  //Fill ConstantNtuple vectors
  //----------------------------------------------------------------------------------------------


  float lReg1Theta;
  float hReg1Theta;
  float lReg2Theta;
  float hReg2Theta;
  float lReg3Theta;
  float hReg3Theta;
  int numOfBfBins;
  int numOfEnergyBins;
  int numOfPhiBins;
  int numOfReg1ThetaBins;
  int numOfReg2ThetaBins;
  int numOfReg3ThetaBins;
  int phiPeriodicity;

  //Ugly hack to circumvent 'stack usage might be unbounded [-Wstack-usage=]' compiler warning that's caused by the use of c-type arrays.
  //This is not for the faint of heart

  //because root GetEntry fills the whole internal array of the vector without changing it's size, we must ensure that it's the right size.
  std::vector<float> avgRecEns(m_numAvgRecEnEntries);
  helperTree->SetBranchAddress(m_avgRecEnBranchName.c_str(), avgRecEns.data()); //Read c-style array right into internal vector array.

  helperTree->SetBranchAddress(m_lReg1ThetaBranchName.c_str(), &lReg1Theta);
  helperTree->SetBranchAddress(m_hReg1ThetaBranchName.c_str(), &hReg1Theta);
  helperTree->SetBranchAddress(m_lReg2ThetaBranchName.c_str(), &lReg2Theta);
  helperTree->SetBranchAddress(m_hReg2ThetaBranchName.c_str(), &hReg2Theta);
  helperTree->SetBranchAddress(m_lReg3ThetaBranchName.c_str(), &lReg3Theta);
  helperTree->SetBranchAddress(m_hReg3ThetaBranchName.c_str(), &hReg3Theta);
  helperTree->SetBranchAddress(m_numOfBfBinsBranchName.c_str(), &numOfBfBins);
  helperTree->SetBranchAddress(m_numOfEnergyBinsBranchName.c_str(), &numOfEnergyBins);
  helperTree->SetBranchAddress(m_numOfPhiBinsBranchName.c_str(), &numOfPhiBins);
  helperTree->SetBranchAddress(m_numOfReg1ThetaBinsBranchName.c_str(), &numOfReg1ThetaBins);
  helperTree->SetBranchAddress(m_numOfReg2ThetaBinsBranchName.c_str(), &numOfReg2ThetaBins);
  helperTree->SetBranchAddress(m_numOfReg3ThetaBinsBranchName.c_str(), &numOfReg3ThetaBins);
  helperTree->SetBranchAddress(m_phiPeriodicityBranchName.c_str(), &phiPeriodicity);

  //Fill vectors
  std::vector<float> m_avgRecEn;
  std::vector<float> m_lReg1Theta;
  std::vector<float> m_hReg1Theta;
  std::vector<float> m_lReg2Theta;
  std::vector<float> m_hReg2Theta;
  std::vector<float> m_lReg3Theta;
  std::vector<float> m_hReg3Theta;
  std::vector<int>   m_numOfBfBins;
  std::vector<int>   m_numOfEnergyBins;
  std::vector<int>   m_numOfPhiBins;
  std::vector<int>   m_numOfReg1ThetaBins;
  std::vector<int>   m_numOfReg2ThetaBins;
  std::vector<int>   m_numOfReg3ThetaBins;
  std::vector<int>   m_phiPeriodicity;

  for (long iEntry = 0; iEntry < helperTree->GetEntries(); ++iEntry) {
    helperTree->GetEntry(iEntry);
    for (unsigned int iIdx = 0; iIdx < avgRecEns.size(); ++iIdx) m_avgRecEn.push_back(avgRecEns[iIdx]);

    m_lReg1Theta.push_back(lReg1Theta);
    m_hReg1Theta.push_back(hReg1Theta);
    m_lReg2Theta.push_back(lReg2Theta);
    m_hReg2Theta.push_back(hReg2Theta);
    m_lReg3Theta.push_back(lReg3Theta);
    m_hReg3Theta.push_back(hReg3Theta);
    m_numOfBfBins.push_back(numOfBfBins);
    m_numOfEnergyBins.push_back(numOfEnergyBins);
    m_numOfPhiBins.push_back(numOfPhiBins);
    m_numOfReg1ThetaBins.push_back(numOfReg1ThetaBins);
    m_numOfReg2ThetaBins.push_back(numOfReg2ThetaBins);
    m_numOfReg3ThetaBins.push_back(numOfReg3ThetaBins);
    m_phiPeriodicity.push_back(phiPeriodicity);
  }

  //----------------------------------------------------------------------------------------------

  //Construct DB object
  DBImportObjPtr<ECLShowerCorrectorLeakageCorrection> dbPtr("ecl_shower_corrector_leakage_corrections");
  dbPtr.construct(m_bgFractionBinNum,
                  m_regNum,
                  m_phiBinNum,
                  m_thetaBinNum,
                  m_energyBinNum,
                  m_correctionFactor,
                  m_avgRecEn,
                  m_lReg1Theta,
                  m_hReg1Theta,
                  m_lReg2Theta,
                  m_hReg2Theta,
                  m_lReg3Theta,
                  m_hReg3Theta,
                  m_numOfBfBins,
                  m_numOfEnergyBins,
                  m_numOfPhiBins,
                  m_numOfReg1ThetaBins,
                  m_numOfReg2ThetaBins,
                  m_numOfReg3ThetaBins,
                  m_phiPeriodicity);

  //Create IOV object
  int startExp = 0;
  int startRun = 0;
  int endExp = -1;
  int endRun = -1;
  IntervalOfValidity iov(startExp, startRun, endExp, endRun);

  //Import into local db
  dbPtr.import(iov);

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
  dbArray.appendNew(ECLConnectedRegion::c_N1, ECLShowerShapeModule::c_thetaType , *theta_N1_graph);

  //N1 phi
  TGraph* phi_N1_graph = getRootObjectFromFile<TGraph*>(inputFile, "SecondMomentCorrections_phi_N1");
  dbArray.appendNew(ECLConnectedRegion::c_N1, ECLShowerShapeModule::c_phiType , *phi_N1_graph);

  //N2 theta
  TGraph* theta_N2_graph = getRootObjectFromFile<TGraph*>(inputFile, "SecondMomentCorrections_theta_N2");
  dbArray.appendNew(ECLConnectedRegion::c_N2, ECLShowerShapeModule::c_thetaType , *theta_N2_graph);

  //N2 phi
  TGraph* phi_N2_graph = getRootObjectFromFile<TGraph*>(inputFile, "SecondMomentCorrections_phi_N2");
  dbArray.appendNew(ECLConnectedRegion::c_N2, ECLShowerShapeModule::c_phiType , *phi_N2_graph);


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

void ECLDatabaseImporter::importShowerEnergyCorrectionTemporary()
{
  if (m_inputFileNames.size() > 1)
    B2FATAL("Sorry, you must only import one file at a time for now!");

  //Expect a txt file
  boost::filesystem::path path(m_inputFileNames[0]);
  if (path.extension() != ".txt")
    B2FATAL("Expecting a .txt file. Aborting");

  TGraph2D graph;

  std::string line;
  std::ifstream infile(m_inputFileNames[0]);
  double energy;
  double bkgFactor;
  double thetaMinInLine;
  double thetaMaxInLine;
  double correctionFactor;

  double thetaMin = DBL_MAX;
  double thetaMax = -DBL_MAX;
  double energyMin = DBL_MAX;
  double energyMax = -DBL_MAX;

  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    iss >> energy;
    iss >> bkgFactor;
    iss >> thetaMinInLine;
    iss >> thetaMaxInLine;
    iss >> correctionFactor;

    //    B2INFO(energy << " " << bkgFactor << " " << thetaMinInLine << " " << thetaMaxInLine << " " << correctionFactor);

    const double theta = 0.5 * (thetaMinInLine + thetaMaxInLine);

    //Find thetaMin and thetaMax
    if (theta < thetaMin)
      thetaMin = theta;

    if (theta > thetaMax)
      thetaMax = theta;

    //Find energyMin and energyMax
    if (energy < energyMin)
      energyMin = energy;

    if (energy > energyMax)
      energyMax = energy;



    graph.SetPoint(graph.GetN(), theta, energy, correctionFactor);
  }

//  B2INFO(thetaMin << " " << thetaMax << " " << energyMin << " " << energyMax);

//    Import to DB
  int startExp = 0;
  int startRun = 0;
  int endExp = -1;
  int endRun = -1;
  IntervalOfValidity iov(startExp, startRun, endExp, endRun);

  if (std::abs(bkgFactor - 1.0) < 1e-9) { //bkgFactor == 1 -> phase 2 backgrounds
    DBImportObjPtr<ECLShowerEnergyCorrectionTemporary> dbPtr("ECLShowerEnergyCorrectionTemporary_phase2");
    dbPtr.construct(graph, thetaMin, thetaMax, energyMin, energyMax);

    //Import into local db
    dbPtr.import(iov);
  }
  /*else (because currently phase_2 and phase_3 are same payload*/ if (std::abs(bkgFactor - 1.0) < 1e-9) {
    DBImportObjPtr<ECLShowerEnergyCorrectionTemporary> dbPtr("ECLShowerEnergyCorrectionTemporary_phase3");
    dbPtr.construct(graph, thetaMin, thetaMax, energyMin, energyMax);

    //Import into local db
    dbPtr.import(iov);
  }


}
