/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// ECL
#include <ecl/dbobjects/ECLDatabaseImporter.h>
#include <ecl/dbobjects/ECLCalibrationDigit.h>

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

ECLDatabaseImporter::ECLDatabaseImporter(vector<string> inputFileNames, std::string name)
{
  m_inputFileNames.push_back(inputFileNames[0]);
  m_name = name;
}

void ECLDatabaseImporter::importDigitCalibration()
{

  TClonesArray digitCalibrationConstants("Belle2::ECLCalibrationDigit");

  TH1F* energy = 0;
  TH1F* amplitude = 0;
  int nFiles = 0;

  for (const string& inputFile : m_inputFileNames) {

    TFile* f = TFile::Open(inputFile.c_str(), "READ");

    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*) next())) {

      string histconstants = key->GetName();

      if (histconstants.find("energy") == 0) {
        energy = (TH1F*)f->Get(histconstants.c_str());
      } else  if (histconstants.find("amplitude") == 0) {
        amplitude = (TH1F*)f->Get(histconstants.c_str());
      }

      else { B2ERROR("Key name does not match any of the following: energy, amplitude!"); }
    }

    nFiles++;
  }

  if (nFiles != 1) { B2FATAL("Sorry, you must only import one file at a time for now!"); }

  // loop over the histogram to fill the TClonesArray
  int cell = 0;
  for (int bin = 1; bin <= amplitude->GetNbinsX(); ++bin) {
    float amplitudeval = amplitude->GetBinContent(bin);
    float energyval = energy->GetBinContent(bin);
    new(digitCalibrationConstants[cell]) ECLCalibrationDigit(bin, amplitudeval, energyval);
    cell++;
  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData(m_name, &digitCalibrationConstants, iov);
}
