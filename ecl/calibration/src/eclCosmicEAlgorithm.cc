#include <ecl/calibration/eclCosmicEAlgorithm.h>
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

/**..Novosibirsk function, plus constant  H. Ikeda et al., Nuclear Instruments and Methods A 441 (2000) 401-426. */
double eclCosmicNovoConst(double* x, double* par)
{
  double qa = 0, qb = 0, qc = 0, qx = 0, qy = 0;

  double peak = par[1];
  double width = par[2];
  double sln4 = sqrt(log(4));
  double y = par[3] * sln4;
  double tail = -log(y + sqrt(1 + y * y)) / sln4;

  if (TMath::Abs(tail) < 1.e-7) {
    qc = 0.5 * TMath::Power(((x[0] - peak) / width), 2);
  } else {
    qa = tail * sqrt(log(4.));
    qb = sinh(qa) / qa;
    qx = (x[0] - peak) / width * qb;
    qy = 1. + tail * qx;

    if (qy > 1.E-7)
      qc = 0.5 * (TMath::Power((log(qy) / tail), 2) + tail * tail);
    else
      qc = 15.0;
  }
  return par[0] * exp(-qc) + par[4];
}

eclCosmicEAlgorithm::eclCosmicEAlgorithm(): CalibrationAlgorithm("eclCosmicECollector"), cellIDLo(1), cellIDHi(8736),
  minEntries(150), maxIterations(10), tRatioMin(0.2), tRatioMax(0.25), performFits(true), findExpValues(false), storeConst(0)
{
  setDescription(
    "Perform energy calibration of ecl crystals by fitting a Novosibirsk function to energy deposited by cosmic rays"
  );
}

CalibrationAlgorithm::EResult eclCosmicEAlgorithm::calibrate()
{

  /**-----------------------------------------------------------------------------------------------*/
  /**..ranges of various fit parameters, and tolerance to determine that fit is at the limit */
  double limitTol = 0.0005; /*< tolerance for checking if a parameter is at the limit */
  double minFitLimit = 1e-25; /*< cut off for labeling a fit as poor */
  double minFitProbIter = 1e-8; /*< cut off for labeling a fit as poor if it also has many iterations */
  double constRatio = 0.5; /*< Novosibirsk normalization must be greater than constRatio x constant term */
  double peakMin(0.5), peakMax(1.75); /*< range for peak of measured energy distribution */
  double peakTol = limitTol * (peakMax - peakMin); /*< fit is at limit if it is within peakTol of min or max */
  double effSigMin(0.08), effSigMax(0.5); /*< range for effective sigma of measured energy distribution */
  double effSigTol = limitTol * (effSigMax - effSigMin);
  double etaMin(-3.), etaMax(1.); /*< Novosibirsk tail parameter range */
  double etaNom(-0.41); /*< Nominal tail parameter */
  double etaTol = limitTol * (etaMax - etaMin);
  double constTol = 0.1; /*< if constant is less than constTol, it will be fixed to 0 */

  /**..Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();

  /**-----------------------------------------------------------------------------------------------*/
  /**..Clean up existing histograms if necessary */
  TH1F* dummy;
  dummy = (TH1F*)gROOT->FindObject("EnvsCrysSameRing");
  if (dummy) {delete dummy;}
  dummy = (TH1F*)gROOT->FindObject("EnvsCrysDifferentRing");
  if (dummy) {delete dummy;}
  dummy = (TH1F*)gROOT->FindObject("IntegralVsCrysSame");
  if (dummy) {delete dummy;}
  dummy = (TH1F*)gROOT->FindObject("IntegralVsCrysDifferent");
  if (dummy) {delete dummy;}
  dummy = (TH1F*)gROOT->FindObject("AverageExpECrysSame");
  if (dummy) {delete dummy;}
  dummy = (TH1F*)gROOT->FindObject("AverageExpECrysDifferent");
  if (dummy) {delete dummy;}
  dummy = (TH1F*)gROOT->FindObject("AverageElecCalibSame");
  if (dummy) {delete dummy;}
  dummy = (TH1F*)gROOT->FindObject("AverageElecCalibDifferent");
  if (dummy) {delete dummy;}
  dummy = (TH1F*)gROOT->FindObject("AverageInitialCalibSame");
  if (dummy) {delete dummy;}
  dummy = (TH1F*)gROOT->FindObject("AverageInitialCalibDifferent");
  if (dummy) {delete dummy;}

  /**-----------------------------------------------------------------------------------------------*/
  /**..Histograms containing the data collected by eclCosmicECollectorModule */
  std::vector<std::shared_ptr<TH2F>> EnvsCrys;
  EnvsCrys.push_back(getObjectPtr<TH2F>("EnvsCrysSameRing"));
  EnvsCrys.push_back(getObjectPtr<TH2F>("EnvsCrysDifferentRing"));

  std::vector<std::shared_ptr<TH1F>> ExpEvsCrys;
  ExpEvsCrys.push_back(getObjectPtr<TH1F>("ExpEvsCrysSameRing"));
  ExpEvsCrys.push_back(getObjectPtr<TH1F>("ExpEvsCrysDifferentRing"));

  std::vector<std::shared_ptr<TH1F>> ElecCalibvsCrys;
  ElecCalibvsCrys.push_back(getObjectPtr<TH1F>("ElecCalibvsCrysSameRing"));
  ElecCalibvsCrys.push_back(getObjectPtr<TH1F>("ElecCalibvsCrysDifferentRing"));

  std::vector<std::shared_ptr<TH1F>> InitialCalibvsCrys;
  InitialCalibvsCrys.push_back(getObjectPtr<TH1F>("InitialCalibvsCrysSameRing"));
  InitialCalibvsCrys.push_back(getObjectPtr<TH1F>("InitialCalibvsCrysDifferentRing"));

  std::vector<std::shared_ptr<TH1F>> CalibEntriesvsCrys;
  CalibEntriesvsCrys.push_back(getObjectPtr<TH1F>("CalibEntriesvsCrysSameRing"));
  CalibEntriesvsCrys.push_back(getObjectPtr<TH1F>("CalibEntriesvsCrysDifferentRing"));

  auto RawDigitAmpvsCrys = getObjectPtr<TH2F>("RawDigitAmpvsCrys");
  auto RawDigitTimevsCrys = getObjectPtr<TH2F>("RawDigitTimevsCrys");

  /**-----------------------------------------------------------------------------------------------*/
  /**..Record the number of entries per crystal in each of the two normalized energy  histograms
   and average the constants obtained from DB  */
  TH1F* IntegralVsCrys[2];
  IntegralVsCrys[0] = new TH1F("IntegralVsCrysSame", "Integral of EnVsCrys for each crystal, same theta ring;Crystal ID", 8736, 0,
                               8736);
  IntegralVsCrys[1] = new TH1F("IntegralVsCrysDifferent", "Integral of EnVsCrys for each crystal, different theta rings;Crystal ID",
                               8736, 0, 8736);

  TH1F* AverageExpECrys[2];
  AverageExpECrys[0] = new TH1F("AverageExpECrysSame",
                                "Average expected E per crys from collector, same theta ring;Crystal ID;Energy (GeV)", 8736, 0, 8736);
  AverageExpECrys[1] = new TH1F("AverageExpECrysDifferent",
                                "Average expected E per crys from collector, different theta ring;Crystal ID;Energy (GeV)", 8736, 0, 8736);

  TH1F* AverageElecCalib[2];
  AverageElecCalib[0] = new TH1F("AverageElecCalibSame",
                                 "Average electronics calib const vs crys, same theta ring;Crystal ID;Calibration constant", 8736, 0, 8736);
  AverageElecCalib[1] = new TH1F("AverageElecCalibDifferent",
                                 "Average electronics calib const vs crys, different theta rings;Crystal ID;Calibration constant", 8736, 0, 8736);

  TH1F* AverageInitialCalib[2];
  AverageInitialCalib[0] = new TH1F("AverageInitialCalibSame",
                                    "Average initial cosmic calib const vs crys, same theta ring;Crystal ID;Calibration constant", 8736, 0, 8736);
  AverageInitialCalib[1] = new TH1F("AverageInitialCalibDifferent",
                                    "Average initial cosmic calib const vs crys, different theta rings;Crystal ID;Calibration constant", 8736, 0, 8736);

  for (int crysID = 0; crysID < 8736; crysID++) {
    int histbin = crysID + 1;
    for (int idir = 0; idir < 2; idir++) {
      TH1D* hEnergy = EnvsCrys[idir]->ProjectionY("hEnergy", histbin, histbin);
      int Integral = hEnergy->Integral();
      IntegralVsCrys[idir]->SetBinContent(histbin, Integral);

      double TotEntries = CalibEntriesvsCrys[idir]->GetBinContent(histbin);

      double expectedE = 0.;
      if (TotEntries > 0.) {expectedE = ExpEvsCrys[idir]->GetBinContent(histbin) / TotEntries;}
      AverageExpECrys[idir]->SetBinContent(histbin, expectedE);
      AverageExpECrys[idir]->SetBinError(histbin, 0.);

      double calibconst = 0.;
      if (TotEntries > 0.) {calibconst = ElecCalibvsCrys[idir]->GetBinContent(histbin) / TotEntries;}
      AverageElecCalib[idir]->SetBinContent(histbin, calibconst);
      AverageElecCalib[idir]->SetBinError(histbin, 0);

      calibconst = 0.;
      if (TotEntries > 0.) {calibconst = InitialCalibvsCrys[idir]->GetBinContent(histbin) / TotEntries;}
      AverageInitialCalib[idir]->SetBinContent(histbin, calibconst);
      AverageInitialCalib[idir]->SetBinError(histbin, 0);
    }
  }

  /**-----------------------------------------------------------------------------------------------*/
  /**..Write out the basic histograms in all cases */
  TFile* histfile = new TFile("eclCosmicEAlgorithm.root", "recreate");
  for (int idir = 0; idir < 2; idir++) {
    EnvsCrys[idir]->Write();
    IntegralVsCrys[idir]->Write();
    AverageExpECrys[idir]->Write();
    AverageElecCalib[idir]->Write();
    AverageInitialCalib[idir]->Write();
  }
  RawDigitAmpvsCrys->Write();
  RawDigitTimevsCrys->Write();

  /**-----------------------------------------------------------------------------------------------*/
  /**..If we have not been asked to do fits, we can quit now */
  if (!performFits) {
    B2INFO("eclCosmicEAlgorithm has not been asked to perform fits; copying input histograms and quitting");
    histfile->Close();
    return c_NotEnoughData;
  }

  /**-----------------------------------------------------------------------------------------------*/
  /**..Check that every crystal has enough entries. If we are finding calibration constants (normal data
   mode), at least 1 histogram must have sufficient statistics. If we are finding expected values
   (used with MC), both must have sufficient statistics.  */
  bool sufficientData = true;
  for (int crysID = cellIDLo - 1; crysID < cellIDHi; crysID++) {
    int histbin = crysID + 1;
    bool SameLow = IntegralVsCrys[0]->GetBinContent(histbin) < minEntries;
    bool DifferentLow = IntegralVsCrys[1]->GetBinContent(histbin) < minEntries;
    if ((SameLow && DifferentLow) || (findExpValues && (SameLow || DifferentLow))) {
      if (storeConst == 1) {B2INFO("eclCosmicEAlgorithm: cellID " << histbin << " has insufficient statistics: " << IntegralVsCrys[0]->GetBinContent(histbin) << " and " << IntegralVsCrys[1]->GetBinContent(histbin) << ". Requirement is " << minEntries);}
      sufficientData = false;
      break;
    }
  }

  /**-----------------------------------------------------------------------------------------------*/
  /** Insufficient data. Quit if we are required to have a successful fit for every crystal */
  if (!sufficientData && storeConst == 1) {
    histfile->Close();
    return c_NotEnoughData;
  }

  /**-----------------------------------------------------------------------------------------------*/
  /**..Some prep for the many fits about to follow  */
  TString preName[2] = {"SameRing", "DifferentRing"};

  TH1F* PeakperCrys[2];
  PeakperCrys[0] = new TH1F("PeakperCrysSame", "Fit peak per crystal, same theta ring;Crystal ID;Peak normalized energy", 8736, 0,
                            8736);
  PeakperCrys[1] = new TH1F("PeakperCrysDifferent", "Fit peak per crystal, different theta ring;Crystal ID;Peak normalized energy",
                            8736, 0, 8736);

  TH1F* SigmaperCrys[2];
  SigmaperCrys[0] = new TH1F("SigmaperCrysSame", "Fit sigma per crysal, same theta ring;Crystal ID;Sigma (ADC)", 8736, 0, 8736);
  SigmaperCrys[1] = new TH1F("SigmaperCrysDifferent", "Fit sigma per crysal, different theta ring;Crystal ID;Sigma (ADC)", 8736, 0,
                             8736);

  TH1F* EtaperCrys[2];
  EtaperCrys[0] = new TH1F("EtaperCrysSame", "Fit eta per crysal, same theta ring;Crystal ID;Eta", 8736, 0, 8736);
  EtaperCrys[1] = new TH1F("EtaperCrysDifferent", "Fit eta per crysal, different theta ring;Crystal ID;Eta", 8736, 0, 8736);

  TH1F* ConstperCrys[2];
  ConstperCrys[0] = new TH1F("ConstperCrysSame", "Fit constant per crystal, same theta ring;Crystal ID;Constant", 8736, 0, 8736);
  ConstperCrys[1] = new TH1F("ConstperCrysDifferent", "Fit constant per crystal, different theta ring;Crystal ID;Constant", 8736, 0,
                             8736);

  TH1F* StatusperCrys[2];
  StatusperCrys[0] = new TH1F("StatusperCrysSame", "Fit status, same theta ring;Crystal ID;Status", 8736, 0, 8736);
  StatusperCrys[1] = new TH1F("StatusperCrysDifferent", "Fit status, different theta ring;Crystal ID;Status", 8736, 0, 8736);

  /**..1D summary histograms */
  TH1F* hStatus[2];
  hStatus[0] = new TH1F("StatusSame", "Fit status, same theta ring", 25, -5, 20);
  hStatus[1] = new TH1F("StatusDifferent", "Fit status, different theta ring", 25, -5, 20);

  TH1F* fracPeakUnc[2];
  fracPeakUnc[0] = new TH1F("fracPeakUncSame", "Fractional uncertainty on peak location, same theta ring", 100, 0, 0.1);
  fracPeakUnc[1] = new TH1F("fracPeakUncDifferent", "Fractional uncertainty on peak location, different theta ring", 100, 0, 0.1);

  TH1F* hfitProb[2];
  hfitProb[0] = new TH1F("fitProbSame", "Probability of fit, same theta ring", 200, 0, 0.02);
  hfitProb[1] = new TH1F("fitProbDifferent", "Probability of fit, different theta ring", 200, 0, 0.02);

  /**..Histograms to store results for DB */
  TH1F* ExpEnergyperCrys[2];
  ExpEnergyperCrys[0] = new TH1F("ExpEnergyperCrysSame", "Expected energy per crystal, same theta ring;Crystal ID;Peak energy (GeV)",
                                 8736, 0, 8736);
  ExpEnergyperCrys[1] = new TH1F("ExpEnergyperCrysDifferent",
                                 "Expected energy per crystal, different theta ring;Crystal ID;Peak energy (GeV)", 8736, 0, 8736);

  TH1F* CalibvsCrys = new TH1F("CalibvsCrys", "Energy calibration constant from cosmics;Crystal ID;Calibration constant", 8736, 0,
                               8736);

  /**-----------------------------------------------------------------------------------------------*/
  /**..Loop over specified crystals and performs fits to the two normalized energy distributions */
  bool allFitsOK = true;
  for (int crysID = cellIDLo - 1; crysID < cellIDHi; crysID++) {
    int histbin = crysID + 1;
    for (int idir = 0; idir < 2; idir++) {

      /**..Extract the 1D normalized energy distribution from the appropriate 2D histogram */
      TString hname = preName[idir];
      hname += "Enormalized";
      hname += crysID;
      TH1D* hEnergy = EnvsCrys[idir]->ProjectionY(hname, histbin, histbin);

      /**..Fit function (xmin, xmax, nparameters) for this histogram */
      double histMin = hEnergy->GetXaxis()->GetXmin();
      double histMax = hEnergy->GetXaxis()->GetXmax();
      TF1* func = new TF1("eclCosmicNovoConst", eclCosmicNovoConst, histMin, histMax, 5);
      func->SetParNames("normalization", "peak", "effSigma", "eta", "const");
      func->SetParLimits(1, peakMin, peakMax);
      func->SetParLimits(2, effSigMin, effSigMax);
      func->SetParLimits(3, etaMin, etaMax);

      /**..Estimate initial parameters. For peak, use maximum bin in the allowed range */
      hEnergy->GetXaxis()->SetRangeUser(peakMin, peakMax);
      int maxBin = hEnergy->GetMaximumBin();
      double peakE = hEnergy->GetBinLowEdge(maxBin);
      double peakEUnc = 0.;
      double normalization = hEnergy->GetMaximum();
      double effSigma = hEnergy->GetRMS();
      double sigmaUnc = 0.;
      hEnergy->GetXaxis()->SetRangeUser(histMin, histMax);

      /**..Fit range is histogram low edge plus a few bins to peak + 2.5*effective sigma */
      double fitlow = 0.25;
      double fithigh = peakE + 2.5 * effSigma;

      /**..Constant from lower edge of plot */
      int il0 = hEnergy->GetXaxis()->FindBin(fitlow);
      int il1 = hEnergy->GetXaxis()->FindBin(fitlow + 0.1);
      double constant = hEnergy->Integral(il0, il1) / (1 + il1 - il0);
      double constUnc = 0.;

      /**..Eta is nominal */
      double eta = etaNom;
      double etaUnc = 0.;

      /**..parameters to control iterations.  dIter checks if we are stuck in a loop */
      double dIter = 0.1 * (histMax - histMin) / hEnergy->GetNbinsX();
      double highold(0.), higholdold(0.);
      double fitProb(0.);
      double fitProbDefault(0.);
      bool fitHist = IntegralVsCrys[idir]->GetBinContent(histbin) >= minEntries; /* fit only if enough events */
      bool fixConst = false;
      int nIter = 0;

      /**-----------------------------------------------------------------------------------------------*/
      /**..Iterate from this point if needed */
      while (fitHist) {
        nIter++;

        /**..Set the initial parameters */
        func->SetParameters(normalization, peakE, effSigma, eta, constant);
        if (fixConst) { func->FixParameter(4, 0); }

        /**..Perform the fit and note the resulting parameters */
        hEnergy->Fit(func, "LIQ", "", fitlow, fithigh);
        normalization = func->GetParameter(0);
        peakE = func->GetParameter(1);
        peakEUnc = func->GetParError(1);
        effSigma = func->GetParameter(2);
        sigmaUnc = func->GetParError(2);
        eta = func->GetParameter(3);
        etaUnc = func->GetParError(3);
        constant = func->GetParameter(4);
        constUnc = func->GetParError(4);
        fitProbDefault = func->GetProb();

        /**..The upper fit range should correspond to 20-25% of the peak. Iterate if necessary. */
        fitHist = false;
        double peak = func->Eval(peakE) - constant;
        double tRatio = (func->Eval(fithigh) - constant) / peak;
        if (tRatio < tRatioMin || tRatio > tRatioMax) {
          double targetY = constant + 0.5 * (tRatioMin + tRatioMax) * peak;
          higholdold = highold;
          highold = fithigh;
          fithigh = func->GetX(targetY, peakE, histMax);
          fitHist = true;

          /**..Check if we are oscillating between two end points */
          if (abs(fithigh - higholdold) < dIter) {fithigh = 0.5 * (highold + higholdold); }

          /**..Many iterations may mean we are stuck in a loop. Try a different end point. */
          if (nIter > maxIterations - 3) {fithigh = 0.33333 * (fithigh + highold + higholdold); }
        }

        /**..Set the constant term to 0 if we are close to the limit */
        if (constant < constTol && !fixConst) {
          constant = 0;
          fixConst = true;
          fitHist = true;
        }

        /**..No more than specified number of iterations */
        if (nIter == maxIterations) {fitHist = false;}
        B2DEBUG(200, "cellID = " << histbin << " " << nIter << " " << preName[idir] << " " << peakE << " " << constant << " " << tRatio <<
                " " << fithigh);

      }

      /**-----------------------------------------------------------------------------------------------*/
      /**..Calculate fit probability. Same as P option in fit, which cannot be used with L */
      fitProb = 0.;
      if (nIter > 0) {
        int lowbin = hEnergy->GetXaxis()->FindBin(fitlow);
        int highbin = hEnergy->GetXaxis()->FindBin(fithigh);
        int npar = 5;
        if (fixConst) {npar = 4;}
        int ndeg = (highbin - lowbin) + 1 - npar;
        double chisq = 0.;
        double halfbinwidth = 0.5 * hEnergy->GetBinWidth(1);
        for (int ib = lowbin; ib <= highbin; ib++) {
          double yexp = func->Eval(hEnergy->GetBinLowEdge(ib) + halfbinwidth);
          double yobs = hEnergy->GetBinContent(ib);
          double dchi2 = (yexp - yobs) * (yexp - yobs) / yexp;
          chisq += dchi2;
        }
        fitProb = 0.5 * (TMath::Prob(chisq, ndeg) + fitProbDefault);
      }

      /**-----------------------------------------------------------------------------------------------*/
      /**..Fit status */
      int iStatus = fitOK; // success
      if (nIter == maxIterations) {iStatus = iterations;} // too many iterations

      /** No peak; normalization of Novo component is too small */
      if (normalization < constRatio * constant) {iStatus = noPeak;}

      /**..poor fit, or relatively poor fit with too many iterations */
      if (fitProb <= minFitLimit || (fitProb < minFitProbIter && iStatus == iterations)) {iStatus = poorFit;}

      /**..parameter at limit */
      if ((peakE < peakMin + peakTol) || (peakE > peakMax - peakTol)) {iStatus = atLimit;}
      if ((effSigma < effSigMin + effSigTol) || (effSigma > effSigMax - effSigTol)) {iStatus = atLimit;}
      if ((eta < etaMin + etaTol) || (eta > etaMax - etaTol)) {iStatus = atLimit;}

      //**..No fit
      if (nIter == 0) {iStatus = notFit;} // not fit

      /**..Store the fit results */
      PeakperCrys[idir]->SetBinContent(histbin, peakE);
      PeakperCrys[idir]->SetBinError(histbin, peakEUnc);
      SigmaperCrys[idir]->SetBinContent(histbin, effSigma);
      SigmaperCrys[idir]->SetBinError(histbin, sigmaUnc);
      EtaperCrys[idir]->SetBinContent(histbin, eta);
      EtaperCrys[idir]->SetBinError(histbin, etaUnc);
      ConstperCrys[idir]->SetBinContent(histbin, constant);
      ConstperCrys[idir]->SetBinError(histbin, constUnc);
      StatusperCrys[idir]->SetBinContent(histbin, iStatus);
      hStatus[idir]->Fill(iStatus);
      fracPeakUnc[idir]->Fill(peakEUnc / peakE);
      hfitProb[idir]->Fill(fitProb);

      /**..Write out the fit distribution */
      B2INFO("cellID " << histbin << " " << preName[idir] << " status = "  << iStatus << " fit probability = " << fitProb);
      histfile->cd();
      hEnergy->Write();
    }
  }

  /**-----------------------------------------------------------------------------------------------*/
  /**..Find expected energies from MC, if requested */
  if (findExpValues) {

    /**..Write out expected energies if status is adequate. Check that every crystal has at least one good fit */
    for (int crysID = 0; crysID < 8736; crysID++) {
      int histbin = crysID + 1;
      bool atLeastOneOK = false;
      for (int idir = 0; idir < 2; idir++) {
        double fitstatus = StatusperCrys[idir]->GetBinContent(histbin);
        double peakE = PeakperCrys[idir]->GetBinContent(histbin);
        double peakEUnc = PeakperCrys[idir]->GetBinError(histbin);

        //**..For failed fits, store the negative of the input expected energy */
        if (fitstatus < iterations) {
          if (histbin >= cellIDLo && histbin <= cellIDHi) {
            B2INFO("eclCosmicEAlgorithm: crystal " << crysID << " " << preName[idir] << " is not a successful fit. Status = " << fitstatus);
          }
          peakE = -1.;
          peakEUnc = 0.;
        } else {
          atLeastOneOK = true;
        }
        double inputExpE = abs(AverageExpECrys[idir]->GetBinContent(histbin));
        ExpEnergyperCrys[idir]->SetBinContent(histbin, inputExpE * peakE);
        ExpEnergyperCrys[idir]->SetBinError(histbin, inputExpE * peakEUnc / peakE);
      }
      if (!atLeastOneOK) {allFitsOK = false;}
    }

    /**-----------------------------------------------------------------------------------------------*/
    /**..Otherwise, find calibration constants */
  } else {

    /**..Find calibration constant separately for the two normalized energy distributions for each crystal */
    for (int crysID = 0; crysID < 8736; crysID++) {
      int histbin = crysID + 1;
      double calibConst[2] = {};
      double calibConstUnc[2] = {999999., 999999.};
      double weight[2] = {};
      for (int idir = 0; idir < 2; idir++) {

        /**..Peak and uncertainty; assume uncertainties on expected energy and elec calib are negligible */
        double peakE = PeakperCrys[idir]->GetBinContent(histbin);
        double fracPeakEUnc = PeakperCrys[idir]->GetBinError(histbin) / peakE;
        double inputConst = AverageInitialCalib[idir]->GetBinContent(histbin);
        double fitstatus = StatusperCrys[idir]->GetBinContent(histbin);
        double inputExpE = AverageExpECrys[idir]->GetBinContent(histbin);
        if (fitstatus >= iterations && inputConst == 0) {B2FATAL("eclCosmicEAlgorithm: input calibration = 0 for idir = " << idir << " and crysID = " << crysID);}

        //** Find constant only if fit was successful and we have a value for the expected energy */
        if (fitstatus >= iterations && inputExpE > 0.) {
          calibConst[idir] = abs(inputConst) / peakE;
          calibConstUnc[idir] = calibConst[idir] * fracPeakEUnc / peakE;
          weight[idir] = 1. / (calibConstUnc[idir] * calibConstUnc[idir]);
        }
        if (fitstatus < iterations && histbin >= cellIDLo && histbin <= cellIDHi) {
          B2INFO("eclCosmicEAlgorithm: cellID " << histbin << " " << preName[idir] << " is not a successful fit. Status = " << fitstatus);
        } else if (inputExpE < 0. && histbin >= cellIDLo && histbin <= cellIDHi) {
          B2INFO("eclCosmicEAlgorithm: cellID " << histbin << " " << preName[idir] << " has no expected energy. Status = " << fitstatus);
        }
      }


      /**..Find the weighted average of the two constants and store in the histogram */
      double averageConst;
      double averageConstUnc;

      /**..If both fits failed, use the negative of the initial "same" calibration constant */
      if (weight[0] == 0 && weight[1] == 0) {
        if (histbin >= cellIDLo && histbin <= cellIDHi) {B2INFO("eclCosmicEAlgorithm: no constant found for cellID = " << histbin << " status = " << StatusperCrys[0]->GetBinContent(histbin) << " and " << StatusperCrys[1]->GetBinContent(histbin));}
        averageConst = -1.*abs(AverageInitialCalib[0]->GetBinContent(histbin));
        averageConstUnc = 0.;
      } else {
        averageConst = (calibConst[0] * weight[0] + calibConst[1] * weight[1]) / (weight[0] + weight[1]);
        averageConstUnc = 1. / sqrt(weight[0] + weight[1]);
      }
      CalibvsCrys->SetBinContent(histbin, averageConst);
      CalibvsCrys->SetBinError(histbin, averageConstUnc);
    }
  }

  /**-----------------------------------------------------------------------------------------------*/
  /**..Write output to DB if requested and successful */
  bool DBsuccess = false;
  if (storeConst == 0 || (storeConst == 1 && allFitsOK)) {
    DBsuccess = true;

    /**..Store expected energy for each crystal and neighbour type from CRY MC */
    if (findExpValues) {
      std::vector<std::string> DBname = {"ECLExpCosmicESame", "ECLExpCosmicEDifferent"};
      for (int idir = 0; idir < 2; idir++) {
        std::vector<float> tempE;
        std::vector<float> tempUnc;
        for (int crysID = 0; crysID < 8736; crysID++) {
          int histbin = crysID + 1;
          tempE.push_back(ExpEnergyperCrys[idir]->GetBinContent(histbin));
          tempUnc.push_back(ExpEnergyperCrys[idir]->GetBinError(histbin));
        }
        ECLCrystalCalib* ExpectedE = new ECLCrystalCalib();
        ExpectedE->setCalibVector(tempE, tempUnc);
        saveCalibration(ExpectedE, DBname[idir]);
        B2INFO("eclCosmicEAlgorithm: successfully stored expected values for " << DBname[idir]);
      }

      /**..Store calibration constant for each crystal (nominally real data) */
    } else {
      std::vector<float> tempCalib;
      std::vector<float> tempCalibUnc;
      for (int crysID = 0; crysID < 8736; crysID++) {
        int histbin = crysID + 1;
        tempCalib.push_back(CalibvsCrys->GetBinContent(histbin));
        tempCalibUnc.push_back(CalibvsCrys->GetBinError(histbin));
      }
      ECLCrystalCalib* CosmicECalib = new ECLCrystalCalib();
      CosmicECalib->setCalibVector(tempCalib, tempCalibUnc);
      saveCalibration(CosmicECalib, "ECLCrystalEnergyCosmic");
      B2INFO("eclCosmicEAlgorithm: successfully stored calibration constants");
    }
  }

  /**..Write out some diagnostic histograms */
  for (int idir = 0; idir < 2; idir++) {
    PeakperCrys[idir]->Write();
    SigmaperCrys[idir]->Write();
    EtaperCrys[idir]->Write();
    ConstperCrys[idir]->Write();
    StatusperCrys[idir]->Write();
    hStatus[idir]->Write();
    fracPeakUnc[idir]->Write();
    hfitProb[idir]->Write();
  }

  /**..Histograms containing values written to DB */
  if (findExpValues) {
    ExpEnergyperCrys[0]->Write();
    ExpEnergyperCrys[1]->Write();
  } else {
    CalibvsCrys->Write();
  }
  histfile->Close();

  /**-----------------------------------------------------------------------------------------------*/
  /**..Clean up histograms in case Algorithm is called again */
  dummy = (TH1F*)gROOT->FindObject("PeakperCrysSame"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("SigmaperCrysSame"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("EtaperCrysSame"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("ConstperCrysSame"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("StatusperCrysSame"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("StatusSame"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("fracPeakUncSame"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("fitProbSame"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("ExpEnergyperCrysSame"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("PeakperCrysDifferent"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("SigmaperCrysDifferent"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("EtaperCrysDifferent"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("ConstperCrysDifferent"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("StatusperCrysDifferent"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("StatusDifferent"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("fracPeakUncDifferent"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("fitProbDifferent"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("ExpEnergyperCrysDifferent"); delete dummy;
  dummy = (TH1F*)gROOT->FindObject("CalibvsCrys"); delete dummy;

  /**-----------------------------------------------------------------------------------------------*/
  /**..Set the return code appropriately */
  if (storeConst == -1) {
    B2INFO("eclCosmicEAlgorithm performed fits but was not asked to store contants");
    return c_Failure;
  } else if (!DBsuccess) {
    if (findExpValues) { B2INFO("eclCosmicEAlgorithm: failed to store expected values"); }
    else { B2INFO("eclCosmicEAlgorithm: failed to store calibration constants"); }
    return c_Failure;
  }
  return c_OK;
}
