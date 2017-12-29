
#include <ecl/calibration/eclMuMuEAlgorithm.h>

#include "TH2F.h"
#include "TFile.h"
#include "TMath.h"
#include "TF1.h"
#include "TMinuit.h"
#include "TROOT.h"

using namespace std;
using namespace Belle2;

//..Novosibirsk function, plus constant
//  H. Ikeda et al., Nuclear Instruments and Methods A 441 (2000) 401-426
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


eclMuMuEAlgorithm::eclMuMuEAlgorithm(): CalibrationAlgorithm("eclMuMuECollector"), cellIDLo(0), cellIDHi(8735), minEntries(200),
  maxIterations(10), tRatioMin(0.2), tRatioMax(0.25), performFits(true)
{
  setDescription(
    "Perform energy calibration of ecl crystals by fitting a Novosibirsk function to energy deposited by muons"
  );
}

CalibrationAlgorithm::EResult eclMuMuEAlgorithm::calibrate()
{
  //..ranges of various fit parameters, and tolerance to determine that fit is at the limit
  double limitTol = 0.00001;
  double peakMin(3200.), peakMax(4400.); /*< range for peak of measured energy distribution */
  double peakTol = limitTol * (peakMax - peakMin); /*< fit is at limit if it is within peakTol of min or max */
  double effSigMin(150.), effSigMax(450.); /*< range for effective sigma of measured energy distribution */
  double effSigTol = limitTol * (effSigMax - effSigMin);
  double etaMin(-1.5), etaMax(1.); /*< Novosibirsk tail parameter range */
  double etaTol = limitTol * (etaMax - etaMin);
  double constMin(0.), constMax(10.); /*< constant term in energy distribution */
  double constTol = limitTol * constMax; /*< if constant is less than constTol, it will be fixed to 0 */

  //..Keep track of the fit results on the preceeding crystal as a measure of the true uncertainty
  double previousPeak(0.), previousUnc(0.);

  //..Put root into batch mode so that we don't try to open a graphics window
  gROOT->SetBatch();

  //..Histograms containing the data collected by eclMuMuECollectorModule
  auto EmuVsCellID0 = getObjectPtr<TH2F>("EmuVsCellID0");
  auto MuonLabPvsCellID0 = getObjectPtr<TH1F>("MuonLabPvsCellID0");

  //..Output histogram
  TH1F* CalibVsCellID0 = new TH1F("CalibVsCellID0", "Calibration constant vs cellID;cellID from 0;counts per GeV", 8736, 0, 8736);

  //..Diagnostic histograms
  TH1F* PeakVsCellID0 = new TH1F("PeakVsCellID0", "Peak of Novo fit vs cellID;cellID from 0;Peak of muon fit (digits)", 8736, 0,
                                 8736);
  TH1F* PeakUncVsCellID0 = new TH1F("PeakUncVsCellID0",
                                    "Uncertainty on peak of Novo fit vs cellID;cellID from 0;Uncertainty of muon fit (digits)", 8736, 0, 8736);
  TH1F* effSigVsCellID0 = new TH1F("effSigVsCellID0", "effSigma vs cellID;cellID from 0;effSigma (digits)", 8736, 0, 8736);
  TH1F* etaVsCellID0 = new TH1F("etaVsCellID0", "eta vs cellID;cellID from 0;Novo eta parameter", 8736, 0, 8736);
  TH1F* constVsCellID0 = new TH1F("constVsCellID0", "fit constant vs cellID;cellID from 0;fit constant", 8736, 0, 8736);
  TH1F* normVsCellID0 = new TH1F("normVsCellID0", "fit normalization vs cellID;cellID from 0;fit normalization", 8736, 0, 8736);
  TH1F* fitLimitVsCellID0 = new TH1F("fitLimitVsCellID0", "fit range upper limit vs cellID;cellID from 0;upper fit limit", 8736, 0,
                                     8736);
  TH1F* StatusVsCellID0 = new TH1F("StatusVsCellID0", "Fit status vs cellID;cellID from 0;Fit status", 8736, 0, 8736);
  TH1F* EntriesVsCellID0 = new TH1F("EntriesVsCellID0", "Entries for each fit crystal;cellID from 0", 8736, 0, 8736);
  TH1F* IntegralVsCellID0 = new TH1F("IntegralVsCellID0", "Integral of EmuVsCellID0 for each cellID;cellID from 0;Entries", 8736, 0,
                                     8736);
  TH1F* dPeakPull = new TH1F("dPeakPull", "Difference in adjacent peaks over unc;Pull in dPeak", 100, -10, 10);
  TH1F* hPeak = new TH1F("hPeak", "Novo fit peak;Peak", 200, 3600, 4000);
  TH1F* hPeakUnc = new TH1F("hPeakUnc", "Novo fit peak uncertainty;Uncertainty on peak", 100, 0, 100);
  TH1F* nIterations = new TH1F("nIterations", "Number of times histogram was fit;Number of iterations", 20, -0.5, 19.5);


  //..Record integral (excluding underflows and overflows) from eclEMuMuCollector per crystal
  int ny = EmuVsCellID0->GetNbinsY();
  int histEntries = 999999;
  for (int ID = 0; ID < 8736; ID++) {
    int IDInt = EmuVsCellID0->Integral(ID + 1, ID + 1, 1, ny);
    IntegralVsCellID0->SetBinContent(ID + 1, IDInt);
    if (ID >= cellIDLo && ID <= cellIDHi && IDInt < histEntries) {histEntries = IDInt; }
  }

  //..Root file for diagnostic output histograms
  TFile* histfile = new TFile("eclMuMuEAlgorithm.root", "recreate");

  //..Write out the basic histograms in all cases
  IntegralVsCellID0->Write();
  EmuVsCellID0->Write();
  MuonLabPvsCellID0->Write();

  //..If we are not fitting, we can quit now
  if (!performFits) {
    histfile->Close();
    B2RESULT("eclMuMuEAlgorithm has not been asked to perform fits; copying input histograms and quitting");
    return c_OK;
  }

  //..Check that we have enough data to continue
  B2RESULT("eclMuMuEAlgorithm: minimum number of entries among all crystals to be fit is " << histEntries << "; must have at least "
           << minEntries << " to fit");
  if (histEntries < minEntries) {
    histfile->Close();
    return c_NotEnoughData;
  }

  //..Loop over specified crystals
  double histMin = EmuVsCellID0->GetYaxis()->GetXmin();
  double histMax = EmuVsCellID0->GetYaxis()->GetXmax();
  for (int ID = cellIDLo; ID <= cellIDHi; ID++) {
    int ID100 = ID / 100;
    ID100 = 100 * ID100;

    //.. Project 1D histogram of energy in this crystal
    TString name = "DigitAmplitude";
    name += ID;
    TH1D* hEnergy = EmuVsCellID0->ProjectionY(name, ID + 1, ID + 1);

    //..Fit function (xmin, xmax, nparameters) for this histogram
    TF1* func = new TF1("eclNovoConst", eclNovoConst, histMin, histMax, 5);
    func->SetParNames("normalization", "peak", "effSigma", "eta", "const");
    func->SetParLimits(1, peakMin, peakMax);
    func->SetParLimits(2, effSigMin, effSigMax);
    func->SetParLimits(3, etaMin, etaMax);
    func->SetParLimits(4, constMin, constMax);

    //..Estimate initial parameters from the histogram. For peak, use maximum bin
    int maxBin = hEnergy->GetMaximumBin();
    double peakE = hEnergy->GetBinLowEdge(maxBin);
    if (peakE < peakMin) {peakE = peakMin;}
    if (peakE > peakMax) {peakE = peakMax;}
    double normalization = hEnergy->GetMaximum();

    //..The first ≥2 bin followed by two additional ≥2 bins is the start of the distribution.
    int startBin = 1;
    int lastBin = hEnergy->GetNbinsX();
    while ((hEnergy->GetBinContent(startBin) < 2 || hEnergy->GetBinContent(startBin + 1) < 2
            || hEnergy->GetBinContent(startBin + 2) < 2) && startBin < lastBin) {
      startBin++;
    }
    double startE = hEnergy->GetBinLowEdge(startBin);

    //..Estimate effective sigma from start and peak
    double initialPeakStart = peakE - startE;
    double effSigma = initialPeakStart / 1.3;
    if (effSigma < effSigMin) {effSigma = effSigMin;}
    if (effSigma > effSigMax) {effSigma = effSigMax;}

    //..eta and constant are nominal values
    double eta = -0.5;
    double constant = 0.01;

    //..Fit range is histogram low edge to peak + 2.5*effective sigma
    double fitlow = histMin;
    double fithigh = peakE + 2.5 * effSigma;
    double highold(0.), higholdold(0.);

    //..Iterate from this point if needed
    bool fitHist = true;
    bool fixConst = false;
    int nIter = 0;
    while (performFits && fitHist) {

      //..Set the initial parameters
      func->SetParameters(normalization, peakE, effSigma, eta, constant);
      if (fixConst) { func->FixParameter(4, 0); }

      //..Fit (temp)
      hEnergy->Fit(func, "LIQ", "", fitlow, fithigh);
      nIter++;
      fitHist = false;
      normalization = func->GetParameter(0);
      peakE = func->GetParameter(1);
      effSigma = func->GetParameter(2);
      eta = func->GetParameter(3);
      constant = func->GetParameter(4);
      if (ID == ID100) {B2INFO(ID << " " << nIter << " " << peakE << " " << constant << " " << fithigh);}

      //..The upper fit range should correspond to 20-25% of the peak. Iterate if necessary.
      double peak = func->Eval(peakE);
      double tRatio = func->Eval(fithigh) / peak;
      if (tRatio < tRatioMin || tRatio > tRatioMax) {
        double targetY = 0.5 * (tRatioMin + tRatioMax) * peak;
        higholdold = highold;
        highold = fithigh;
        fithigh = func->GetX(targetY, peakE, histMax);
        fitHist = true;

        //..Check if we are oscillating between two end points
        if (abs(fithigh - higholdold) < 1) {fithigh = 0.5 * (highold + higholdold); }

        //..Many iterations may mean we are stuck in a loop. Try a different end point.
        if (nIter > maxIterations - 3) {fithigh = 0.33333 * (fithigh + highold + higholdold); }
      }

      //..Set the constant term to 0 if we are close to the limit
      if (constant < constTol && !fixConst) {
        constant = 0;
        fixConst = true;
        fitHist = true;
      }

      //..No more than specified number of iterations
      if (nIter == maxIterations) {fitHist = false;}
    }

    //..Fit status
    int iStatus = 9;
    if (nIter == maxIterations) {iStatus = 2;}

    if ((peakE < peakMin + peakTol) || (peakE > peakMax - peakTol)) {iStatus = 3;}
    if ((effSigma < effSigMin + effSigTol) || (effSigma > effSigMax - effSigTol)) {iStatus = 3;}
    if ((eta < etaMin + etaTol) || (eta > etaMax - etaTol)) {iStatus = 3;}
    if (constant > constMax - constTol) {iStatus = 3;}

    //..Peak of true energy deposition in crystal depends on log(P) of muon due to relativistic rise.
    //  Hardwired for now. Hearty, 17-Jan-2017
    double trueConst = 0.1799;
    double trueSlope = 0.005838;
    int histbin = PeakVsCellID0->GetXaxis()->FindBin(ID + 0.001);
    double LogMuonPlab = log(MuonLabPvsCellID0->GetBinContent(histbin));
    double peakTrueE = trueConst + trueSlope * LogMuonPlab;

    //..Calibration constant is ratio of peak of Novo fit over predicted peak true energy
    double CalibConst = peakE / peakTrueE;
    double peakEUnc = func->GetParError(1);
    double CalibConstUnc = peakEUnc / peakTrueE;
    CalibVsCellID0->SetBinContent(histbin, CalibConst);
    CalibVsCellID0->SetBinError(histbin, CalibConstUnc);

    //..diagnostic histograms
    PeakVsCellID0->SetBinContent(histbin, peakE);
    PeakVsCellID0->SetBinError(histbin, 0);
    PeakUncVsCellID0->SetBinContent(histbin, peakEUnc);
    PeakUncVsCellID0->SetBinError(histbin, 0);
    effSigVsCellID0->SetBinContent(histbin, effSigma);
    effSigVsCellID0->SetBinError(histbin, func->GetParError(2));
    etaVsCellID0->SetBinContent(histbin, eta);
    etaVsCellID0->SetBinError(histbin, func->GetParError(3));
    constVsCellID0->SetBinContent(histbin, constant);
    constVsCellID0->SetBinError(histbin, func->GetParError(4));
    normVsCellID0->SetBinContent(histbin, func->GetParameter(0));
    normVsCellID0->SetBinError(histbin, func->GetParError(0));
    fitLimitVsCellID0->SetBinContent(histbin, fithigh);
    fitLimitVsCellID0->SetBinError(histbin, 0);
    StatusVsCellID0->SetBinContent(histbin, iStatus);
    StatusVsCellID0->SetBinError(histbin, 0);
    EntriesVsCellID0->SetBinContent(histbin, hEnergy->GetEntries());
    nIterations->Fill(nIter);
    hPeak->Fill(peakE);
    hPeakUnc->Fill(peakEUnc);

    double dPeak = peakE - previousPeak;
    double dPeakUnc = sqrt(peakEUnc * peakEUnc + previousUnc * previousUnc);
    if (previousPeak > 1) {dPeakPull->Fill(dPeak / dPeakUnc);}
    previousPeak = peakE;
    previousUnc = peakEUnc;


    //..Output to log file if status is not nominal
    if (iStatus != 9) {
      B2INFO("eclMuMuEAlgorithm fit: " << ID << " " << iStatus << " " << nIter << " " << peakE << " " << peakEUnc << " " << CalibConst <<
             " " << CalibConstUnc);
    }

    //..Store histogram with fit (temp)
    if (performFits) {
      histfile->cd();
      hEnergy->Write();
    }
  }

  histfile->cd();
  if (performFits) {
    PeakVsCellID0->Write();
    PeakUncVsCellID0->Write();
    effSigVsCellID0->Write();
    etaVsCellID0->Write();
    constVsCellID0->Write();
    normVsCellID0->Write();
    fitLimitVsCellID0->Write();
    StatusVsCellID0->Write();
    EntriesVsCellID0->Write();
    dPeakPull->Write();
    hPeak->Write();
    hPeakUnc->Write();
    nIterations->Write();
    CalibVsCellID0->Write();
  }
  histfile->Close();

  //..Actual CAF results
  B2RESULT("eclMuMuEAlgorithm Calibration Successful");
  saveCalibration(CalibVsCellID0, getPrefix());

  return c_OK;
}
