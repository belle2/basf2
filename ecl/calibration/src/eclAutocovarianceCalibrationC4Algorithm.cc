/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/calibration/eclAutocovarianceCalibrationC4Algorithm.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

#include <framework/database/DBImportObjPtr.h>
#include <ecl/dbobjects/ECLAutoCovariance.h>

/* ROOT headers. */
#include <TFile.h>
#include <TGraph.h>
#include <TH2.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

/**-----------------------------------------------------------------------------------------------*/
eclAutocovarianceCalibrationC4Algorithm::eclAutocovarianceCalibrationC4Algorithm():
  CalibrationAlgorithm("eclAutocovarianceCalibrationC4Collector")
{
  setDescription(
    "Perform validation of coveriance matrix calibration"
  );
}

CalibrationAlgorithm::EResult eclAutocovarianceCalibrationC4Algorithm::calibrate()
{


  ///** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();

  int m_CountLimit = 1000;

  ///**-----------------------------------------------------------------------------------------------*/
  ///** Histograms containing the data collected by eclAutocovarianceCalibrationC4Collector */
  auto Chi2VsCrysID = getObjectPtr<TH2F>("Chi2VsCrysID");

  std::vector<double> cryIDs;
  std::vector<double> means;
  std::vector<double> RMSs;
  std::vector<double> counts;

  for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {

    TH1F* hChi2 = (TH1F*)Chi2VsCrysID->ProjectionY("hChi2", crysID + 1, crysID + 1);

    int totalCounts = hChi2->GetEntries();

    if (totalCounts < m_CountLimit) {
      B2INFO("eclAutocovarianceCalibrationC4Algorithm: Warning Below Count Limit: crysID totalCounts m_CountLimit: " << crysID << " " <<
             totalCounts << " " << m_CountLimit);
    }

    float mean = hChi2->GetMean();

    cryIDs.push_back(crysID + 1);
    means.push_back(mean);
    RMSs.push_back(hChi2->GetRMS());
    counts.push_back(totalCounts);

  }



  auto gMeanVsCrysID = new TGraph(cryIDs.size(), cryIDs.data(), means.data());
  gMeanVsCrysID->SetName("gMeanVsCrysID");
  gMeanVsCrysID->SetMarkerStyle(20);
  auto gRMSVsCrysID = new TGraph(cryIDs.size(), cryIDs.data(), RMSs.data());
  gRMSVsCrysID->SetName("gRMSVsCrysID");
  gRMSVsCrysID->SetMarkerStyle(20);
  auto gCountsVsCrysID = new TGraph(cryIDs.size(), cryIDs.data(), counts.data());
  gCountsVsCrysID->SetName("gCountsVsCrysID");
  gCountsVsCrysID->SetMarkerStyle(20);

  /** Write out the basic histograms in all cases */
  TString fName = m_outputName;
  TFile* histfile = new TFile(fName, "recreate");
  histfile->cd();
  gMeanVsCrysID->Write();
  gRMSVsCrysID->Write();
  gCountsVsCrysID->Write();
  Chi2VsCrysID->Write();

  return c_OK;
}
