/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/calibration/eclMuMuEAlgorithm.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

/* ROOT headers. */
#include <TF1.h>
#include <TFile.h>
#include <TH2F.h>
#include <TMath.h>
#include <TROOT.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

/** Novosibirsk function, plus constant H. Ikeda et al., Nuclear Instruments and Methods A 441 (2000) 401-426 */
// cppcheck-suppress constParameter ; TF1 fit functions cannot have const parameters
double eclNovoConst(double* x, double* par)
{
  double qc = 0;

  double peak = par[1];
  double width = par[2];
  double sln4 = sqrt(log(4));
  double y = par[3] * sln4;
  double tail = -log(y + sqrt(1 + y * y)) / sln4;

  if (TMath::Abs(tail) < 1.e-7) {
    qc = 0.5 * TMath::Power(((x[0] - peak) / width), 2);
  } else {
    double qa = tail * sqrt(log(4.));
    double qb = sinh(qa) / qa;
    double qx = (x[0] - peak) / width * qb;
    double qy = 1. + tail * qx;

    if (qy > 1.E-7)
      qc = 0.5 * (TMath::Power((log(qy) / tail), 2) + tail * tail);
    else
      qc = 15.0;
  }
  return par[0] * exp(-qc) + par[4];
}


eclMuMuEAlgorithm::eclMuMuEAlgorithm(): CalibrationAlgorithm("eclMuMuECollector"), cellIDLo(1),
  cellIDHi(ECLElementNumbers::c_NCrystals), minEntries(150),
  nToRebin(1000), tRatioMin(0.05), tRatioMax(0.40), lowerEdgeThresh(0.10), performFits(true), findExpValues(false), storeConst(0)
{
  setDescription(
    "Perform energy calibration of ecl crystals by fitting a Novosibirsk function to energy deposited by muons"
  );
}

CalibrationAlgorithm::EResult eclMuMuEAlgorithm::calibrate()
{
  /**-----------------------------------------------------------------------------------------------*/
  /** ranges of various fit parameters, and tolerance to determine that fit is at the limit */
  double limitTol = 0.0005; /*< tolerance for checking if a parameter is at the limit */
  double minFitLimit = 1e-25; /*< cut off for labeling a fit as poor */
  double peakMin(0.4), peakMax(2.2); /*< range for peak of normalized energy distribution */
  double peakTol = limitTol * (peakMax - peakMin); /*< fit is at limit if it is within peakTol of min or max */
  double effSigMin(0.02), effSigMax(0.2); /*< range for effective sigma of normalized energy distribution */
  double effSigTol = limitTol * (effSigMax - effSigMin); /*< fit is at limit if it is within effSigTol of min or max */
  double etaNom(-0.41); /*< Nominal tail parameter; fixed to this value for low statistics fits */
  double etaMin(-1.), etaMax(0.); /*< Novosibirsk tail parameter range */
  double etaTol = limitTol * (etaMax - etaMin); /*< fit is at limit if it is within etaTol of min or max */

  /** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();

  /**-----------------------------------------------------------------------------------------------*/
  /** Write out the job parameters */
  B2INFO("eclMuMuEAlgorithm parameters:");
  B2INFO("cellIDLo = " << cellIDLo);
  B2INFO("cellIDHi = " << cellIDHi);
  B2INFO("minEntries = " << minEntries);
  B2INFO("tRatioMin = " << tRatioMin);
  B2INFO("tRatioMax = " << tRatioMax);
  B2INFO("lowerEdgeThresh = " << lowerEdgeThresh);
  B2INFO("performFits = " << performFits);
  B2INFO("findExpValues = " << findExpValues);
  B2INFO("storeConst = " << storeConst);

  /**-----------------------------------------------------------------------------------------------*/
  /** Clean up existing histograms if necessary */
  TH1F* dummy;
  dummy = (TH1F*)gROOT->FindObject("IntegralVsCrysID");
  if (dummy) {delete dummy;}
  dummy = (TH1F*)gROOT->FindObject("AverageExpECrys");
  if (dummy) {delete dummy;}
  dummy = (TH1F*)gROOT->FindObject("AverageElecCalib");
  if (dummy) {delete dummy;}
  dummy = (TH1F*)gROOT->FindObject("AverageInitCalib");
  if (dummy) {delete dummy;}

  /**-----------------------------------------------------------------------------------------------*/
  /** Histograms containing the data collected by eclMuMuECollectorModule */
  auto TrkPerCrysID = getObjectPtr<TH1F>("TrkPerCrysID");
  auto EnVsCrysID = getObjectPtr<TH2F>("EnVsCrysID");
  auto ExpEvsCrys = getObjectPtr<TH1F>("ExpEvsCrys");
  auto ElecCalibvsCrys = getObjectPtr<TH1F>("ElecCalibvsCrys");
  auto InitialCalibvsCrys = getObjectPtr<TH1F>("InitialCalibvsCrys");
  auto CalibEntriesvsCrys = getObjectPtr<TH1F>("CalibEntriesvsCrys");
  auto RawDigitAmpvsCrys = getObjectPtr<TH2F>("RawDigitAmpvsCrys");
  auto RawDigitTimevsCrys = getObjectPtr<TH2F>("RawDigitTimevsCrys");
  auto hitCrysVsExtrapolatedCrys = getObjectPtr<TH2F>("hitCrysVsExtrapolatedCrys");

  /**-----------------------------------------------------------------------------------------------*/
  /** Record the number of entries per crystal in the normalized energy histogram and calculate the average expected energy per crystal and calibration constants from Collector */

  TH1F* IntegralVsCrysID = new TH1F("IntegralVsCrysID", "Integral of EnVsCrysID for each crystal;crystal ID;Entries",
                                    ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals);
  TH1F* AverageExpECrys = new TH1F("AverageExpECrys", "Average expected E per crys from collector;Crystal ID;Energy (GeV)",
                                   ECLElementNumbers::c_NCrystals, 0,
                                   ECLElementNumbers::c_NCrystals);
  TH1F* AverageElecCalib = new TH1F("AverageElecCalib", "Average electronics calib const vs crystal;Crystal ID;Calibration constant",
                                    ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals);
  TH1F* AverageInitCalib = new TH1F("AverageInitCalib", "Average initial calib const vs crystal;Crystal ID;Calibration constant",
                                    ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals);

  for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {
    TH1D* hEnergy = EnVsCrysID->ProjectionY("hEnergy", crysID + 1, crysID + 1);
    int Integral = hEnergy->Integral();
    IntegralVsCrysID->SetBinContent(crysID + 1, Integral);

    double TotEntries = CalibEntriesvsCrys->GetBinContent(crysID + 1);

    double expectedE = 0.;
    if (TotEntries > 0.) {expectedE = ExpEvsCrys->GetBinContent(crysID + 1) / TotEntries;}
    AverageExpECrys->SetBinContent(crysID + 1, expectedE);

    double calibconst = 0.;
    if (TotEntries > 0.) {calibconst = ElecCalibvsCrys->GetBinContent(crysID + 1) / TotEntries;}
    AverageElecCalib->SetBinContent(crysID + 1, calibconst);

    calibconst = 0.;
    if (TotEntries > 0.) {calibconst = InitialCalibvsCrys->GetBinContent(crysID + 1) / TotEntries;}
    AverageInitCalib->SetBinContent(crysID + 1, calibconst);
  }

  /**-----------------------------------------------------------------------------------------------*/
  /** Write out the basic histograms in all cases */
  TFile* histfile = new TFile("eclMuMuEAlgorithm.root", "recreate");
  TrkPerCrysID->Write();
  EnVsCrysID->Write();
  IntegralVsCrysID->Write();
  AverageExpECrys->Write();
  AverageElecCalib->Write();
  AverageInitCalib->Write();
  RawDigitAmpvsCrys->Write();
  RawDigitTimevsCrys->Write();
  hitCrysVsExtrapolatedCrys->Write();

  /**-----------------------------------------------------------------------------------------------*/
  /** If we have not been asked to do fits, we can quit now */
  if (!performFits) {
    B2RESULT("eclMuMuEAlgorithm has not been asked to perform fits; copying input histograms and quitting");
    histfile->Close();
    return c_NotEnoughData;
  }

  /**-----------------------------------------------------------------------------------------------*/
  /** Check that every crystal has enough entries. */
  bool sufficientData = true;
  for (int crysID = cellIDLo - 1; crysID < cellIDHi; crysID++) {
    if (IntegralVsCrysID->GetBinContent(crysID + 1) < minEntries) {
      if (storeConst == 1) {B2RESULT("eclMuMuEAlgorithm: crystal " << crysID << " has insufficient statistics: " << IntegralVsCrysID->GetBinContent(crysID + 1) << ". Requirement is " << minEntries);}
      sufficientData = false;
      break;
    }
  }

  /** Insufficient data. Quit if we are required to have a successful fit for every crystal */
  if (!sufficientData && storeConst == 1) {
    histfile->Close();
    return c_NotEnoughData;
  }

  /**-----------------------------------------------------------------------------------------------*/
  /** Some prep for the many fits about to follow  */

  /** histograms to store results for database */
  TH1F* CalibVsCrysID = new TH1F("CalibVsCrysID", "Calibration constant vs crystal ID;crystal ID;counts per GeV",
                                 ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals);
  TH1F* ExpEnergyperCrys = new TH1F("ExpEnergyperCrys", "Expected energy per crystal;Crystal ID;Peak energy (GeV)",
                                    ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals);

  /** Diagnostic histograms */
  TH1F* PeakVsCrysID = new TH1F("PeakVsCrysID", "Peak of Novo fit vs crystal ID;crystal ID;Peak normalized energy",
                                ECLElementNumbers::c_NCrystals, 0,
                                ECLElementNumbers::c_NCrystals);
  TH1F* EdgeVsCrysID = new TH1F("EdgeVsCrysID", "Lower edge of Novo fit vs crystal ID;crystal ID", ECLElementNumbers::c_NCrystals, 0,
                                ECLElementNumbers::c_NCrystals);
  TH1F* effSigVsCrysID = new TH1F("effSigVsCrysID", "effSigma vs crystal ID;crystal ID;sigma)", ECLElementNumbers::c_NCrystals, 0,
                                  ECLElementNumbers::c_NCrystals);
  TH1F* etaVsCrysID = new TH1F("etaVsCrysID", "eta vs crystal ID;crystal ID;Novo eta parameter", ECLElementNumbers::c_NCrystals, 0,
                               ECLElementNumbers::c_NCrystals);
  TH1F* normVsCrysID = new TH1F("normVsCrysID", "Novosibirsk normalization vs crystal ID;crystal ID;normalization",
                                ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals);
  TH1F* lowerLimitVsCrysID = new TH1F("lowerLimitVsCrysID", "fit range lower limit vs crystal ID;crystal ID;lower fit limit",
                                      ECLElementNumbers::c_NCrystals, 0,
                                      ECLElementNumbers::c_NCrystals);
  TH1F* fitLimitVsCrysID = new TH1F("fitLimitVsCrysID", "fit range upper limit vs crystal ID;crystal ID;upper fit limit",
                                    ECLElementNumbers::c_NCrystals, 0,
                                    ECLElementNumbers::c_NCrystals);
  TH1F* StatusVsCrysID = new TH1F("StatusVsCrysID", "Fit status vs crystal ID;crystal ID;Fit status", ECLElementNumbers::c_NCrystals,
                                  0, ECLElementNumbers::c_NCrystals);

  /** 1D summary histograms */
  TH1F* hStatus = new TH1F("hStatus", "Fit status", 25, -5, 20);
  TH1F* hPeak = new TH1F("hPeak", "Peaks of normalized energy distributions, successful fits;Peak of Novosibirsk fit", 200, 0.8, 1.2);
  TH1F* fracPeakUnc = new TH1F("fracPeakUnc", "Fractional uncertainty on peak uncertainty, successful fits;Uncertainty on peak", 100,
                               0, 0.1);


  /**-----------------------------------------------------------------------------------------------*/
  /** Fits are requested and there is sufficient data. Loop over specified crystals and performs fits to the amplitude distributions */
  bool allFitsOK = true;
  for (int crysID = cellIDLo - 1; crysID < cellIDHi; crysID++) {

    /**  Project 1D histogram of energy in this crystal */
    TString name = "Enormalized";
    name += crysID;
    TH1D* hEnergy = EnVsCrysID->ProjectionY(name, crysID + 1, crysID + 1);

    /** Fit function (xmin, xmax, nparameters) for this histogram */
    double histMin = hEnergy->GetXaxis()->GetXmin();
    double histMax = hEnergy->GetXaxis()->GetXmax();
    TF1* func = new TF1("eclNovoConst", eclNovoConst, histMin, histMax, 5);
    func->SetParNames("normalization", "peak", "effSigma", "eta", "const");
    func->SetParLimits(1, peakMin, peakMax);
    func->SetParLimits(2, effSigMin, effSigMax);
    func->SetParLimits(3, etaMin, etaMax);

    //..Currently not using the constant term
    double constant = 0.;
    func->FixParameter(4, constant);

    //..If low statistics, rebin, and fix eta
    if (hEnergy->GetEntries() < nToRebin) {
      hEnergy->Rebin(2);
      func->FixParameter(3, etaNom);
    }

    /** Estimate initial parameters from the histogram. For peak, use maximum bin  in the allowed range */
    hEnergy->GetXaxis()->SetRangeUser(peakMin, peakMax);
    double peak = hEnergy->GetMaximum();
    int maxBin = hEnergy->GetMaximumBin();
    double peakE = hEnergy->GetBinLowEdge(maxBin);
    double peakEUnc = 0.;
    double normalization = hEnergy->GetMaximum();
    double normUnc = 0.;
    double effSigma = hEnergy->GetRMS();
    double sigmaUnc = 0.;
    hEnergy->GetXaxis()->SetRangeUser(histMin, histMax);
    double fitProb = 0.;

    /** eta is nominal values */
    double eta = etaNom;
    double etaUnc = 0.;

    //..Will find the lower edge of normalized energy at the end of the fit
    double lowerEnEdge = 0.;

    //..Fit range from set of bins with sufficient entries.
    double targetY = tRatioMin * peak;
    int iLow = maxBin;
    while (hEnergy->GetBinContent(iLow) > targetY) {iLow--;}
    double fitlow = hEnergy->GetBinLowEdge(iLow);

    targetY = tRatioMax * peak;
    int iHigh = maxBin;
    while (hEnergy->GetBinContent(iHigh) > targetY) {iHigh++;}
    double fithigh = hEnergy->GetBinLowEdge(iHigh + 1);

    /**---------------------------------------------------------------------------------------*/
    /** Fit */
    bool fitHist = IntegralVsCrysID->GetBinContent(crysID + 1) >= minEntries; /* fit only if enough events */
    if (fitHist) {

      /** Set the initial parameters */
      func->SetParameters(normalization, peakE, effSigma, eta, constant);

      /** Fit */
      hEnergy->Fit(func, "LIQ", "", fitlow, fithigh);
      normalization = func->GetParameter(0);
      normUnc = func->GetParError(0);
      peakE = func->GetParameter(1);
      peakEUnc = func->GetParError(1);
      effSigma = func->GetParameter(2);
      sigmaUnc = func->GetParError(2);
      eta = func->GetParameter(3);
      etaUnc = func->GetParError(3);
      fitProb = func->GetProb();

      //..Lower edge of the fit function is used to find the calibration constant.
      //  Can now use the peak of the fit, instead of the bin content.
      peak = func->Eval(peakE);
      targetY = lowerEdgeThresh * peak;

      /** bins on either side of this value */
      iHigh = hEnergy->GetXaxis()->FindBin(peakE) + 1;
      iLow = hEnergy->GetXaxis()->FindBin(fitlow);
      int iLast = iHigh;
      for (int ibin = iHigh; ibin > iLow; ibin--) {
        double xc = hEnergy->GetBinCenter(ibin);
        if (func->Eval(xc) > targetY) {iLast = ibin;}
      }
      double xHigh = hEnergy->GetBinCenter(iLast);
      double xLow = hEnergy->GetBinCenter(iLast - 1);

      /** look for the target value between these two points */
      if (func->Eval(xLow) < targetY and func->Eval(xHigh) > targetY) {
        func->SetNpx(1000);
        lowerEnEdge = func->GetX(targetY, xLow, xHigh);
      }
    }

    /**-----------------------------------------------------------------------------------------*/
    /** Fit status */
    int iStatus = fitOK; // success

    /** did not find lower edge */
    if (lowerEnEdge < 0.01) {iStatus = noLowerEdge;}

    /** poor fit */
    if (fitProb <= minFitLimit) {iStatus = poorFit;}

    /** parameter at limit */
    if ((peakE < peakMin + peakTol) || (peakE > peakMax - peakTol)) {iStatus = atLimit;}
    if ((effSigma < effSigMin + effSigTol) || (effSigma > effSigMax - effSigTol)) {iStatus = atLimit;}
    if ((eta < etaMin + etaTol) || (eta > etaMax - etaTol)) {iStatus = atLimit;}

    //** No fit
    if (!fitHist) {iStatus = notFit;} // not fit

    /** fill diagnostic histograms */
    int histbin = crysID + 1;
    PeakVsCrysID->SetBinContent(histbin, peakE);
    PeakVsCrysID->SetBinError(histbin, peakEUnc);
    EdgeVsCrysID->SetBinContent(histbin, lowerEnEdge);
    EdgeVsCrysID->SetBinError(histbin, peakEUnc); // approximate
    effSigVsCrysID->SetBinContent(histbin, effSigma);
    effSigVsCrysID->SetBinError(histbin, sigmaUnc);
    etaVsCrysID->SetBinContent(histbin, eta);
    etaVsCrysID->SetBinError(histbin, etaUnc);
    normVsCrysID->SetBinContent(histbin, normalization);
    normVsCrysID->SetBinError(histbin, normUnc);
    lowerLimitVsCrysID->SetBinContent(histbin, fitlow);
    lowerLimitVsCrysID->SetBinError(histbin, 0);
    fitLimitVsCrysID->SetBinContent(histbin, fithigh);
    fitLimitVsCrysID->SetBinError(histbin, 0);
    StatusVsCrysID->SetBinContent(histbin, iStatus);
    StatusVsCrysID->SetBinError(histbin, 0);

    /** 1D summary histograms */
    hStatus->Fill(iStatus);
    if (iStatus >= iterations) {
      hPeak->Fill(peakE);
      fracPeakUnc->Fill(peakEUnc / peakE);
    }

    /** Store histogram with fit */
    B2INFO("cellID " << crysID + 1 << " status = "  << iStatus << " fit probability = " << fitProb);
    histfile->cd();
    hEnergy->Write();

  } /* end of loop over crystals */

  /**---------------------------------------------------------------------------------------------*/
  /** Interpret results of fit as expected energy or calibration constant */
  for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {
    int histbin = crysID + 1;
    double fitstatus = StatusVsCrysID->GetBinContent(histbin);
    double peakE = PeakVsCrysID->GetBinContent(histbin);
    double edge = EdgeVsCrysID->GetBinContent(histbin);
    double fracpeakEUnc = PeakVsCrysID->GetBinError(histbin) / peakE;

    /** if the fit is not successful, set peakE and edge to -1, so that output calib = -1 * abs(input calib) */
    if (fitstatus < iterations) {
      peakE = -1.;
      edge = -1.;
      fracpeakEUnc = 0.;
      if (histbin >= cellIDLo && histbin <= cellIDHi) {
        B2RESULT("eclMuMuEAlgorithm: cellID " << histbin << " is not a successful fit. Status = " << fitstatus);
        allFitsOK = false;
      }
    }

    /** Find expected energies from MC, if requested */
    if (findExpValues) {
      double inputExpE = abs(AverageExpECrys->GetBinContent(histbin));
      ExpEnergyperCrys->SetBinContent(histbin, inputExpE * edge);
      ExpEnergyperCrys->SetBinError(histbin, fracpeakEUnc * inputExpE * peakE);
    } else {

      /** Otherwise, calibration constant */
      double inputCalib = abs(AverageInitCalib->GetBinContent(histbin));
      CalibVsCrysID->SetBinContent(histbin, inputCalib / edge);
      CalibVsCrysID->SetBinError(histbin, fracpeakEUnc * inputCalib / peakE);
    }
  }

  /**-----------------------------------------------------------------------------------------------*/
  /** Write output to DB if requested and successful */
  bool DBsuccess = false;
  if (storeConst == 0 || (storeConst == 1 && allFitsOK)) {
    DBsuccess = true;
    if (findExpValues) {

      /** Store expected energies */
      std::vector<float> tempE;
      std::vector<float> tempUnc;
      for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {
        tempE.push_back(ExpEnergyperCrys->GetBinContent(crysID + 1));
        tempUnc.push_back(ExpEnergyperCrys->GetBinError(crysID + 1));
      }
      ECLCrystalCalib* ExpectedE = new ECLCrystalCalib();
      ExpectedE->setCalibVector(tempE, tempUnc);
      saveCalibration(ExpectedE, "ECLExpMuMuE");
      B2RESULT("eclCosmicEAlgorithm: successfully stored expected energies ECLExpMuMuE");

    } else {

      /** Store calibration constants */
      std::vector<float> tempCalib;
      std::vector<float> tempCalibUnc;
      for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {
        tempCalib.push_back(CalibVsCrysID->GetBinContent(crysID + 1));
        tempCalibUnc.push_back(CalibVsCrysID->GetBinError(crysID + 1));
      }
      ECLCrystalCalib* MuMuECalib = new ECLCrystalCalib();
      MuMuECalib->setCalibVector(tempCalib, tempCalibUnc);
      saveCalibration(MuMuECalib, "ECLCrystalEnergyMuMu");
      B2RESULT("eclMuMuEAlgorithm: successfully stored ECLCrystalEnergyMuMu calibration constants");
    }
  }

  /**-----------------------------------------------------------------------------------------------*/

  /** Write out diagnostic histograms */
  PeakVsCrysID->Write();
  EdgeVsCrysID->Write();
  effSigVsCrysID->Write();
  etaVsCrysID->Write();
  normVsCrysID->Write();
  lowerLimitVsCrysID->Write();
  fitLimitVsCrysID->Write();
  StatusVsCrysID->Write();
  hPeak->Write();
  fracPeakUnc->Write();
  hStatus->Write();

  /** Histograms containing values written to DB */
  if (findExpValues) {
    ExpEnergyperCrys->Write();
  } else {
    CalibVsCrysID->Write();
  }
  histfile->Close();

  /**-----------------------------------------------------------------------------------------------*/
  /** Clean up histograms in case Algorithm is called again */
  dummy = (TH1F*)gROOT->FindObject("PeakVsCrysID"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("EdgeVsCrysID"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("effSigVsCrysID"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("etaVsCrysID"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("normVsCrysID"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("lowerLimitVsCrysID"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("fitLimitVsCrysID"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("StatusVsCrysID"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("fitProbSame"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("fracPeakUnc"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("hStatus"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("ExpEnergyperCrys"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("CalibVsCrysID"); delete dummy;


  /**-----------------------------------------------------------------------------------------------*/
  /** Set the return code appropriately */
  if (storeConst == -1) {
    B2RESULT("eclMuMuEAlgorithm performed fits but was not asked to store contants");
    return c_Failure;
  } else if (!DBsuccess) {
    if (findExpValues) { B2RESULT("eclMuMuEAlgorithm: failed to store expected values"); }
    else { B2RESULT("eclMuMuEAlgorithm: failed to store calibration constants"); }
    return c_Failure;
  }
  return c_OK;
}
