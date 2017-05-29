/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// DEDX
#include <reconstruction/dbobjects/DedxDatabaseImporter.h>
#include <reconstruction/dbobjects/CDCDedxWireGain.h>
#include <reconstruction/dbobjects/CDCDedxRunGain.h>
#include <reconstruction/dbobjects/CDCDedxCosine.h>

// FRAMEWORK
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>

// ROOT
#include <TNtuple.h>
#include <TH2.h>
#include <TFile.h>
#include <TKey.h>
#include <string>
#include <vector>
#include <TClonesArray.h>

// NAMESPACES
using namespace Belle2;

DedxDatabaseImporter::DedxDatabaseImporter(std::string inputFileName, std::string name)
{
  m_inputFileNames.push_back(inputFileName);
  m_name = name;
}

void DedxDatabaseImporter::importWireGainCalibration()
{

  TClonesArray wireGainCalibrationConstants("Belle2::CDCDedxWireGain");

  TH2F* gains = 0;
  int nFiles = 0;

  for (const std::string& inputFile : m_inputFileNames) {

    TFile* f = TFile::Open(inputFile.c_str(), "READ");

    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*) next())) {

      std::string histconstants = key->GetName();

      if (histconstants.compare("CDCDedxWireGains") == 0) {
        gains = (TH2F*)f->Get(histconstants.c_str());
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
  for (int bin = 1; bin <= gains->GetNbinsX(); ++bin) {
    int wireid = gains->GetBinContent(bin, 1);
    float gainval = gains->GetBinContent(bin, 2);
    new(wireGainCalibrationConstants[bin - 1]) CDCDedxWireGain(wireid, gainval);
  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData(m_name, &wireGainCalibrationConstants, iov);
}

void DedxDatabaseImporter::importRunGainCalibration()
{

  TClonesArray runGainCalibrationConstants("Belle2::CDCDedxRunGain");

  TH1F* gains = 0;
  int nFiles = 0;

  for (const std::string& inputFile : m_inputFileNames) {

    TFile* f = TFile::Open(inputFile.c_str(), "READ");

    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*) next())) {

      std::string histconstants = key->GetName();

      if (histconstants.compare("runGains") == 0) {
        gains = (TH1F*)f->Get(histconstants.c_str());
      }

      else { B2ERROR("Key name does not match: runGains!"); }
    }

    nFiles++;
  }

  if (nFiles != 1) { B2FATAL("Sorry, you must only import one file at a time for now!"); }

  // fill the TClonesArray

  // Using a TNtuple seems to fail for some reason...
  //  float* args = gains->GetArgs();
  //  int run = floor(args[0]);
  //  float gainval = args[1];

  int run = gains->GetBinContent(1);
  float gainval = gains->GetBinContent(2);
  new(runGainCalibrationConstants[0]) CDCDedxRunGain(run, gainval);

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData(m_name, &runGainCalibrationConstants, iov);
}

void DedxDatabaseImporter::importCosineCalibration()
{

  TClonesArray cosineCalibrationConstants("Belle2::CDCDedxCosine");

  TH1F* means = 0;
  int nFiles = 0;

  for (const std::string& inputFile : m_inputFileNames) {

    TFile* f = TFile::Open(inputFile.c_str(), "READ");

    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*) next())) {

      std::string histconstants = key->GetName();

      if (histconstants.compare("cosine") == 0) {
        means = (TH1F*)f->Get(histconstants.c_str());
      }

      else { B2ERROR("Key name does not match: cosine!"); }
    }

    nFiles++;
  }

  if (nFiles != 1) { B2FATAL("Sorry, you must only import one file at a time for now!"); }

  // fill the TClonesArray
  std::vector<float> costh;
  std::vector<float> meanvals;
  for (int bin = 1; bin <= means->GetNbinsX(); ++bin) {
    costh.push_back(means->GetBinCenter(bin));
    meanvals.push_back(means->GetBinContent(bin));
  }
  int nbins = costh.size();
  new(cosineCalibrationConstants[0]) CDCDedxCosine(nbins, costh, meanvals);

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData(m_name, &cosineCalibrationConstants, iov);
}
