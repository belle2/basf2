/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// DEDX
#include <reconstruction/dbobjects/CDCDedxDatabaseImporter.h>
#include <reconstruction/dbobjects/CDCDedxScaleFactor.h>
#include <reconstruction/dbobjects/CDCDedxHadronCor.h>
#include <reconstruction/dbobjects/CDCDedxMeanPars.h>
#include <reconstruction/dbobjects/CDCDedxSigmaPars.h>
#include <reconstruction/dbobjects/DedxPDFs.h>

// FRAMEWORK
#include <framework/logging/Logger.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>

// ROOT
#include <TFile.h>
#include <TH1F.h>
#include <TKey.h>
#include <string>
#include <vector>
#include <TClonesArray.h>

// NAMESPACES
using namespace Belle2;

CDCDedxDatabaseImporter::CDCDedxDatabaseImporter(std::string inputFileName, const std::string& name)
{
  m_inputFileNames.push_back(inputFileName);
  m_name = name;
}

void CDCDedxDatabaseImporter::importPDFs()
{
  TClonesArray dedxPDFs("Belle2::DedxPDFs");
  TFile* infile = TFile::Open(m_inputFileNames[0].c_str(), "READ");
  new (dedxPDFs[0]) DedxPDFs(infile);

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData(m_name, dedxPDFs[0], iov);
}

void CDCDedxDatabaseImporter::importScaleFactor(double scale)
{

  TClonesArray scaleFactor("Belle2::CDCDedxScaleFactor");
  new (scaleFactor[0]) CDCDedxScaleFactor(scale);

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData(m_name, scaleFactor[0], iov);
}

void CDCDedxDatabaseImporter::importHadronCorrection()
{

  TClonesArray hadronCorrection("Belle2::CDCDedxHadronCor");

  TH1F* parhist = 0;
  int nFiles = 0;

  for (const std::string& inputFile : m_inputFileNames) {

    TFile* f = TFile::Open(inputFile.c_str(), "READ");

    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*) next())) {

      std::string histconstants = key->GetName();

      if (histconstants.compare("CDCDedxHadronCor") == 0) {
        parhist = (TH1F*)f->Get(histconstants.c_str());
        B2INFO("Key name matches: " << histconstants);
      }

      else {
        B2WARNING("Key name does not match: " << histconstants);
        continue;
      }
    }

    nFiles++;
  }

  if (nFiles != 1) { B2FATAL("Sorry, you must only import one file at a time for now!"); }

  // loop over the histogram to fill the TClonesArray
  if (!parhist) B2FATAL("None of the input files contains the histogram called 'CDCDedxHadronCor'!");
  short version = parhist->GetBinContent(1);
  std::vector<double> hadroncor;
  for (int bin = 2; bin <= parhist->GetNbinsX(); ++bin) {
    hadroncor.push_back(parhist->GetBinContent(bin));
  }
  new (hadronCorrection[0]) CDCDedxHadronCor(version, hadroncor);

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData(m_name, hadronCorrection[0], iov);
}

void CDCDedxDatabaseImporter::importMeanParameters()
{

  TClonesArray meanParameters("Belle2::CDCDedxMeanPars");

  TH1F* parhist = 0;
  int nFiles = 0;

  for (const std::string& inputFile : m_inputFileNames) {

    TFile* f = TFile::Open(inputFile.c_str(), "READ");

    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*) next())) {

      std::string histconstants = key->GetName();

      if (histconstants.compare("CDCDedxMeanPars") == 0) {
        parhist = (TH1F*)f->Get(histconstants.c_str());
        B2INFO("Key name matches: " << histconstants);
      }

      else {
        B2WARNING("Key name does not match: " << histconstants);
        continue;
      }
    }

    nFiles++;
  }

  if (nFiles != 1) { B2FATAL("Sorry, you must only import one file at a time for now!"); }

  // loop over the histogram to fill the TClonesArray
  if (!parhist) B2FATAL("None of the input files contains the histogram called 'CDCDedxMeanPars'!");
  short version = parhist->GetBinContent(1);
  std::vector<double> meanpars;
  for (int bin = 2; bin <= parhist->GetNbinsX(); ++bin) {
    meanpars.push_back(parhist->GetBinContent(bin));
  }
  new (meanParameters[0]) CDCDedxMeanPars(version, meanpars);

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData(m_name, meanParameters[0], iov);
}

void CDCDedxDatabaseImporter::importSigmaParameters()
{

  TClonesArray sigmaParameters("Belle2::CDCDedxSigmaPars");

  TH1F* parhist = 0;
  int nFiles = 0;

  for (const std::string& inputFile : m_inputFileNames) {

    TFile* f = TFile::Open(inputFile.c_str(), "READ");

    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*) next())) {

      std::string histconstants = key->GetName();

      if (histconstants.compare("CDCDedxSigmaPars") == 0) {
        parhist = (TH1F*)f->Get(histconstants.c_str());
        B2INFO("Key name matches: " << histconstants);
      }

      else {
        B2WARNING("Key name does not match: " << histconstants);
        continue;
      }
    }

    nFiles++;
  }

  if (nFiles != 1) { B2FATAL("Sorry, you must only import one file at a time for now!"); }

  // loop over the histogram to fill the TClonesArray
  if (!parhist) B2FATAL("None of the input files contains the histogram called 'CDCDedxSigmaPars'!");
  short version = parhist->GetBinContent(1);
  std::vector<double> sigmapars;
  for (int bin = 2; bin <= parhist->GetNbinsX(); ++bin) {
    sigmapars.push_back(parhist->GetBinContent(bin));
  }
  new (sigmaParameters[0]) CDCDedxSigmaPars(version, sigmapars);

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData(m_name, sigmaParameters[0], iov);
}
