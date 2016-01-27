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
#include <reconstruction/dbobjects/DedxCalibrationWire.h>

// FRAMEWORK
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>

// ROOT
#include <TH1.h>
#include <TFile.h>
#include <TKey.h>
#include <string>
#include <TClonesArray.h>

// NAMESPACES
using namespace std;
using namespace Belle2;

DedxDatabaseImporter::DedxDatabaseImporter(vector<string> inputFileNames, std::string name)
{
  m_inputFileNames.push_back(inputFileNames[0]);
  m_name = name;
}

void DedxDatabaseImporter::importWireGainCalibration()
{

  TClonesArray wireGainCalibrationConstants("Belle2::DedxCalibrationWire");

  TH1F* gain = 0;
  int nFiles = 0;

  for (const string& inputFile : m_inputFileNames) {

    TFile* f = TFile::Open(inputFile.c_str(), "READ");

    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*) next())) {

      string histconstants = key->GetName();

      if (histconstants.find("gain") == 0) {
        gain = (TH1F*)f->Get(histconstants.c_str());
      }

      else { B2ERROR("Key name does not match: gain!"); }
    }

    nFiles++;
  }

  if (nFiles != 1) { B2FATAL("Sorry, you must only import one file at a time for now!"); }

  // loop over the histogram to fill the TClonesArray
  int wire = 0;
  for (int bin = 1; bin <= gain->GetNbinsX(); ++bin) {
    float gainval = gain->GetBinContent(bin);
    new(wireGainCalibrationConstants[wire]) DedxCalibrationWire(bin, gainval);
    wire++;
  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData(m_name, &wireGainCalibrationConstants, iov);
}
