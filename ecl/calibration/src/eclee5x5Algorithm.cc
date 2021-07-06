/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ecl/calibration/eclee5x5Algorithm.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

#include "TH2F.h"
#include "TFile.h"
#include "TF1.h"
#include "TROOT.h"
#include "TMatrixD.h"
#include "TMatrixDSym.h"
#include "TDecompLU.h"

using namespace std;
using namespace Belle2;
using namespace ECL;

/**-----------------------------------------------------------------------------------------------*/
eclee5x5Algorithm::eclee5x5Algorithm(): CalibrationAlgorithm("eclee5x5Collector")
{
  setDescription(
    "Perform energy calibration of ecl crystals by analyzing energy in 25-crystal sums from Bhabha events"
  );
}

CalibrationAlgorithm::EResult eclee5x5Algorithm::calibrate()
{
  /**-----------------------------------------------------------------------------------------------*/
  /** Clean up existing histograms if necessary */
  TH1F* dummy;
  dummy = (TH1F*)gROOT->FindObject("AverageExpECrys");
  if (dummy) {delete dummy;}
  dummy = (TH1F*)gROOT->FindObject("AverageElecCalib");
  if (dummy) {delete dummy;}
  dummy = (TH1F*)gROOT->FindObject("AverageInitCalib");
  if (dummy) {delete dummy;}
  dummy = (TH1F*)gROOT->FindObject("meanEnvsCrysID");
  if (dummy) {delete dummy;}

  /** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();

  /**-----------------------------------------------------------------------------------------------*/
  /** Write out the job parameters */
  B2INFO("eclee5x5Algorithm parameters:");
  B2INFO("outputName = " << m_outputName);
  B2INFO("minEntries = " << m_minEntries);
  B2INFO("payloadName = " << m_payloadName);
  B2INFO("storeConst = " << m_storeConst);
  if (m_payloadName == "ECLeedPhiData" or m_payloadName == "ECLeedPhiMC" or m_payloadName == "None") {
    B2INFO("fracLo = " << m_fracLo);
    B2INFO("fracHiSym = " << m_fracHiSym);
    B2INFO("fracHiASym = " << m_fracHiASym);
    B2INFO("nsigLo = " << m_nsigLo);
    B2INFO("nsigHiSym = " << m_nsigHiSym);
    B2INFO("nsigHiASym = " << m_nsigHiASym);
  }

  /**-----------------------------------------------------------------------------------------------*/
  /** Histograms containing the data collected by eclee5x5CollectorModule */
  auto EnVsCrysID = getObjectPtr<TH2F>("EnVsCrysID");
  auto RvsCrysID = getObjectPtr<TH1F>("RvsCrysID");
  auto NRvsCrysID = getObjectPtr<TH1F>("NRvsCrysID");
  auto Qmatrix = getObjectPtr<TH2F>("Qmatrix");
  auto ElecCalibvsCrys = getObjectPtr<TH1F>("ElecCalibvsCrys");
  auto ExpEvsCrys = getObjectPtr<TH1F>("ExpEvsCrys");
  auto InitialCalibvsCrys = getObjectPtr<TH1F>("InitialCalibvsCrys");
  auto CalibEntriesvsCrys = getObjectPtr<TH1F>("CalibEntriesvsCrys");
  auto EntriesvsCrys = getObjectPtr<TH1F>("EntriesvsCrys");
  auto dPhivsThetaID = getObjectPtr<TH2F>("dPhivsThetaID");

  /**-----------------------------------------------------------------------------------------------*/
  /** Calculate the average expected energy per crystal and calibration constants from Collector,
   and mean normalized energy */
  TH1F* AverageExpECrys = new TH1F("AverageExpECrys", "Average expected E per crys from collector;Crystal ID;Energy (GeV)", 8736, 0,
                                   8736);
  TH1F* AverageElecCalib = new TH1F("AverageElecCalib", "Average electronics calib const vs crystal;Crystal ID;Calibration constant",
                                    8736, 0, 8736);
  TH1F* AverageInitCalib = new TH1F("AverageInitCalib", "Average initial calib const vs crystal;Crystal ID;Calibration constant",
                                    8736, 0, 8736);
  TH1F* meanEnvsCrysID = new TH1F("meanEnvsCrysID", "Mean normalized energy vs crystal;CrystalID;E/Eexp", 8736, 0, 8736);

  for (int cellID = 1; cellID <= 8736; cellID++) {
    double TotEntries = CalibEntriesvsCrys->GetBinContent(cellID);
    if (TotEntries > 0.) {
      AverageElecCalib->SetBinContent(cellID, ElecCalibvsCrys->GetBinContent(cellID) / TotEntries);
      AverageExpECrys->SetBinContent(cellID, ExpEvsCrys->GetBinContent(cellID) / TotEntries);
      AverageInitCalib->SetBinContent(cellID, InitialCalibvsCrys->GetBinContent(cellID) / TotEntries);
    }

    TH1D* En = EnVsCrysID->ProjectionY("En", cellID, cellID);
    meanEnvsCrysID->SetBinContent(cellID, En->GetMean());
    meanEnvsCrysID->SetBinError(cellID, En->GetStdDev());
  }

  /**-----------------------------------------------------------------------------------------------*/
  /** Write out the basic histograms in all cases */
  TString fName = m_outputName;
  TFile* histfile = new TFile(fName, "recreate");
  EnVsCrysID->Write();
  RvsCrysID->Write();
  NRvsCrysID->Write();
  Qmatrix->Write();
  AverageElecCalib->Write();
  AverageExpECrys->Write();
  AverageInitCalib->Write();
  EntriesvsCrys->Write();
  dPhivsThetaID->Write();
  meanEnvsCrysID->Write();

  /**-----------------------------------------------------------------------------------------------*/
  /** If we have not been asked to do fits, we can quit now */
  if (m_payloadName == "None") {
    B2INFO("eclee5x5Algorithm has not been asked to find constants; copying input histograms and quitting");
    histfile->Close();
    return c_NotEnoughData;
  }

  /**-----------------------------------------------------------------------------------------------*/
  /** Check that all crystals to be calibrated have enough statistics */
  for (int cellID = 1; cellID <= 8736; cellID++) {

    /** Only crystals with initial calib>0 are going to be calibrated */
    if (AverageInitCalib->GetBinContent(cellID) > 0.) {
      if (EntriesvsCrys->GetBinContent(cellID) < m_minEntries) {
        histfile->Close();
        B2INFO("eclee5x5Algorithm: insufficient data for cellID = " << cellID << " " << EntriesvsCrys->GetBinContent(cellID) << " entries");
        return c_NotEnoughData;
      }
    }
  }


  /**-----------------------------------------------------------------------------------------------*/
  /** need crystal per ring for the dPhi payloads */
  m_eclNeighbours5x5 = new ECL::ECLNeighbours("N", 2);


  /**-----------------------------------------------------------------------------------------------*/
  /** Ready to find new calibration constants */
  bool foundConst = false;
  TH1F* gVsCrysID = new TH1F("gVsCrysID", "Ratio of new to old calibration vs crystal ID;crystal ID;vector g", 8736, 0, 8736);
  TH1F* CalibVsCrysID = new TH1F("CalibVsCrysID", "Calibration constant vs crystal ID;crystal ID;counts per GeV", 8736, 0, 8736);
  TH1F* ExpEnergyperCrys = new TH1F("ExpEnergyperCrys", "Expected energy per crystal;Crystal ID;Energy in 25 crystal sum (GeV)", 8736,
                                    0, 8736);
  TString title = "dPhi cut per crystal " + m_payloadName + ";Crystal ID;dPhi requirement (deg)";
  TH1F* dPhiperCrys = new TH1F("dPhiperCrys", title, 8736, 0, 8736);

  /**-----------------------------------------------------------------------------------------------*/
  /** Expected energy payload */
  if (m_payloadName == "ECLExpee5x5E") {
    for (int cellID = 1; cellID <= 8736; cellID++) {
      float mean = meanEnvsCrysID->GetBinContent(cellID);
      float stdDev = meanEnvsCrysID->GetBinError(cellID);
      float inputE = AverageExpECrys->GetBinContent(cellID);
      if (mean > 0. and stdDev > 0.) {
        ExpEnergyperCrys->SetBinContent(cellID, mean * abs(inputE));
        ExpEnergyperCrys->SetBinError(cellID, stdDev * abs(inputE));
      } else {
        ExpEnergyperCrys->SetBinContent(cellID, inputE);
        ExpEnergyperCrys->SetBinError(cellID, 0.05 * inputE);
      }
    }

    //..Generate the payload, if requested
    foundConst = true;
    if (m_storeConst) {
      std::vector<float> tempCalib;
      std::vector<float> tempCalibStdDev;

      for (int cellID = 1; cellID <= 8736; cellID++) {
        tempCalib.push_back(ExpEnergyperCrys->GetBinContent(cellID));
        tempCalibStdDev.push_back(ExpEnergyperCrys->GetBinError(cellID));
      }
      ECLCrystalCalib* ExpectedE = new ECLCrystalCalib();
      ExpectedE->setCalibVector(tempCalib, tempCalibStdDev);
      saveCalibration(ExpectedE, "ECLExpee5x5E");
      B2INFO("eclCosmicEAlgorithm: successfully stored expected energies ECLExpee5x5E");
    }

    /**-----------------------------------------------------------------------------------------------*/
    /** Single crystal energy calibration from matrix inversion */
  } else if (m_payloadName == "ECLCrystalEnergy5x5") {

    //..Create the Q matrix and the R vector
    TMatrixDSym matrixQ(8736);
    TVectorD vectorR(8736);
    for (int ix = 1; ix <= 8736; ix++) {
      vectorR[ix - 1] = RvsCrysID->GetBinContent(ix);
      for (int iy = 1; iy <= 8736; iy++) {
        matrixQ[ix - 1][iy - 1] = Qmatrix->GetBinContent(ix, iy);
      }
    }

    //..Crystals that are not being calibrated have no entries in NR. Adjust R and Q to get g=-1
    int nNotCalibrated = 0;
    for (int cellID = 1; cellID <= 8736; cellID++) {
      if (NRvsCrysID->GetBinContent(cellID) < 0.5) {
        for (int othercell = 1; othercell <= 8736; othercell++) {
          matrixQ[cellID - 1][othercell - 1] = 0.;
          matrixQ[othercell - 1][cellID - 1] = 0.;
        }
        matrixQ[cellID - 1][cellID - 1] = 1.;
        vectorR[cellID - 1] = -1.;
        nNotCalibrated++;
      }
    }
    B2INFO("eclCosmicEAlgorithm: " << nNotCalibrated << " crystals will not be calibrated. ");

    //..Invert to solve Q g = R
    TDecompLU lu(matrixQ);
    bool solved;
    TVectorD vectorg = lu.Solve(vectorR, solved);

    //..Fill histograms and check that there are no unexpected negative output values
    if (solved) {
      foundConst = true;
      for (int cellID = 1; cellID <= 8736; cellID++) {
        gVsCrysID->SetBinContent(cellID, vectorg[cellID - 1]);
        gVsCrysID->SetBinError(cellID, 0.);
        float newCalib = vectorg[cellID - 1] * abs(AverageInitCalib->GetBinContent(cellID));
        CalibVsCrysID->SetBinContent(cellID, newCalib);
        CalibVsCrysID->SetBinError(cellID, 0.);

        if (vectorg[cellID - 1] < 0. and NRvsCrysID->GetBinContent(cellID) > 0.) {foundConst = false;}
      }
    }

    //..Generate the payload if requested, and if the matrix inversion worked
    if (m_storeConst and foundConst) {
      std::vector<float> tempCalib;
      std::vector<float> tempCalibStdDev;

      for (int cellID = 1; cellID <= 8736; cellID++) {
        tempCalib.push_back(CalibVsCrysID->GetBinContent(cellID));
        tempCalibStdDev.push_back(CalibVsCrysID->GetBinError(cellID));
      }
      ECLCrystalCalib* e5x5ECalib = new ECLCrystalCalib();
      e5x5ECalib->setCalibVector(tempCalib, tempCalibStdDev);
      saveCalibration(e5x5ECalib, "ECLCrystalEnergyee5x5");
      B2INFO("eclCosmicEAlgorithm: successfully stored calibration ECLCrystalEnergyee5x5");
    }

    /**-----------------------------------------------------------------------------------------------*/
    /** Obtain new values for dPhi* selection, either data or mc */
  } else if (m_payloadName == "ECLeedPhiData" or m_payloadName == "ECLeedPhiMC") {

    //..Find mean and sigma of Gaussian fit to ThetaID projections with sufficient statistics
    float dPhiCenter[69] = {};
    float dPhiHalfWidth[69] = {};
    int firstID = 0; /** first thetaID with enough statistics */
    int lastID = 0; /** last thetaID with enough statistics */
    const int nbins = dPhivsThetaID->GetNbinsX();
    for (int ib = 1; ib <= nbins; ib++) {
      TH1D* proj = dPhivsThetaID->ProjectionY("proj", ib, ib);
      if (proj->Integral() < 100) {continue;}
      int thetaID = (int)dPhivsThetaID->GetXaxis()->GetBinCenter(ib);
      if (firstID == 0) { firstID = thetaID; }
      lastID = thetaID;

      //..Find the fit limits
      double fracHi = m_fracHiSym;
      double nsigHi = m_nsigHiSym;
      if (thetaID <= m_lastLoThetaID) {
        fracHi = m_fracHiASym;
        nsigHi = m_nsigHiASym;
      }

      //..Fit range includes all bins with entries>m_fracLo*peak on the low side of the
      //  peak, and entries>m_fracHi*peak on the high side.
      //  i.e. low edge of bin on low side, high edge on high side = low edge of bin+1
      double peak = proj->GetMaximum();
      int nPhiBins = proj->GetNbinsX();
      int binLo = 1;
      do  {
        binLo++;
      } while (proj->GetBinContent(binLo) < m_fracLo * peak and binLo < nPhiBins);
      double xfitLo = proj->GetBinLowEdge(binLo);

      //..Start search for the upper edge of the fit range at 175 deg to avoid gamma gamma
      // and e gamma peaks
      int binHi = proj->GetXaxis()->FindBin(175.01);
      do {
        binHi--;
      } while (proj->GetBinContent(binHi)<fracHi* peak and binHi>1);
      double xfitHi = proj->GetBinLowEdge(binHi + 1);

      //..Check that the fit region is sensible
      int peakBin = proj->GetMaximumBin();
      if (binLo >= peakBin or binHi <= peakBin) {
        B2ERROR("Flawed dPhi fit range for thetaID = " << thetaID << " peakBin = " << peakBin << " binLo = " << binLo << "binHi = " <<
                binHi);
      }

      //..Now fit a Gaussian to the selected region
      proj->Fit("gaus", "", "", xfitLo, xfitHi);

      //..Find mean, sigma, and selection range. Record center and half-width of range.
      TF1* fitGaus = proj->GetFunction("gaus");
      float mean = fitGaus->GetParameter(1);
      float sigma = fitGaus->GetParameter(2);
      float dPhiLo = mean - m_nsigLo * sigma;
      float dPhiHi = mean + nsigHi * sigma;
      dPhiCenter[thetaID] = 0.5 * (dPhiLo + dPhiHi); /** not equal to mean at low thetaID */
      dPhiHalfWidth[thetaID] = 0.5 * (dPhiHi - dPhiLo);
    }

    //..Pad the thetaID's without fits with the first or last thetaID values
    for (int thetaID = 0; thetaID < firstID; thetaID++) {
      dPhiCenter[thetaID] = dPhiCenter[firstID];
      dPhiHalfWidth[thetaID] = dPhiHalfWidth[firstID];
    }
    for (int thetaID = lastID + 1; thetaID < 69; thetaID++) {
      dPhiCenter[thetaID] = dPhiCenter[lastID];
      dPhiHalfWidth[thetaID] = dPhiHalfWidth[lastID];
    }

    //..Now copy these to each crystal to generate the payload and fill the output histogram.
    //  We will use ECLNeighours to get the number of crystals in each theta ring
    m_eclNeighbours5x5 = new ECL::ECLNeighbours("N", 2);
    std::vector<float> tempCalib;
    std::vector<float> tempCalibWidth;
    tempCalib.resize(8736);
    tempCalibWidth.resize(8736);
    int crysID = 0;
    for (int thetaID = 0; thetaID < 69; thetaID++) {
      for (int ic = 0; ic < m_eclNeighbours5x5->getCrystalsPerRing(thetaID); ic++) {
        tempCalib.at(crysID) = dPhiCenter[thetaID];
        tempCalibWidth.at(crysID) = dPhiHalfWidth[thetaID];
        dPhiperCrys->SetBinContent(crysID + 1, dPhiCenter[thetaID]);
        dPhiperCrys->SetBinError(crysID + 1, dPhiHalfWidth[thetaID]);
        crysID++;
      }
    }

    //..Store the payload, if requested
    foundConst = true;
    if (m_storeConst) {
      ECLCrystalCalib* eedPhi = new ECLCrystalCalib();
      eedPhi->setCalibVector(tempCalib, tempCalibWidth);
      saveCalibration(eedPhi, m_payloadName);
      B2INFO("eclCosmicEAlgorithm: successfully stored calibration " << m_payloadName);
    }

    /**-----------------------------------------------------------------------------------------------*/
    /** Invalid payload specified */
  } else {
    B2ERROR("eclee5x5Algorithm: invalid payload name: m_payloadName = " << m_payloadName);
  }

  /**-----------------------------------------------------------------------------------------------*/
  /** Write out appropriate histograms, then delete in case algorithm is called again */
  histfile->cd();
  if (m_payloadName == "ECLExpee5x5E") {
    ExpEnergyperCrys->Write();
  } else if (m_payloadName == "ECLCrystalEnergy5x5") {
    gVsCrysID->Write();
    CalibVsCrysID->Write();
  } else if (m_payloadName == "ECLeedPhiData" or m_payloadName == "ECLeedPhiMC") {
    dPhiperCrys->Write();
  }
  histfile->Close();

  dummy = (TH1F*)gROOT->FindObject("gVsCrysID"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("CalibVsCrysID"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("ExpEnergyperCrys"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("dPhiperCrys"); delete dummy;

  /**-----------------------------------------------------------------------------------------------*/
  /** Set the return code appropriately */
  if (!m_storeConst) {
    if (foundConst) {
      B2INFO("eclee5x5Algorithm successfully found constants but was not asked to store them");
    } else {
      B2INFO("eclee5x5Algorithm was not asked to store constants, and did not succeed in finding them");
    }
    return c_Failure;
  } else if (!foundConst) {
    if (m_payloadName == "ECLExpee5x5E") {
      B2INFO("eclee5x5Algorithm: failed to store expected values");
    } else if (m_payloadName == "ECLCrystalEnergy5x5") {
      B2INFO("eclee5x5Algorithm: failed to store calibration constants");
    } else if (m_payloadName == "ECLeedPhiData" or m_payloadName == "ECLeedPhiMC") {
      B2INFO("eclee5x5Algorithm: failed to find dPhi* selection criteria");
    }
    return c_Failure;
  }
  return c_OK;
}
