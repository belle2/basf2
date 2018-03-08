
#include <ecl/calibration/eclGammaGammaEAlgorithm.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

#include "TH2F.h"
#include "TFile.h"
#include "TMath.h"
#include "TF1.h"
#include "TMinuit.h"
#include "TROOT.h"

using namespace std;
using namespace Belle2;
using namespace ECL;

/**-----------------------------------------------------------------------------------------------*/
/** Novosibirsk function, plus constant H. Ikeda et al., Nuclear Instruments and Methods A 441 (2000) 401-426 */
double eclGammaGammaNovoConst(double* x, double* par)
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


/**-----------------------------------------------------------------------------------------------*/
eclGammaGammaEAlgorithm::eclGammaGammaEAlgorithm(): CalibrationAlgorithm("eclGammaGammaECollector"),
  outputName("eclGammaGammaEAlgorithm.root"), cellIDLo(1), cellIDHi(8736), minEntries(150),
  maxIterations(10), tRatioMin(0.45), tRatioMax(0.60), upperEdgeThresh(0.02), performFits(true), findExpValues(false), storeConst(0)
{
  setDescription(
    "Perform energy calibration of ecl crystals by fitting a Novosibirsk function to energy deposited by photons in e+e- --> gamma gamma"
  );
}

CalibrationAlgorithm::EResult eclGammaGammaEAlgorithm::calibrate()
{
  /**-----------------------------------------------------------------------------------------------*/
  /** ranges of various fit parameters, and tolerance to determine that fit is at the limit */
  double limitTol = 0.0005; /*< tolerance for checking if a parameter is at the limit */
  double minFitLimit = 1e-25; /*< cut off for labeling a fit as poor */
  double minFitProbIter = 1e-8; /*< cut off for labeling a fit as poor if it also has many iterations */
  double constRatio = 0.5; /*< Novosibirsk normalization must be greater than constRatio x constant term */
  double peakMin(0.4), peakMax(2.2); /*< range for peak of normalized energy distribution */
  double peakTol = limitTol * (peakMax - peakMin); /*< fit is at limit if it is within peakTol of min or max */
  double effSigMin(0.02), effSigMax(0.4); /*< range for effective sigma of normalized energy distribution */
  double effSigTol = limitTol * (effSigMax - effSigMin); /*< fit is at limit if it is within effSigTol of min or max */
  double etaNom(0.41); /*< Nominal tail parameter */
  double etaMin(0.), etaMax(5.0); /*< Novosibirsk tail parameter range */
  double etaTol = limitTol * (etaMax - etaMin); /*< fit is at limit if it is within etaTol of min or max */
  double constTol = 0.001; /*< if constant is less than constTol, it will be fixed to 0 */

  /** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();

  /**-----------------------------------------------------------------------------------------------*/
  /** Write out the job parameters */
  B2INFO("eclGammaGammaAlgorithm parameters:");
  B2INFO("outputName = " << outputName);
  B2INFO("cellIDLo = " << cellIDLo);
  B2INFO("cellIDHi = " << cellIDHi);
  B2INFO("minEntries = " << minEntries);
  B2INFO("maxIterations = " << maxIterations);
  B2INFO("tRatioMin = " << tRatioMin);
  B2INFO("tRatioMax = " << tRatioMax);
  B2INFO("upperEdgeThresh = " << upperEdgeThresh);
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
  /** Histograms containing the data collected by eclGammaGammaECollectorModule */
  auto EnVsCrysID = getObjectPtr<TH2F>("EnVsCrysID");
  auto ExpEvsCrys = getObjectPtr<TH1F>("ExpEvsCrys");
  auto ElecCalibvsCrys = getObjectPtr<TH1F>("ElecCalibvsCrys");
  auto InitialCalibvsCrys = getObjectPtr<TH1F>("InitialCalibvsCrys");
  auto CalibEntriesvsCrys = getObjectPtr<TH1F>("CalibEntriesvsCrys");

  /**-----------------------------------------------------------------------------------------------*/
  /** Record the number of entries per crystal in the normalized energy histogram and calculate the average expected energy per crystal and calibration constants from Collector */

  TH1F* IntegralVsCrysID = new TH1F("IntegralVsCrysID", "Integral of EnVsCrysID for each crystal;crystal ID;Entries", 8736, 0, 8736);
  TH1F* AverageExpECrys = new TH1F("AverageExpECrys", "Average expected E per crys from collector;Crystal ID;Energy (GeV)", 8736, 0,
                                   8736);
  TH1F* AverageElecCalib = new TH1F("AverageElecCalib", "Average electronics calib const vs crystal;Crystal ID;Calibration constant",
                                    8736, 0, 8736);
  TH1F* AverageInitCalib = new TH1F("AverageInitCalib", "Average initial calib const vs crystal;Crystal ID;Calibration constant",
                                    8736, 0, 8736);

  for (int crysID = 0; crysID < 8736; crysID++) {
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
  TString fName = outputName;
  TFile* histfile = new TFile(fName, "recreate");
  EnVsCrysID->Write();
  IntegralVsCrysID->Write();
  AverageExpECrys->Write();
  AverageElecCalib->Write();
  AverageInitCalib->Write();

  /**-----------------------------------------------------------------------------------------------*/
  /** If we have not been asked to do fits, we can quit now */
  if (!performFits) {
    B2INFO("eclGammaGammaEAlgorithm has not been asked to perform fits; copying input histograms and quitting");
    histfile->Close();
    return c_NotEnoughData;
  }

  /**-----------------------------------------------------------------------------------------------*/
  /** Check that every crystal has enough entries, if so requested */
  bool sufficientData = true;
  for (int crysID = cellIDLo - 1; crysID < cellIDHi; crysID++) {
    if (IntegralVsCrysID->GetBinContent(crysID + 1) < minEntries) {
      if (storeConst == 1) {B2INFO("eclGammaGammaEAlgorithm: crystal " << crysID << " has insufficient statistics: " << IntegralVsCrysID->GetBinContent(crysID + 1) << ". Requirement is " << minEntries);}
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
  TH1F* CalibVsCrysID = new TH1F("CalibVsCrysID", "Calibration constant vs crystal ID;crystal ID;counts per GeV", 8736, 0, 8736);
  TH1F* ExpEnergyperCrys = new TH1F("ExpEnergyperCrys", "Expected energy per crystal;Crystal ID;Peak energy (GeV)", 8736, 0, 8736);

  /** Diagnostic histograms */
  TH1F* PeakVsCrysID = new TH1F("PeakVsCrysID", "Peak of Novo fit vs crystal ID;crystal ID;Peak normalized energy", 8736, 0, 8736);
  TH1F* EdgeVsCrysID = new TH1F("EdgeVsCrysID", "Upper edge of Novo fit vs crystal ID;crystal ID;Maximum normalized energy", 8736, 0,
                                8736);
  TH1F* effSigVsCrysID = new TH1F("effSigVsCrysID", "effSigma vs crystal ID;crystal ID;sigma)", 8736, 0, 8736);
  TH1F* etaVsCrysID = new TH1F("etaVsCrysID", "eta vs crystal ID;crystal ID;Novo eta parameter", 8736, 0, 8736);
  TH1F* constVsCrysID = new TH1F("constVsCrysID", "fit constant vs crystal ID;crystal ID;fit constant", 8736, 0, 8736);
  TH1F* normVsCrysID = new TH1F("normVsCrysID", "Novosibirsk normalization vs crystal ID;crystal ID;normalization", 8736, 0, 8736);
  TH1F* fitLimitVsCrysID = new TH1F("fitLimitVsCrysID", "fit range lower limit vs crystal ID;crystal ID;upper fit limit", 8736, 0,
                                    8736);
  TH1F* StatusVsCrysID = new TH1F("StatusVsCrysID", "Fit status vs crystal ID;crystal ID;Fit status", 8736, 0, 8736);
  TH1F* FitProbVsCrysID = new TH1F("FitProbVsCrysID", "Fit probability vs crystal id;crystal ID;Fit probability", 8736, 0, 8736);

  /** 1D summary histograms */
  TH1F* hStatus = new TH1F("hStatus", "Fit status", 25, -5, 20);
  TH1F* hPeak = new TH1F("hPeak", "Peaks of normalized energy distributions, successful fits;Peak of Novosibirsk fit", 200, 0.8, 1.2);
  TH1F* fracPeakUnc = new TH1F("fracPeakUnc", "Fractional uncertainty on peak uncertainty, successful fits;Uncertainty on peak", 100,
                               0, 0.1);
  TH1F* nIterations = new TH1F("nIterations", "Number of times histogram was fit;Number of iterations", 20, -0.5, 19.5);


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
    TF1* func = new TF1("eclGammaGammaNovoConst", eclGammaGammaNovoConst, histMin, histMax, 5);
    func->SetParNames("normalization", "peak", "effSigma", "eta", "const");
    func->SetParLimits(1, peakMin, peakMax);
    func->SetParLimits(2, effSigMin, effSigMax);
    func->SetParLimits(3, etaMin, etaMax);
    func->SetParLimits(4, 0., hEnergy->GetMaximum());

    /** Estimate initial parameters from the histogram. For peak, use maximum bin  in the allowed range */
    hEnergy->GetXaxis()->SetRangeUser(peakMin, peakMax);
    int maxBin = hEnergy->GetMaximumBin();
    double peakE = hEnergy->GetBinLowEdge(maxBin);
    double peakEUnc = 0.;
    double normalization = hEnergy->GetMaximum();
    double normUnc = 0.;
    double effSigma = hEnergy->GetRMS();
    double sigmaUnc = 0.;
    hEnergy->GetXaxis()->SetRangeUser(histMin, histMax);

    /** Fit range is just below peak to the histogram maximum */
    double fitlow = peakE - effSigma;
    double fithigh = histMax;

    /** eta and constant are nominal values */
    double eta = etaNom;
    double etaUnc = 0.;
    double constant = 0.01 * normalization;
    double constUnc = 0.;

    /** Parameters to control iterations. dIter checks if we are stuck in a loop */
    double dIter = 0.1 * (histMax - histMin) / hEnergy->GetNbinsX();
    double fitProb(0.);
    double fitProbDefault(0.);
    double lowold(0.), lowoldold(0.);
    bool fixConst = false;
    int nIter = 0;
    bool fitHist = IntegralVsCrysID->GetBinContent(crysID + 1) >= minEntries; /* fit only if enough events */

    /**---------------------------------------------------------------------------------------*/
    /** Iterate from this point */
    while (fitHist) {

      /** Set the initial parameters */
      func->SetParameters(normalization, peakE, effSigma, eta, constant);
      if (fixConst) { func->FixParameter(4, 0); }

      /** Fit */
      hEnergy->Fit(func, "LIQ", "", fitlow, fithigh);
      nIter++;
      fitHist = false;
      normalization = func->GetParameter(0);
      normUnc = func->GetParError(0);
      peakE = func->GetParameter(1);
      peakEUnc = func->GetParError(1);
      effSigma = func->GetParameter(2);
      sigmaUnc = func->GetParError(2);
      eta = func->GetParameter(3);
      etaUnc = func->GetParError(3);
      constant = func->GetParameter(4);
      constUnc = func->GetParError(4);
      fitProbDefault = func->GetProb();

      /** The lower fit range should correspond the specified fraction of the peak. Iterate if necessary. */
      double peak = func->Eval(peakE) - constant;
      double tRatio = (func->Eval(fitlow) - constant) / peak;
      if (tRatio < tRatioMin || tRatio > tRatioMax) {
        double targetY = constant + 0.5 * (tRatioMin + tRatioMax) * peak;
        lowoldold = lowold;
        lowold = fitlow;
        fitlow = func->GetX(targetY, histMin, peakE);
        fitHist = true;

        /** Check if we are oscillating between two end points */
        if (abs(fitlow - lowoldold) < dIter) {fitlow = 0.5 * (lowold + lowoldold); }

        /** Many iterations may mean we are stuck in a loop. Try a different end point. */
        if (nIter > maxIterations - 3) {fitlow = 0.33333 * (fitlow + lowold + lowoldold); }
      }

      /** Set the constant term to 0 if we are close to the limit */
      if (constant < constTol && !fixConst) {
        constant = 0;
        fixConst = true;
        fitHist = true;
      }

      /** No more than specified number of iterations */
      if (nIter == maxIterations) {fitHist = false;}
      B2DEBUG(200, crysID << " " << nIter << " " << peakE << " " << constant << " " << tRatio << " " << fitlow);
    }

    /**-----------------------------------------------------------------------------------------------*/
    /**..Manually calculate a more meaningful fit probability. Same as P option in fit, which cannot be used with L */
    fitProb = 0.;
    if (nIter > 0) {
      int lowbin = hEnergy->GetXaxis()->FindBin(fitlow);
      int highbin = hEnergy->GetXaxis()->FindBin(fithigh);
      int npar = 5;
      if (fixConst) {npar = 4;}
      int ndeg = -npar;
      double chisq = 0.;
      double binwidth = hEnergy->GetBinWidth(1);
      for (int ib = lowbin; ib <= highbin; ib++) {
        double xlow = hEnergy->GetBinLowEdge(ib);
        double yexp = func->Integral(xlow, xlow + binwidth) / binwidth;

        /** only include this bin if meaningful */
        if (yexp > constTol) {
          double yobs = hEnergy->GetBinContent(ib);
          double dnom = yexp;
          if (yexp < 0.9999 && yobs > yexp) {dnom = yobs;}
          double dchi2 = (yexp - yobs) * (yexp - yobs) / dnom;
          chisq += dchi2;
          ndeg++;
        }
      }
      fitProb = TMath::Prob(chisq, ndeg);
    }

    /**-----------------------------------------------------------------------------------------*/
    /** Fit status */
    int iStatus = fitOK; // success
    if (nIter == maxIterations) {iStatus = iterations;} // too many iterations

    /** No peak; normalization of Novo component is too small */
    if (normalization < constRatio * constant) {iStatus = noPeak;}

    /** poor fit, or relatively poor fit with too many iterations */
    if (fitProb <= minFitLimit || (fitProb < minFitProbIter && iStatus == iterations)) {iStatus = poorFit;}

    /** parameter at limit */
    if ((peakE < peakMin + peakTol) || (peakE > peakMax - peakTol)) {iStatus = atLimit;}
    if ((effSigma < effSigMin + effSigTol) || (effSigma > effSigMax - effSigTol)) {iStatus = atLimit;}
    if ((eta < etaMin + etaTol) || (eta > etaMax - etaTol)) {iStatus = atLimit;}

    //** No fit
    if (nIter == 0) {iStatus = notFit;} // not fit

    /**-----------------------------------------------------------------------------------------*/
    /** Find upper edge of Novosibirsk fit, if possible */
    double upperEdge = peakE;
    double edgeUnc = peakEUnc;

    if (iStatus >= iterations) {

      /** Look for the fit to drop to specified fraction of peak. */
      double targetY = constant + upperEdgeThresh * (func->Eval(peakE) - constant);

      /** bins on either side of this value */
      int iLow = hEnergy->GetXaxis()->FindBin(peakE) + 1;
      int iHigh = hEnergy->GetNbinsX();
      int iLast = iLow;
      for (int ibin = iLow; ibin < iHigh; ibin++) {
        double xc = hEnergy->GetBinCenter(ibin);
        if (func->Eval(xc) > targetY) {iLast = ibin;}
      }
      double xLow = hEnergy->GetBinCenter(iLast);
      double xHigh = hEnergy->GetBinCenter(iLast + 1);

      /** look for the target value between these two points */
      func->SetNpx(1000);
      upperEdge = func->GetX(targetY, xLow, xHigh);


    } else if (iStatus > notFit) {

      /** Fit was not successful despite sufficient statistics; find upper edge from bin contents */
      int iLast = -1;
      int thisBin = hEnergy->GetBinContent(1);
      for (int ibin = 2; ibin < hEnergy->GetNbinsX(); ibin++) {
        int prevBin = thisBin;
        thisBin = hEnergy->GetBinContent(ibin);
        if (thisBin > 0 && thisBin + prevBin >= 2) {iLast = ibin;}
      }
      if (iLast > 0) {upperEdge = hEnergy->GetBinCenter(iLast);}
    }

    /**-----------------------------------------------------------------------------------------*/
    /** fill diagnostic histograms */
    int histbin = crysID + 1;
    PeakVsCrysID->SetBinContent(histbin, peakE);
    PeakVsCrysID->SetBinError(histbin, peakEUnc);
    EdgeVsCrysID->SetBinContent(histbin, upperEdge);
    EdgeVsCrysID->SetBinError(histbin, edgeUnc);
    effSigVsCrysID->SetBinContent(histbin, effSigma);
    effSigVsCrysID->SetBinError(histbin, sigmaUnc);
    etaVsCrysID->SetBinContent(histbin, eta);
    etaVsCrysID->SetBinError(histbin, etaUnc);
    constVsCrysID->SetBinContent(histbin, constant);
    constVsCrysID->SetBinError(histbin, constUnc);
    normVsCrysID->SetBinContent(histbin, normalization);
    normVsCrysID->SetBinError(histbin, normUnc);
    fitLimitVsCrysID->SetBinContent(histbin, fitlow);
    fitLimitVsCrysID->SetBinError(histbin, 0);
    StatusVsCrysID->SetBinContent(histbin, iStatus);
    StatusVsCrysID->SetBinError(histbin, 0);
    FitProbVsCrysID->SetBinContent(histbin, fitProb);
    FitProbVsCrysID->SetBinError(histbin, 0);

    /** 1D summary histograms */
    hStatus->Fill(iStatus);
    nIterations->Fill(nIter);
    if (iStatus >= iterations) {
      hPeak->Fill(peakE);
      fracPeakUnc->Fill(peakEUnc / peakE);
    }

    /** Store histogram with fit */
    B2INFO("cellID " << crysID + 1 << " status = "  << iStatus << " fit probability = " << fitProb << " default prob = " <<
           fitProbDefault);
    histfile->cd();
    hEnergy->Write();

  } /* end of loop over crystals */

  /**---------------------------------------------------------------------------------------------*/
  /** Interpret results of fit as expected energy or calibration constant */
  for (int crysID = 0; crysID < 8736; crysID++) {
    int histbin = crysID + 1;
    double fitstatus = StatusVsCrysID->GetBinContent(histbin);
    double upperEdge = EdgeVsCrysID->GetBinContent(histbin);
    double fracEdgeUnc = EdgeVsCrysID->GetBinError(histbin) / upperEdge;

    /** if no fit, set upperEdge to -1, so that output calib = -1 * abs(input calib) */
    if (fitstatus < 0) {
      upperEdge = -1.;
      fracEdgeUnc = 0.;
      if (histbin >= cellIDLo && histbin <= cellIDHi) {
        B2INFO("eclGammaGammaEAlgorithm: cellID " << histbin << " is not a successful fit. Status = " << fitstatus);
        allFitsOK = false;
      }
    }

    /** Find expected energies from MC, if requested */
    if (findExpValues) {
      double inputExpE = abs(AverageExpECrys->GetBinContent(histbin));
      ExpEnergyperCrys->SetBinContent(histbin, inputExpE * upperEdge);
      ExpEnergyperCrys->SetBinError(histbin, fracEdgeUnc * inputExpE * upperEdge);
    } else {

      /** Otherwise, calibration constant */
      double inputCalib = abs(AverageInitCalib->GetBinContent(histbin));
      CalibVsCrysID->SetBinContent(histbin, inputCalib / upperEdge);
      CalibVsCrysID->SetBinError(histbin, fracEdgeUnc * inputCalib / upperEdge);
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
      for (int crysID = 0; crysID < 8736; crysID++) {
        tempE.push_back(ExpEnergyperCrys->GetBinContent(crysID + 1));
        tempUnc.push_back(ExpEnergyperCrys->GetBinError(crysID + 1));
      }
      ECLCrystalCalib* ExpectedE = new ECLCrystalCalib();
      ExpectedE->setCalibVector(tempE, tempUnc);
      saveCalibration(ExpectedE, "ECLExpGammaGammaE");
      B2INFO("eclCosmicEAlgorithm: successfully stored expected energies ECLExpGammaGammaE");

    } else {

      /** Store calibration constants */
      std::vector<float> tempCalib;
      std::vector<float> tempCalibUnc;
      for (int crysID = 0; crysID < 8736; crysID++) {
        tempCalib.push_back(CalibVsCrysID->GetBinContent(crysID + 1));
        tempCalibUnc.push_back(CalibVsCrysID->GetBinError(crysID + 1));
      }
      ECLCrystalCalib* GammaGammaECalib = new ECLCrystalCalib();
      GammaGammaECalib->setCalibVector(tempCalib, tempCalibUnc);
      saveCalibration(GammaGammaECalib, "ECLCrystalEnergyGammaGamma");
      B2INFO("eclGammaGammaEAlgorithm: successfully stored ECLCrystalEnergyGammaGamma calibration constants");
    }
  }

  /**-----------------------------------------------------------------------------------------------*/

  /** Write out diagnostic histograms */
  PeakVsCrysID->Write();
  EdgeVsCrysID->Write();
  effSigVsCrysID->Write();
  etaVsCrysID->Write();
  constVsCrysID->Write();
  normVsCrysID->Write();
  fitLimitVsCrysID->Write();
  StatusVsCrysID->Write();
  FitProbVsCrysID->Write();
  hPeak->Write();
  fracPeakUnc->Write();
  nIterations->Write();
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
  dummy = (TH1F*)gROOT->FindObject("constVsCrysID"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("normVsCrysID"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("fitLimitVsCrysID"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("StatusVsCrysID"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("FitProbVsCrysID"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("fracPeakUnc"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("nIterations"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("hStatus"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("ExpEnergyperCrys"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("CalibVsCrysID"); delete dummy;


  /**-----------------------------------------------------------------------------------------------*/
  /** Set the return code appropriately */
  if (storeConst == -1) {
    B2INFO("eclGammaGammaEAlgorithm performed fits but was not asked to store contants");
    return c_Failure;
  } else if (!DBsuccess) {
    if (findExpValues) { B2INFO("eclGammaGammaEAlgorithm: failed to store expected values"); }
    else { B2INFO("eclGammaGammaEAlgorithm: failed to store calibration constants"); }
    return c_Failure;
  }
  return c_OK;
}
