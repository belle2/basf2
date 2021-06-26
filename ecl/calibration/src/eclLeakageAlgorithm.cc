#include <ecl/calibration/eclLeakageAlgorithm.h>
#include <ecl/dbobjects/ECLLeakageCorrections.h>


#include "TH1D.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TDirectory.h"
#include <iostream>

using namespace std;
using namespace Belle2;
using namespace ECL;
using namespace Calibration;

/**************************************************************************
 * eclLeakageAlgorithm analyzes single photon MC to find the eclLeakage payload
 * After initial setup, the position dependent corrections are found using
 * photons with good reconstruction. This correction is then applied prior
 * to finding the correction that depends on the number of crystals.
 *
 * Major steps in the process:
 * 1. Set up (read in parameters and tree, define histogram binning)
 * 2. Loop through data to histogram difference between true and reconstructed
 *    location. Determine cut for each thetaID/energy, used to select photons for
 *    all future steps.
 * 3. Fill and fit histograms of normalized reconstructed energy, one per
 *    thetaID/energy. Novosibirsk fit parameter eta is floated in this fit, then
 *    fixed for fits of data with finer location binning. Peak (i.e. overall correction
 *    for this thetaID/energy) is used to normalize position-dependent correction.
 * 4. Fill and fit histograms of normalized energy for each location (nominally 29
 *    locations in theta and phi per thetaID) to get the position-dependent correction.
 *    Nominally 69 x 8 x 29 x 3 = 48,024 histograms. 3 = theta, phi next to mech, phi not
 *    next to mech.
 * 5. Fill and fit histograms of normalized reconstructed energy corrected for position
 *    dependent leakage, one per thetaID/energy. Resulting eta used for nCrys fits.
 * 6. Fill and fit histograms of normalized energy corrected for location for each value
 *    of nCrys, for each thetaID / energy (nominally 69 x 8 x 21 = 11,592 histograms).
 *    Fix up corrections for those values of nCrys without sufficient statistics.
 * 7. Pack payload quantities into vectors and histograms
 * 8. Fill and fit summary and resolution histograms
 * 9. Finish; close histogram file and and write payload

 **************************************************************************/


/**-------------------------------------------------------------------------------------*/
//..Function to get starting parameters for the Novo fit
std::vector<double> eclLeakageFitParameters(TH1F* h, const double& target)
{

  //..Find the smallest range of bins that contain at least "target" entries
  double maxIntegral = h->Integral();
  const int nBins = h->GetNbinsX();
  int minLo = 1;
  int minHi = nBins;

  //..Store a vector of integrals over histogram ranges so that I only need to
  //  call h->Integral nBins times, instead of nBins^2.
  std::vector<double> intVector;  // intVector[n] = sum of histogram bins [1,n+1] inclusive
  intVector.push_back(h->GetBinContent(1));
  for (int iLo = 2; iLo <= nBins; iLo++) {
    double nextIntegral = intVector[iLo - 2] + h->GetBinContent(iLo);
    intVector.push_back(nextIntegral);
  }

  //..Now just look through all possible bin ranges to find the best one
  for (int iLo = 2; iLo <= nBins; iLo++) {
    for (int iHi = iLo; iHi <= nBins; iHi++) {

      //..sum[iLo, iHi] = sum[1, iHi] - sum[1,iLo-1] = intVector[iHi-1] - intVector[iLo-2]
      double integral = intVector[iHi - 1] - intVector[iLo - 2];

      //..same number of bins, pick the pair with more entries
      if ((integral > target and (iHi - iLo) < (minHi - minLo)) or
          (integral > target and (iHi - iLo) == (minHi - minLo) and integral > maxIntegral)
         ) {
        minLo = iLo;
        minHi = iHi;
        maxIntegral = integral;
      }
    }
  }

  //..Fit parameters are derived (in part) from this range of bins
  const double lowE = h->GetBinLowEdge(minLo);
  const double highE = h->GetBinLowEdge(minHi + 1);
  const double peak = 0.5 * (lowE + highE);
  const double sigma = 0.4 * (highE - lowE);
  const double eta = 0.4; // typical value
  const int nfitBins = (1 + minHi - minLo);

  std::vector<double> parameters;
  parameters.push_back(peak);
  parameters.push_back(sigma);
  parameters.push_back(eta);
  parameters.push_back(lowE);
  parameters.push_back(highE);
  parameters.push_back(nfitBins);

  return parameters;
}

/**-------------------------------------------------------------------------------------*/
//..Novosibirsk; H. Ikeda et al. / NIM A 441 (2000) 401-426
double eclLeakageNovo(Double_t* x, Double_t* par)
{

  Double_t peak = par[1];
  Double_t width = par[2];
  Double_t sln4 = sqrt(log(4));
  Double_t y = par[3] * sln4;
  Double_t tail = -log(y + sqrt(1 + y * y)) / sln4;
  Double_t qc = 0.;

  if (TMath::Abs(tail) < 1.e-7)
    qc = 0.5 * TMath::Power(((x[0] - peak) / width), 2);
  else {
    double qa = tail * sqrt(log(4.));
    double qb = sinh(qa) / qa;
    double qx = (x[0] - peak) / width * qb;
    double qy = 1. + tail * qx;

    if (qy > 1.E-7)
      qc = 0.5 * (TMath::Power((log(qy) / tail), 2) + tail * tail);
    else
      qc = 15.0;
  }

  return par[0] * exp(-qc);
}


/**-------------------------------------------------------------------------------------*/
//..Function to check for bad fits
// 0 = good, 1 = redo fit, 2 = lowStat, 3 = lowProb, 4 = peakAtLimit, 5 = sigmaAtLimit, 6 = etaAtLimit
int eclLeakageFitQuality(const double& fitLo, const double& fitHi, const double& fitPeak, const double& fitSigma,
                         const double& fitdEta, const double& fitProb)
{
  const double tolerance = 0.02; /**< parameter is within this fraction of range of limit */
  const double redoFitProb = 1e-5; /**< may refit hist if prob is below this value */
  const double minFitProb = 1e-9; /**< fit prob below this is bad */

  int fitStatus = 0;
  if (fitProb < redoFitProb) {fitStatus = 1;}
  if (fitProb < minFitProb) {fitStatus = 3;}
  double tol = tolerance * (fitHi - fitLo);
  if (fitPeak < (fitLo + tol) or fitPeak > (fitHi - tol)) {fitStatus = 4;}
  if (fitSigma<tol or fitSigma>(fitHi - fitLo - tol)) {fitStatus = 5;}
  double tolEta = tolerance;
  if (fitdEta < tolEta) {fitStatus = 6;} // just use the tolerance for eta (note that it is eta - limit that is passed)
  return fitStatus;
}

/**-------------------------------------------------------------------------------------*/
eclLeakageAlgorithm::eclLeakageAlgorithm(): CalibrationAlgorithm("eclLeakageCollector")
{
  setDescription(
    "Generate payload ECLLeakageCorrection from single photon MC recorded by eclLeakageCollectorModule"
  );
}



/**-------------------------------------------------------------------------------------*/
CalibrationAlgorithm::EResult eclLeakageAlgorithm::calibrate()
{

  //====================================================================================
  //====================================================================================
  //..Step 1. Set up prior to first loop through data
  B2INFO("starting eclLeakageAlgorithm");

  //-----------------------------------------------------------------------------------
  //..Read in histograms created by the collector and fix normalization
  auto inputParameters = getObjectPtr<TH1F>("inputParameters");
  int lastBin = inputParameters->GetNbinsX();
  double scale = inputParameters->GetBinContent(lastBin); // number of times inputParameters was filled
  for (int ib = 1; ib < lastBin; ib++) {
    double param = inputParameters->GetBinContent(ib);
    inputParameters->SetBinContent(ib, param / scale);
    inputParameters->SetBinError(ib, 0.);
  }

  //..And write to disk.
  TFile* histfile = new TFile("eclLeakageAlgorithm.root", "recreate");
  inputParameters->Write();

  //..Parameters
  const int nThetaID = 69; /**< number of thetaID */
  const int nLeakReg = 3; /**< 0 = forward, 1 = barrel, 2 = backward */
  const int firstBarrelID = 13; /**< first thetaID of the barrel */
  const int lastBarrelID = 58; /**< last thetaID of the barrel */
  const int firstUsefulThID = 3; /**< first thetaID to find leakage corrections */
  const int lastUsefulThID = 66; /**< last thetaID to find leakage correction */

  const int nPositions = (int)(inputParameters->GetBinContent(1) + 0.001); /**< locations across crystal */
  const int nEnergies = (int)(inputParameters->GetBinContent(2) + 0.001); /**< number of energy points */
  const int nbinX = nEnergies * nThetaID; /**< number of thetaID/energy combinations */

  const double etaMin = -5.; /**< minimum allowed eta in Novosibirsk fits */
  const double etaMax = 5.; /**< maximum allowed eta */


  //..Energies
  float generatedE[nLeakReg][nEnergies]; // 3 regions forward barrel backward
  int bin = 2; // bin 1 = nPositions, bin 2 = nEnergies, bin 3 = first energy
  for (int ireg = 0; ireg < nLeakReg; ireg++) {
    std::cout << "energies ireg " << ireg << ": ";
    for (int ie = 0; ie < nEnergies; ie++) {
      bin++;
      generatedE[ireg][ie] = inputParameters->GetBinContent(bin);
      std::cout << generatedE[ireg][ie] << " ";
    }
    std::cout << std::endl;
  }

  //..Energy per thetaID (in MeV, for use in titles etc)
  int iEnergiesMeV[nEnergies][nThetaID];
  for (int thID = 0; thID < nThetaID; thID++) {
    int ireg = 0;
    if (thID >= firstBarrelID and thID <= lastBarrelID) {
      ireg = 1;
    } else if (thID > lastBarrelID) {
      ireg = 2;
    }
    for (int ie = 0; ie < nEnergies; ie++) {
      iEnergiesMeV[ie][thID] = (int)(1000.*generatedE[ireg][ie] + 0.5);
    }
  }

  //-----------------------------------------------------------------------------------
  //..Bins for eFrac histograms (eFrac = uncorrected reconstructed E / E true)
  const double eFracLo = 0.4; // low edge of eFrac histograms
  const double eFracHi = 1.5; // high edge of eFrac histograms
  int nEfracBins[nEnergies][nThetaID];
  for (int thID = 0; thID < nThetaID; thID++) {
    std::cout << "nBins for thetaID = " << thID << ": ";
    for (int ie = 0; ie < nEnergies; ie++) {

      //..ballpark resolution
      double res_squared = 0.0001 + 0.064 / iEnergiesMeV[ie][thID];

      //..Convert this to an even integer to get number of bins
      double binNumOver2 = 3. / sqrt(res_squared);
      int tempNBin = (int)(binNumOver2 + 0.5);
      nEfracBins[ie][thID] = 2 * tempNBin;
      std::cout << nEfracBins[ie][thID] << " ";
    }
    std::cout << std::endl;
  }

  //-----------------------------------------------------------------------------------
  //..Set up Novosibirsk fit function
  TString statusString[7] = {"good", "refit", "lowStat", "lowProb", "peakAtLimit", "sigmaAtLimit", "etaAtLimit"}; /**< categories of fit status */
  const double fracEnt[2] = {0.683, 0.5}; // fit range includes 68% or 50% of entries
  const double minEntries = 100.; // don't use fits with fewer entries
  const double minMaxBin = 50.; // rebin if max bin is below this
  const double highMaxBin = 300.; // can float eta is max bin is above this

  TF1* func = new TF1("eclLeakageNovo", eclLeakageNovo, eFracLo, eFracHi, 4);
  func->SetParNames("normalization", "peak", "effSigma", "eta");
  func->SetLineColor(kRed);

  //-----------------------------------------------------------------------------------
  //..Specify the TTree
  auto tree = getObjectPtr<TTree>("tree");
  tree->SetBranchAddress("cellID", &t_cellID);
  tree->SetBranchAddress("thetaID", &t_thetaID);
  tree->SetBranchAddress("region", &t_region);
  tree->SetBranchAddress("thetaBin", &t_thetaBin);
  tree->SetBranchAddress("phiBin", &t_phiBin);
  tree->SetBranchAddress("phiMech", &t_phiMech);
  tree->SetBranchAddress("energyBin", &t_energyBin);
  tree->SetBranchAddress("nCrys", &t_nCrys);
  tree->SetBranchAddress("energyFrac", &t_energyFrac);
  tree->SetBranchAddress("origEnergyFrac", &t_origEnergyFrac);
  tree->SetBranchAddress("locationError", &t_locationError);

  const int treeEntries = tree->GetEntries();
  B2INFO("eclLeakageAlgorithm entries = " << treeEntries);

  //====================================================================================
  //====================================================================================
  //..Step 2. First loop. Derive location cut for each energy and thetaID

  //-----------------------------------------------------------------------------------
  //..Histograms of location error for each energy and thetaID
  TH1F* locError[nEnergies][nThetaID];
  TString name;
  TString title;
  for (int thID = 0; thID < nThetaID; thID++) {
    for (int ie = 0; ie < nEnergies; ie++) {
      name = "locError_" + std::to_string(ie) + "_" +  std::to_string(thID);
      title = "Location error " + to_string(iEnergiesMeV[ie][thID]) + " MeV thetaID " + std::to_string(thID) + ";location error (cm)";
      locError[ie][thID] = new TH1F(name, title, 300, 0, 30.);
    }
  }

  //-----------------------------------------------------------------------------------
  //..Loop over tree and histogram the location errors
  for (int i = 0; i < treeEntries; i++) {
    tree->GetEntry(i);
    if (t_thetaID >= firstUsefulThID and t_thetaID <= lastUsefulThID and t_energyBin >= 0) {
      locError[t_energyBin][t_thetaID]->Fill(t_locationError);
    }
  }

  //-----------------------------------------------------------------------------------
  //..Cut is location where the distribution drops below 2.5% of peak after pedestal subtraction
  const double minLocEntries = 49.5; // Write to disk if sufficient entries
  const double peakFrac = 0.025; // look for distribution to drop to this fraction of peak value
  const double startOfPed = 10.0001; // cm

  float maxLocCut[nEnergies][nThetaID] = {}; // location cut for each energy and thetaID

  //..Summary histograms of cut values and fraction of events passing cut
  TH1F* locCutSummary = new TH1F("locCutSummary", "location cut for each thetaID/energy; xBin = thetaID + ie*nTheta", nbinX, 0,
                                 nbinX);
  TH1F* locCutEfficiency = new TH1F("locCutEfficiency",
                                    "Fraction of events passing location cut for each thetaID/energy; xBin = thetaID + ie*nTheta", nbinX, 0, nbinX);

  for (int thID = firstUsefulThID; thID <= lastUsefulThID; thID++) {
    for (int ie = 0; ie < nEnergies; ie++) {
      if (locError[ie][thID]->Integral() < minLocEntries) {continue;}

      //..Pedestal per bin is average from 10 cm onwards
      const int i10cm = locError[ie][thID]->GetXaxis()->FindBin(startOfPed);
      const int ifinal = locError[ie][thID]->GetNbinsX();
      double pedestal = locError[ie][thID]->Integral(i10cm, ifinal) / (ifinal + 1 - i10cm);
      double threshold = pedestal + peakFrac * (locError[ie][thID]->GetMaximum() - pedestal);
      const int ipeak = locError[ie][thID]->GetMaximumBin();

      //..Lower edge of 2nd consecutive bin below threshold
      const int ix = 1 + thID + nThetaID * ie; // summary hist bin; +1 because first hist bin is 1, not 0
      for (int ib = ipeak; ib <= ifinal; ib++) {
        const double ibEnt = locError[ie][thID]->GetBinContent(ib);
        const double ibMinus1 = locError[ie][thID]->GetBinContent(ib - 1);

        //..This is the cut location. Record in array and in summary histogram
        if (ibEnt < threshold and ibMinus1 < threshold) {
          maxLocCut[ie][thID] = locError[ie][thID]->GetBinLowEdge(ib);
          locCutSummary->SetBinContent(ix, maxLocCut[ie][thID]);
          locCutSummary->SetBinError(ix, 0);

          //..Efficiency, sort of
          const double entries = locError[ie][thID]->GetEntries();
          const double pass = locError[ie][thID]->Integral(1, ib - 1); // bin ib is not included
          locCutEfficiency->SetBinContent(ix, pass / entries);
          locCutEfficiency->SetBinError(ix, 0);

          break;
        }
      }
      std::cout << "  thID " << thID << " E " << iEnergiesMeV[ie][thID] << " ped " << pedestal << " threshold " << threshold << " cut " <<
                maxLocCut[ie][thID] << std::endl;

      //..Write this one to disk, after supplementing the title with cut and efficiency
      title = locError[ie][thID]->GetTitle();
      TString sCut;
      const double locEff = locCutEfficiency->GetBinContent(ix);
      sCut.Form(" cut %0.1f cm, eff %0.2f", maxLocCut[ie][thID], locEff);
      title += sCut;
      locError[ie][thID]->SetTitle(title);
      histfile->cd();
      locError[ie][thID]->Write();
    }
  }

  //..Write out the summary histogram
  histfile->cd();
  locCutSummary->Write();
  locCutEfficiency->Write();

  //====================================================================================
  //====================================================================================
  //..Step 3. Second loop, fill histograms of e/eTrue for each energy and thetaID

  //-----------------------------------------------------------------------------------
  //..One histogram of e/eTrue per energy per thetaID.
  //  Used to fix eta in subsequent fits, and to get overall correction for that thetaID
  TH1F* hELabUncorr[nEnergies][nThetaID];
  for (int thID = firstUsefulThID; thID <= lastUsefulThID; thID++) {
    TString sthID = std::to_string(thID);
    for (int ie = 0; ie < nEnergies; ie++) {
      name = "hELabUncorr_" + std::to_string(ie) + "_" + sthID;
      title = "eFrac " + to_string(iEnergiesMeV[ie][thID]) + " MeV thetaID " + sthID + ";E/Etrue";

      //..High statistics for these plots; use more bins
      hELabUncorr[ie][thID] = new TH1F(name, title, 2 * nEfracBins[ie][thID], eFracLo, eFracHi);
    }
  }

  //-----------------------------------------------------------------------------------
  //..Loop over events and store eFrac
  for (int i = 0; i < treeEntries; i++) {
    tree->GetEntry(i);

    //..Only events with good reconstruction
    bool goodReco = t_thetaID >= firstUsefulThID and t_thetaID <= lastUsefulThID and t_energyBin >= 0
                    and t_locationError < maxLocCut[t_energyBin][t_thetaID];
    if (not goodReco) {continue;}

    //..Fill histogram for full thetaID
    hELabUncorr[t_energyBin][t_thetaID]->Fill(t_energyFrac);
  }

  //-----------------------------------------------------------------------------------
  //..Fit each thetaID/energy histogram to get peak (overall correction) and eta (fixed
  //  in subsequent fits to individual locations).
  float peakUncorr[nEnergies][nThetaID]; // store peak from each fit
  float etaUncorr[nEnergies][nThetaID]; // store eta from each fit
  std::vector<TString> failedELabUncorr; // names of hists with failed fits
  std::vector<int> statusELabUncorr; // status of failed fits
  int payloadStatus = 0; // Overall status of payload determination

  //..Fit each histogram
  for (int thID = firstUsefulThID; thID <= lastUsefulThID; thID++) {
    TString sthID = std::to_string(thID);
    for (int ie = 0; ie < nEnergies; ie++) {
      TH1F* hEnergy = (TH1F*)hELabUncorr[ie][thID];
      double peak = -1.;
      double eta = 0.;
      int fitStatus = 2;
      double entries = hEnergy->Integral();
      int nIter = 0; // keep track of attempts to fit this histogram
      bool fitHist = entries > minEntries;

      //..Possibly iterate fit starting from this point
      while (fitHist) {

        //..Fit parameters
        double norm = hEnergy->GetMaximum();
        double target = fracEnt[nIter] * entries; // fit range contains 68% or 50%
        std::vector<double> startingParameters;// peak, sigma, eta, fitLow, fitHigh, nbins
        startingParameters = eclLeakageFitParameters(hEnergy, target);
        func->SetParameters(norm, startingParameters[0], startingParameters[1], startingParameters[2]);
        func->SetParLimits(1, startingParameters[3], startingParameters[4]);
        func->SetParLimits(2, 0., startingParameters[4] - startingParameters[3]);
        func->SetParLimits(3, etaMin, etaMax);

        //..Fit
        name = hEnergy->GetName();
        std::cout << "Fitting " << name.Data() << std::endl;
        hEnergy->Fit(func, "LIEQ", "", startingParameters[3], startingParameters[4]);
        peak = func->GetParameter(1);
        double effSigma = func->GetParameter(2);
        eta = func->GetParameter(3);
        double prob = func->GetProb();

        //..Check fit quality  0 = good, 1 = redo fit, 2 = lowStat, 3 = lowProb,
        //  4 = peakAtLimit, 5 = sigmaAtLimit, 6 = etaAtLimit
        double dEta = min((etaMax - eta), (eta - etaMin));
        fitStatus = eclLeakageFitQuality(startingParameters[3], startingParameters[4], peak, effSigma, dEta, prob);

        //..If the fit probability is low, refit using a smaller range (fracEnt)
        if ((fitStatus == 1 or fitStatus == 3) and nIter == 0) {
          nIter++;
        } else {
          fitHist = false;
        }
      }

      //-----------------------------------------------------------------------------------
      //..Record failures and fit results.
      //  Mark payload as failed if low stats or a fit at limit.
      if (fitStatus >= 2) {
        statusELabUncorr.push_back(fitStatus);
        failedELabUncorr.push_back(hEnergy->GetName());
        if (fitStatus == 2 or fitStatus == 4) {payloadStatus = 1;} // failed
      }
      peakUncorr[ie][thID] = peak;
      etaUncorr[ie][thID] = eta;

      //..Write to disk
      if (entries > minEntries) {
        histfile->cd();
        hELabUncorr[ie][thID]->Write();
      }
    }
  }

  //-----------------------------------------------------------------------------------
  //..Quit now if one of these fits failed, since we will not get a payload.
  if (payloadStatus != 0) {
    B2ERROR("ecLeakageAlgorithm: fit to hELabUncorr failed. ");
    const int nbad = statusELabUncorr.size();
    for (int ibad = 0; ibad < nbad; ibad++) {
      int badStat = statusELabUncorr[ibad];
      B2ERROR(" histogram " << failedELabUncorr[ibad].Data() << " status " << badStat << " " << statusString[badStat].Data());
    }
    return c_Failure;
  }

  //====================================================================================
  //====================================================================================
  //..Step 4. Third loop, fill histograms of e/eTrue as a function of position.
  //  29 locations in theta, 29 in phi.
  //  Crystals next to mechanical structure in phi are treated separately from
  //  crystals without mechanical structure.

  //-----------------------------------------------------------------------------------
  //..Histograms to store the energy
  const int nDir = 3;
  TString dirName[nDir] = {"theta", "phiMech", "phiNoMech"};

  TH1F* eFracPosition[nEnergies][nThetaID][nDir][nPositions]; // the histograms
  std::vector<TString> failedeFracPosition; // names of hists with failed fits
  std::vector<int> statuseFracPosition; // status of failed fits

  for (int thID = firstUsefulThID; thID <= lastUsefulThID; thID++) {
    TString sthID = std::to_string(thID);
    for (int ie = 0; ie < nEnergies; ie++) {
      TString sie = std::to_string(ie);
      for (int idir = 0; idir < nDir; idir++) {
        TString sidir = std::to_string(idir);
        for (int ipos = 0; ipos < nPositions; ipos++) {
          TString sipos = std::to_string(ipos);
          name = "eFracPosition_" + sie + "_" + sthID + "_" + sidir + "_" + sipos;
          title = "eFrac " + to_string(iEnergiesMeV[ie][thID]) + " MeV thetaID " + sthID + " " + dirName[idir] + " pos " + sipos +
                  "; E/Etrue";
          eFracPosition[ie][thID][idir][ipos] = new TH1F(name, title, nEfracBins[ie][thID], eFracLo, eFracHi);
        }
      }
    }
  }

  //..And some summary histograms
  TH1F* statusOfeFracPosition = new TH1F("statusOfeFracPosition",
                                         "status of eFrac fits for each position. 0 = good, 1 = redo fit, 2 = lowStat, 3 = lowProb, 4 = peakAtLimit, 5 = sigmaAtLimit", 6, 0,
                                         6);
  TH1F* probOfeFracPosition = new TH1F("probOfeFracPosition", "fit probability of eFrac fits for each position;probability", 100, 0,
                                       1);
  TH1F* maxOfeFracPosition = new TH1F("maxOfeFracPosition", "max entries of eFrac histograms;maximum bin content", 100, 0, 1000);

  //-----------------------------------------------------------------------------------
  //..Loop over events and store eFrac
  for (int i = 0; i < treeEntries; i++) {
    tree->GetEntry(i);

    //..Only events with good reconstruction
    bool goodReco = t_thetaID >= firstUsefulThID and t_thetaID <= lastUsefulThID and t_energyBin >= 0
                    and t_locationError < maxLocCut[t_energyBin][t_thetaID];
    if (not goodReco) {continue;}

    //..Theta location (idir = 0)
    int idir = 0;
    eFracPosition[t_energyBin][t_thetaID][idir][t_thetaBin]->Fill(t_energyFrac);

    //..phi location. Note that t_phiBin starts at mechanical edge, if there is one.
    idir = t_phiMech + 1;
    eFracPosition[t_energyBin][t_thetaID][idir][t_phiBin]->Fill(t_energyFrac);
  }

  //-----------------------------------------------------------------------------------
  //..Now fit many many histograms to get the position-dependent corrections
  float positionCorrection[nEnergies][nThetaID][nDir][nPositions];
  float positionCorrectionUnc[nEnergies][nThetaID][nDir][nPositions];
  int nHistToFit = 0;


  //..Temp histogram of position corrections
  TH1F* thetaCorSummary = new TH1F("thetaCorSummary", "Theta dependent corrections;theta dependent correction", 100, 0.4, 1.4);
  TH1F* phiCorSummary = new TH1F("phiCorSummary", "Phi dependent corrections;phi dependent correction", 100, 0.4, 1.4);

  for (int thID = firstUsefulThID; thID <= lastUsefulThID; thID++) {
    for (int ie = 0; ie < nEnergies; ie++) {
      for (int idir = 0; idir < nDir; idir++) {
        for (int ipos = 0; ipos < nPositions; ipos++) {
          TH1F* hEnergy = (TH1F*)eFracPosition[ie][thID][idir][ipos];
          if (hEnergy->Integral() > minEntries) {nHistToFit++;}

          //..Default peak from fit to full thetaID/energy = peakUncorr;
          //  correction = peak / sqrt(peakUncorr) = sqrt(peakUncorr)
          double correction = sqrt(peakUncorr[ie][thID]);
          double corrUnc = 0.05; // arbitrary uncertainty
          double prob = -1.;
          int fitStatus = 2; // low stats
          if (hEnergy->GetEntries() < 0.5) {fitStatus = -2;} // unused, eg barrel pos=2
          int nIter = 0; // keep track of attempts to fit this histogram
          double entries = hEnergy->Integral();
          bool fitHist = entries > minEntries;

          //..Possibly iterate fit starting from this point
          while (fitHist) {
            fitHist = false;

            //..Fit parameters
            double target = fracEnt[nIter] * entries; // fit range contains 68%
            std::vector<double> startingParameters;// peak, sigma, eta, fitLow, fitHigh, nbins
            bool findParm = true;
            while (findParm) {
              startingParameters = eclLeakageFitParameters(hEnergy, target);

              //..Rebin if stats are low and we have enough DOF
              if (hEnergy->GetMaximum()<minMaxBin and startingParameters[5]>11.9) {
                hEnergy->Rebin(2);
              } else {
                findParm = false;
              }
            }
            double norm = hEnergy->GetMaximum(); // max bin after rebinning
            double fitLow = startingParameters[3];
            double fitHigh = startingParameters[4];

            //..Eta from the fit to the full crystal
            double etaFix = etaUncorr[ie][thID];
            func->SetParameters(norm, startingParameters[0], startingParameters[1], etaFix);
            func->SetParLimits(1, fitLow, fitHigh);
            func->SetParLimits(2, 0., fitHigh - fitLow);

            //..Fix eta, unless really good statistics
            if (hEnergy->GetMaximum() < highMaxBin) {
              func->SetParLimits(3, etaFix, etaFix);
            } else {
              func->SetParLimits(3, etaMin, etaMax);
            }

            //..Fit
            name = hEnergy->GetName();
            std::cout << "Fitting " << name.Data() << std::endl;
            hEnergy->Fit(func, "LIEQ", "", fitLow, fitHigh);
            double peak = func->GetParameter(1);
            double effSigma = func->GetParameter(2);
            double eta = func->GetParameter(3);
            prob = func->GetProb();

            //..Check fit quality  0 = good, 1 = redo fit, 2 = lowStat, 3 = lowProb,
            //  4 = peakAtLimit, 5 = sigmaAtLimit, 6 = etaAtLimit.
            double dEta = min((etaMax - eta), (eta - etaMin));
            fitStatus = eclLeakageFitQuality(fitLow, fitHigh, peak, effSigma, dEta, prob);

            //..If the fit probability is low, refit using a smaller range (fracEnt)
            if ((fitStatus == 1 or fitStatus == 3) and nIter == 0) {
              nIter++;
              fitHist = true;

              //  Store correction except for peak or sigma at limit.
            } else if (fitStatus <= 3) {

              //..Divide by sqrt(peak for full crystal) to get correct average
              //  when multiplying the theta and phi corrections
              correction = peak / sqrt(peakUncorr[ie][thID]);
              corrUnc = func->GetParError(1) / sqrt(peakUncorr[ie][thID]);
            }
          }

          //..Store the correction for this position
          positionCorrection[ie][thID][idir][ipos] = correction;
          positionCorrectionUnc[ie][thID][idir][ipos] = corrUnc;
          statusOfeFracPosition->Fill(fitStatus + 0.00001);
          probOfeFracPosition->Fill(prob);
          maxOfeFracPosition->Fill(hEnergy->GetMaximum());

          //..Summary histogram for successful fits
          if (prob > 0 and fitStatus <= 3) {
            if (idir == 0) {
              thetaCorSummary->Fill(correction);
            } else {
              phiCorSummary->Fill(correction);
            }
          }

          //..Record the failed fit details. We may still use the correction.
          if (fitStatus >= 2) {
            statuseFracPosition.push_back(fitStatus);
            failedeFracPosition.push_back(hEnergy->GetName());
            histfile->cd();
            hEnergy->Write();
          }
        }
      }
    }
  }

  //-----------------------------------------------------------------------------------
  //..Summarize position fit results
  B2INFO("eclLeakageAlgorithm: " << nHistToFit << " position histograms to fit");
  int nbadFit = statuseFracPosition.size();
  B2INFO("Failed fits: " << nbadFit);
  for (int ibad = 0; ibad < nbadFit; ibad++) {
    int badStat = statuseFracPosition[ibad];
    B2ERROR(" histogram " << failedeFracPosition[ibad].Data() << " status " << badStat << " " << statusString[badStat].Data());
  }

  //..Write to disk
  histfile->cd();
  statusOfeFracPosition->Write();
  probOfeFracPosition->Write();
  maxOfeFracPosition->Write();
  thetaCorSummary->Write();
  phiCorSummary->Write();

  //====================================================================================
  //====================================================================================
  //..Step 5. Fourth loop, fill and fit histograms of ePos for each thetaID/energy,
  //  where ePos = normalized energy after position correction.

  //-----------------------------------------------------------------------------------
  //..One histogram of ePos per energy per thetaID.
  TH1F* hEPos[nEnergies][nThetaID];
  for (int thID = firstUsefulThID; thID <= lastUsefulThID; thID++) {
    TString sthID = std::to_string(thID);
    for (int ie = 0; ie < nEnergies; ie++) {
      name = "hEPos_" + std::to_string(ie) + "_" + sthID;
      title = "ePos " + to_string(iEnergiesMeV[ie][thID]) + " MeV thetaID " + sthID + ";Position corrected E/Etrue";

      //..High statistics for these plots; use more bins
      hEPos[ie][thID] = new TH1F(name, title, 2 * nEfracBins[ie][thID], eFracLo, eFracHi);
    }
  }

  //-----------------------------------------------------------------------------------
  //..Loop over events and store ePos
  for (int i = 0; i < treeEntries; i++) {
    tree->GetEntry(i);

    //..Only events with good reconstruction
    bool goodReco = t_thetaID >= firstUsefulThID and t_thetaID <= lastUsefulThID and t_energyBin >= 0
                    and t_locationError < maxLocCut[t_energyBin][t_thetaID];
    if (not goodReco) {continue;}

    //..Position-dependent leakage corrections
    int idir = 0;
    float thetaPosCor = positionCorrection[t_energyBin][t_thetaID][idir][t_thetaBin];

    idir = t_phiMech + 1;
    float phiPosCor = positionCorrection[t_energyBin][t_thetaID][idir][t_phiBin];

    float ePos = t_energyFrac / thetaPosCor / phiPosCor;

    //..Fill histogram for full thetaID
    hEPos[t_energyBin][t_thetaID]->Fill(ePos);
  }

  //-----------------------------------------------------------------------------------
  //..Fit each thetaID/energy histogram to get eta (fixed for later nCrys fits)
  float etaEpos[nEnergies][nThetaID]; // store eta from each fit
  std::vector<TString> failedEPos; // names of hists with failed fits
  std::vector<int> statusEPos; // status of failed fits

  for (int thID = firstUsefulThID; thID <= lastUsefulThID; thID++) {
    TString sthID = std::to_string(thID);
    for (int ie = 0; ie < nEnergies; ie++) {
      TH1F* hEnergy = (TH1F*)hEPos[ie][thID];
      double eta = 0.;
      int fitStatus = 2;
      double entries = hEnergy->Integral();
      int nIter = 0; // keep track of attempts to fit this histogram
      bool fitHist = entries > minEntries;

      //..Possibly iterate fit starting from this point
      while (fitHist) {

        //..Fit parameters
        double norm = hEnergy->GetMaximum();
        double target = fracEnt[nIter] * entries; // fit range contains 68% or 50%
        std::vector<double> startingParameters;// peak, sigma, eta, fitLow, fitHigh, nbins
        startingParameters = eclLeakageFitParameters(hEnergy, target);
        func->SetParameters(norm, startingParameters[0], startingParameters[1], startingParameters[2]);
        func->SetParLimits(1, startingParameters[3], startingParameters[4]);
        func->SetParLimits(2, 0., startingParameters[4] - startingParameters[3]);
        func->SetParLimits(3, etaMin, etaMax);

        //..Fit
        name = hEnergy->GetName();
        std::cout << "Fitting " << name.Data() << std::endl;
        hEnergy->Fit(func, "LIEQ", "", startingParameters[3], startingParameters[4]);
        double peak = func->GetParameter(1);
        double effSigma = func->GetParameter(2);
        eta = func->GetParameter(3);
        double prob = func->GetProb();

        //..Check fit quality  0 = good, 1 = redo fit, 2 = lowStat, 3 = lowProb,
        //  4 = peakAtLimit, 5 = sigmaAtLimit, 6 = etaAtLimit
        double dEta = min((etaMax - eta), (eta - etaMin));
        fitStatus = eclLeakageFitQuality(startingParameters[3], startingParameters[4], peak, effSigma, dEta, prob);

        //..If the fit probability is low, refit using a smaller range (fracEnt)
        if ((fitStatus == 1 or fitStatus == 3) and nIter == 0) {
          nIter++;
        } else {
          fitHist = false;
        }
      }

      //-----------------------------------------------------------------------------------
      //..Record failures and fit results.
      //  Mark payload as failed if low stats or a fit at limit.
      if (fitStatus >= 2) {
        statusEPos.push_back(fitStatus);
        failedEPos.push_back(hEnergy->GetName());
        if (fitStatus == 2 or fitStatus == 4) {payloadStatus = 1;} // failed
      }
      etaEpos[ie][thID] = eta;

      //..Write to disk
      if (entries > minEntries) {
        histfile->cd();
        hEPos[ie][thID]->Write();
      }
    }
  }

  //====================================================================================
  //====================================================================================
  //..Step 6. Fifth loop, fill and fit histograms of ePos for each nCrys, for each
  //  thetaID/energy, where ePos = normalized energy after position correction.
  const int maxN = 21; // maximum number of crystals in a cluster

  //-----------------------------------------------------------------------------------
  //..Histograms of ePos
  TH1F* ePosnCry[nEnergies][nThetaID][maxN + 1]; // +1 to include 0
  for (int thID = firstUsefulThID; thID <= lastUsefulThID; thID++) {
    TString sthID = std::to_string(thID);
    for (int ie = 0; ie < nEnergies; ie++) {
      TString sie = std::to_string(ie);
      for (int in = 0; in <= maxN; in++) {
        name = "ePosnCry_" + sie + "_" + sthID + "_" + std::to_string(in);
        title = "ePos " + to_string(iEnergiesMeV[ie][thID]) + " MeV thetaID " + sthID + " nCrys " + std::to_string(
                  in) + "; corrected E/Etrue";
        ePosnCry[ie][thID][in] = new TH1F(name, title, nEfracBins[ie][thID], eFracLo, eFracHi);
      }
    }
  }

  //-----------------------------------------------------------------------------------
  //..Loop over events and store ePos
  for (int i = 0; i < treeEntries; i++) {
    tree->GetEntry(i);

    //..Only events with good reconstruction
    bool goodReco = t_thetaID >= firstUsefulThID and t_thetaID <= lastUsefulThID and t_energyBin >= 0
                    and t_locationError < maxLocCut[t_energyBin][t_thetaID];
    if (not goodReco) {continue;}

    //..Position-dependent leakage corrections
    int idir = 0;
    float thetaPosCor = positionCorrection[t_energyBin][t_thetaID][idir][t_thetaBin];

    idir = t_phiMech + 1;
    float phiPosCor = positionCorrection[t_energyBin][t_thetaID][idir][t_phiBin];

    float ePos = t_energyFrac / thetaPosCor / phiPosCor;
    ePosnCry[t_energyBin][t_thetaID][t_nCrys]->Fill(ePos);
  }

  //-----------------------------------------------------------------------------------
  //..Fit ePos histograms for each value of nCrys. Many of these will not have enough stats
  float nCrysCorrection[nEnergies][nThetaID][maxN + 1];
  float nCrysCorrectionUnc[nEnergies][nThetaID][maxN + 1];

  //..Keep track of the peak nCrys for each thetaID/energy. Use this later to fix up
  //  values of nCrys without a successful fit
  int maxNCry[nEnergies][nThetaID] = {};

  for (int thID = firstUsefulThID; thID <= lastUsefulThID; thID++) {
    for (int ie = 0; ie < nEnergies; ie++) {
      double maxIntegral = 0;
      for (int in = 0; in <= maxN; in++) {

        TH1F* hEnergy = (TH1F*)ePosnCry[ie][thID][in];
        if (hEnergy->Integral() > minEntries) {nHistToFit++;}

        //..Defaults for failed fits. Will fix up in the next step.
        double correction = -1.;
        double corrUnc = 0.05; // arbitrary uncertainty
        int nIter = 0; // keep track of attempts to fit this histogram
        double entries = hEnergy->Integral();
        bool fitHist = entries > minEntries;

        //..nCrys with most entries
        if (entries > maxIntegral) {
          maxIntegral = entries;
          maxNCry[ie][thID] = in;
        }

        //..Possibly iterate fit starting from this point
        while (fitHist) {
          fitHist = false;

          //..Fit parameters
          double target = fracEnt[nIter] * entries; // fit range contains 68%
          std::vector<double> startingParameters;// peak, sigma, eta, fitLow, fitHigh, nbins
          bool findParm = true;
          while (findParm) {
            startingParameters = eclLeakageFitParameters(hEnergy, target);

            //..Rebin if stats are low and we have enough DOF
            if (hEnergy->GetMaximum()<minMaxBin and startingParameters[5]>11.9) {
              hEnergy->Rebin(2);
            } else {
              findParm = false;
            }
          }
          double norm = hEnergy->GetMaximum(); // max bin after rebinning
          double fitLow = startingParameters[3];
          double fitHigh = startingParameters[4];

          //..Eta from the fit to the full crystal after position-dependent leakage correction
          double etaFix = etaEpos[ie][thID];
          func->SetParameters(norm, startingParameters[0], startingParameters[1], etaFix);
          func->SetParLimits(1, fitLow, fitHigh);
          func->SetParLimits(2, 0., fitHigh - fitLow);

          //..Fix eta, unless really good statistics
          if (hEnergy->GetMaximum() < highMaxBin) {
            func->SetParLimits(3, etaFix, etaFix);
          } else {
            func->SetParLimits(3, etaMin, etaMax);
          }

          //..Fit
          name = hEnergy->GetName();
          std::cout << "Fitting " << name.Data() << std::endl;
          hEnergy->Fit(func, "LIEQ", "", fitLow, fitHigh);
          double peak = func->GetParameter(1);
          double effSigma = func->GetParameter(2);
          double eta = func->GetParameter(3);
          double prob = func->GetProb();

          //..Check fit quality  0 = good, 1 = redo fit, 2 = lowStat, 3 = lowProb,
          //  4 = peakAtLimit, 5 = sigmaAtLimit, 6 = etaAtLimit.
          double dEta = min((etaMax - eta), (eta - etaMin));
          int fitStatus = eclLeakageFitQuality(fitLow, fitHigh, peak, effSigma, dEta, prob);

          //..If the fit probability is low, refit using a smaller range (fracEnt)
          if ((fitStatus == 1 or fitStatus == 3) and nIter == 0) {
            nIter++;
            fitHist = true;

            //  Store correction except for peak or sigma at limit.
          } else if (fitStatus <= 3) {
            correction = peak;
            corrUnc = func->GetParError(1);
          }
        }

        //..Store the correction for this position
        nCrysCorrection[ie][thID][in] = correction;
        nCrysCorrectionUnc[ie][thID][in] = corrUnc;
      }
    }
  }

  //-----------------------------------------------------------------------------------
  //..Now fix up corrections for nCrys values that did not have a successful fit.
  //  The logic is that if "n" is not successful, then use the correction from either
  //  n+1 or n-1, which ever is closer to the most likely value of nCrys
  for (int thID = firstUsefulThID; thID <= lastUsefulThID; thID++) {
    for (int ie = 0; ie < nEnergies; ie++) {

      //..If the most likely value of nCrys does not have a successful correction
      //  mark the algorithm as failed
      int highestN = maxNCry[ie][thID];
      if (nCrysCorrection[ie][thID][highestN] <= 0) {
        B2ERROR("eclLeakageAlgorithm: no nCrys correction for ie " << ie << " thetaID " << thID);
        return c_Failure;
      }

      //..Start at the highest and work upwards
      for (int in = highestN + 1; in <= maxN; in++) {
        if (nCrysCorrection[ie][thID][in] < 0) {
          nCrysCorrection[ie][thID][in] = nCrysCorrection[ie][thID][in - 1];
          nCrysCorrectionUnc[ie][thID][in] = nCrysCorrectionUnc[ie][thID][in - 1];
        }
      }

      //..Start at the highest and move downwards
      for (int in = highestN - 1; in >= 0; in--) {
        if (nCrysCorrection[ie][thID][in] < 0) {
          nCrysCorrection[ie][thID][in] = nCrysCorrection[ie][thID][in + 1];
          nCrysCorrectionUnc[ie][thID][in] = nCrysCorrectionUnc[ie][thID][in + 1];
        }
      }
    }
  }

  //====================================================================================
  //====================================================================================
  //..Step 7. Store quantities needed for the payloads

  //-----------------------------------------------------------------------------------
  //..First, we need to fix up the thetaID that have been so far missed.
  //  Just copy the values from the first or last thetaID for which corrections were found.

  //..ThetaID before the first useful one
  for (int thID = firstUsefulThID - 1; thID >= 0; thID--) {
    for (int ie = 0; ie < nEnergies; ie++) {

      //..Position dependent
      for (int idir = 0; idir < 3; idir++) {
        for (int ipos = 0; ipos < nPositions; ipos++) {
          positionCorrection[ie][thID][idir][ipos] = positionCorrection[ie][thID + 1][idir][ipos];
          positionCorrectionUnc[ie][thID][idir][ipos] = positionCorrectionUnc[ie][thID + 1][idir][ipos];
        }
      }

      //..nCrys
      for (int in = 0; in <= maxN; in++) {
        nCrysCorrection[ie][thID][in] = nCrysCorrection[ie][thID + 1][in];
        nCrysCorrectionUnc[ie][thID][in] = nCrysCorrectionUnc[ie][thID + 1][in];
      }
    }
  }

  //..ThetaID beyond last useful one
  for (int thID = lastUsefulThID + 1; thID < nThetaID; thID++) {
    for (int ie = 0; ie < nEnergies; ie++) {

      //..Position dependent
      for (int idir = 0; idir < 3; idir++) {
        for (int ipos = 0; ipos < nPositions; ipos++) {
          positionCorrection[ie][thID][idir][ipos] = positionCorrection[ie][thID - 1][idir][ipos];
          positionCorrectionUnc[ie][thID][idir][ipos] = positionCorrectionUnc[ie][thID - 1][idir][ipos];
        }
      }

      //..nCrys
      for (int in = 0; in <= maxN; in++) {
        nCrysCorrection[ie][thID][in] = nCrysCorrection[ie][thID - 1][in];
        nCrysCorrectionUnc[ie][thID][in] = nCrysCorrectionUnc[ie][thID - 1][in];
      }
    }
  }

  //-----------------------------------------------------------------------------------
  //..std::vectors of generated energies
  std::vector<float> forwardVector;
  std::vector<float> barrelVector;
  std::vector<float> backwardVector;
  for (int ie = 0; ie < nEnergies; ie++) {
    forwardVector.push_back(log(generatedE[0][ie]));
    barrelVector.push_back(log(generatedE[1][ie]));
    backwardVector.push_back(log(generatedE[2][ie]));
  }

  //..Store in array format for validation studies
  float leakLogE[3][12] = {}; /**< log(E) values for each region */
  for (int ie = 0; ie < nEnergies; ie++) {
    leakLogE[0][ie] = forwardVector[ie];
    leakLogE[1][ie] = barrelVector[ie];
    leakLogE[2][ie] = backwardVector[ie];
  }

  //-----------------------------------------------------------------------------------
  //..Position dependent corrections

  //..2D histogram of theta-dependent corrections
  TH2F thetaCorrection("thetaCorrection", "Theta correction vs bin;bin = thetaID + 69*energyBin;local theta bin", nbinX, 0, nbinX,
                       nPositions, 0, nPositions);
  int ix = 0;
  for (int ie = 0; ie < nEnergies; ie++) {
    for (int thID = 0; thID < nThetaID; thID++) {
      ix++;
      for (int ipos = 0; ipos < nPositions; ipos++) {
        int iy = ipos + 1;
        float correction = positionCorrection[ie][thID][0][ipos];
        float corrUnc = positionCorrectionUnc[ie][thID][0][ipos];
        thetaCorrection.SetBinContent(ix, iy, correction);
        thetaCorrection.SetBinError(ix, iy, corrUnc);
      }
    }
  }

  //..2D histogram of phi-dependent corrections. Twice as many x bins;
  //  one set for crystals next to mechanical structure, one for otherwise.
  TH2F phiCorrection("phiCorrection", "Phi correction vs bin;bin = thetaID + 69*energyBin;local phi bin", 2 * nbinX, 0, 2 * nbinX,
                     nPositions, 0, nPositions);
  ix = 0;
  for (int ie = 0; ie < nEnergies; ie++) {
    for (int thID = 0; thID < nThetaID; thID++) {
      ix++;
      for (int ipos = 0; ipos < nPositions; ipos++) {
        int iy = ipos + 1;

        //..First set of corrections
        float correction = positionCorrection[ie][thID][1][ipos];
        float corrUnc = positionCorrectionUnc[ie][thID][1][ipos];
        phiCorrection.SetBinContent(ix, iy, correction);
        phiCorrection.SetBinError(ix, iy, corrUnc);

        //..Second set
        correction = positionCorrection[ie][thID][2][ipos];
        corrUnc = positionCorrectionUnc[ie][thID][2][ipos];
        phiCorrection.SetBinContent(ix + nbinX, iy, correction);
        phiCorrection.SetBinError(ix + nbinX, iy, corrUnc);
      }
    }
  }

  //-----------------------------------------------------------------------------------
  //..nCrys dependent corrections
  TH2F nCrystalCorrection("nCrystalCorrection", "nCrys correction vs bin;bin = thetaID + 69*energyBin;nCrys", nbinX, 0, nbinX,
                          maxN + 1, 0, maxN + 1);

  ix = 0;
  for (int ie = 0; ie < nEnergies; ie++) {
    for (int thID = 0; thID < nThetaID; thID++) {
      ix++;
      for (int in = 0; in <= maxN; in++) {
        int iy = in + 1;
        float correction = nCrysCorrection[ie][thID][in];
        float corrUnc = nCrysCorrectionUnc[ie][thID][in];
        nCrystalCorrection.SetBinContent(ix, iy, correction);
        nCrystalCorrection.SetBinError(ix, iy, corrUnc);
      }
    }
  }


  //====================================================================================
  //====================================================================================
  //..Step 8. Diagnostic histograms

  //..Start by storing the payload histograms
  histfile->cd();
  thetaCorrection.Write();
  phiCorrection.Write();
  nCrystalCorrection.Write();

  //-----------------------------------------------------------------------------------
  //..One histogram of new and original reconstructed energy after leakage correction
  //  per generated energy per region. Also uncorrected.
  const int nResType = 4;
  TString resName[nResType] = {"Uncorrected", "Original", "Corrected measured", "Corrected true"};
  TString regName[nLeakReg] = {"forward", "barrel", "backward"};
  TH1F* energyResolution[nLeakReg][nEnergies][nResType];

  //..Base number of bins on a typical thetaID for each region
  int thIDReg[nLeakReg];
  thIDReg[0] = 9;
  thIDReg[1] = 35;
  thIDReg[2] = 61;

  for (int ireg = 0; ireg < nLeakReg; ireg++) {
    for (int ie = 0; ie < nEnergies; ie++) {

      //..Titles and bins for this region and energy
      TString stireg = std::to_string(ireg);
      TString stie = std::to_string(ie);
      TString stieName = std::to_string(iEnergiesMeV[ie][thIDReg[ireg]]);
      int nbinReg = 20 * nEfracBins[ie][thIDReg[ireg]];

      for (int ires = 0; ires < nResType; ires++) {
        name = "energyResolution_" + stireg + "_" + stie + "_" + std::to_string(ires);
        title = resName[ires] + " energy, " + stieName + " MeV, " + regName[ireg] + ";Reconstructed E/Etrue";
        energyResolution[ireg][ie][ires] = new TH1F(name, title, nbinReg, eFracLo, eFracHi);
      }
    }
  }

  //-----------------------------------------------------------------------------------
  //..Loop over events and store leakage-corrected energies
  for (int i = 0; i < treeEntries; i++) {
    tree->GetEntry(i);

    //..Only events with good reconstruction
    bool goodReco = t_thetaID >= firstUsefulThID and t_thetaID <= lastUsefulThID and t_energyBin >= 0
                    and t_locationError < maxLocCut[t_energyBin][t_thetaID];
    if (not goodReco) {continue;}

    //-----------------------------------------------------------------------------------
    //..Corrections using true energy

    //..Position-dependent leakage corrections using true energy
    int idir = 0;
    float thetaPosCor = positionCorrection[t_energyBin][t_thetaID][idir][t_thetaBin];

    idir = t_phiMech + 1;
    float phiPosCor = positionCorrection[t_energyBin][t_thetaID][idir][t_phiBin];
    float posCor = thetaPosCor * phiPosCor;

    //..nCrystal-dependent leakage correction using true energy
    float nCrysCor = nCrysCorrection[t_energyBin][t_thetaID][t_nCrys];
    float corrTrue = posCor * nCrysCor;

    //-----------------------------------------------------------------------------------
    //..Find correction using measured energy. The correction is a function of corrected
    //  energy, so will need to iterate
    float corrMeasured = 0.95; // typical correction as starting point
    for (int iter = 0; iter < 2; iter++) {


      //..Energy points that bracket this value
      float energyRaw = t_energyFrac * generatedE[t_region][t_energyBin];
      float logEnergy = log(energyRaw / corrMeasured);
      int ie0 = 0; // lower energy point
      if (logEnergy < leakLogE[t_region][0]) {
        ie0 = 0;
      } else if (logEnergy > leakLogE[t_region][nEnergies - 1]) {
        ie0 = nEnergies - 2;
      } else {
        while (logEnergy > leakLogE[t_region][ie0 + 1]) {ie0++;}
      }

      //..Corrections from lower and upper energy points
      float cor0 = positionCorrection[ie0][t_thetaID][0][t_thetaBin] * positionCorrection[ie0][t_thetaID][t_phiMech + 1][t_phiBin] *
                   nCrysCorrection[ie0][t_thetaID][t_nCrys];
      float cor1 = positionCorrection[ie0 + 1][t_thetaID][0][t_thetaBin] * positionCorrection[ie0 + 1][t_thetaID][t_phiMech +
                   1][t_phiBin] * nCrysCorrection[ie0 + 1][t_thetaID][t_nCrys];

      //..Interpolate in logE
      corrMeasured = cor0 + (cor1 - cor0) * (logEnergy - leakLogE[t_region][ie0]) / (leakLogE[t_region][ie0 + 1] -
                     leakLogE[t_region][ie0]);
    }

    //-----------------------------------------------------------------------------------
    //..Fill the histograms
    energyResolution[t_region][t_energyBin][0]->Fill(t_energyFrac); // uncorrected
    energyResolution[t_region][t_energyBin][1]->Fill(t_origEnergyFrac); // original
    energyResolution[t_region][t_energyBin][2]->Fill(t_energyFrac / corrMeasured); // corrected, measured energy
    energyResolution[t_region][t_energyBin][3]->Fill(t_energyFrac / corrTrue); // corrected, true energy
  }

  //-----------------------------------------------------------------------------------
  //..Fit each histogram to find peak, and extract resolution.

  //..Store the peak and resolution values for each histogram
  float peakEnergy[nLeakReg][nEnergies][nResType];
  float energyRes[nLeakReg][nEnergies][nResType];

  //..Loop over the histograms to be fit
  for (int ireg = 0; ireg < nLeakReg; ireg++) {
    for (int ie = 0; ie < nEnergies; ie++) {

      //..Base the resolution on the number of entries in the corrected plot for all 4 cases.
      double entries = energyResolution[ireg][ie][nResType - 1]->Integral();
      for (int ires = 0; ires < nResType; ires++) {
        TH1F* hEnergy = (TH1F*)energyResolution[ireg][ie][ires];
        double peak = -1.;
        double res68 = 99.; // resolution based on 68.3% of the entries
        int nIter = 0; // keep track of attempts to fit this histogram
        bool fitHist = entries > minEntries;

        //..Possibly iterate fit starting from this point
        while (fitHist) {

          //..Fit parameters
          double norm = hEnergy->GetMaximum();
          double target = fracEnt[nIter] * entries; // fit range contains 68.3% or 50%
          std::vector<double> startingParameters;// peak, sigma, eta, fitLow, fitHigh, nbins
          startingParameters = eclLeakageFitParameters(hEnergy, target);
          func->SetParameters(norm, startingParameters[0], startingParameters[1], startingParameters[2]);
          func->SetParLimits(1, startingParameters[3], startingParameters[4]);
          func->SetParLimits(2, 0., startingParameters[4] - startingParameters[3]);
          func->SetParLimits(3, etaMin, etaMax);

          //..First iteration the fitting range contains >68.3% of the histogram integral.
          if (nIter == 0) {

            //..Find the bin numbers used in the fit
            int minLo = hEnergy->GetXaxis()->FindBin(startingParameters[3] + 0.0001);
            int minHi = hEnergy->GetXaxis()->FindBin(startingParameters[4] - 0.0001);

            //..Subtract a partial bin to get to exactly 68.3%
            double dx = hEnergy->GetBinLowEdge(minLo + 1) - hEnergy->GetBinLowEdge(minLo);
            double overage = hEnergy->Integral(minLo, minHi) - target;
            double subLo = overage / hEnergy->GetBinContent(minLo);
            double subHi = overage / hEnergy->GetBinContent(minHi);

            //..Resolution is half the range that contains 68.3% of the events
            res68 = 0.5 * dx * (1 + minHi - minLo - max(subLo, subHi));
          }

          //..Fit
          name = hEnergy->GetName();
          std::cout << "Fitting " << name.Data() << std::endl;
          hEnergy->Fit(func, "LIEQ", "", startingParameters[3], startingParameters[4]);
          peak = func->GetParameter(1);
          double effSigma = func->GetParameter(2);
          double eta = func->GetParameter(3);
          double prob = func->GetProb();

          //..Check fit quality  0 = good, 1 = redo fit, 2 = lowStat, 3 = lowProb,
          //  4 = peakAtLimit, 5 = sigmaAtLimit, 6 = etaAtLimit
          double dEta = min((etaMax - eta), (eta - etaMin));
          int fitStatus = eclLeakageFitQuality(startingParameters[3], startingParameters[4], peak, effSigma, dEta, prob);

          //..If the fit probability is low, refit using a smaller range (fracEnt)
          if ((fitStatus == 1 or fitStatus == 3) and nIter == 0) {
            nIter++;
          } else {
            fitHist = false;
          }

        } // end of while

        //..Record peak and resolution
        peakEnergy[ireg][ie][ires] = peak;
        energyRes[ireg][ie][ires] = res68;

        //..And write to disk
        histfile->cd();
        hEnergy->Write();
      }
    }
  }

  //-----------------------------------------------------------------------------------
  //..Summarize resolution
  int nresBins = nEnergies * nLeakReg * (nResType + 1); // +1 to add an empty bin after each set of 4
  TH1F* peakSummary = new TH1F("peakSummary", "Peak E/Etrue for each method, region, energy;Energy energy point", nresBins, 0,
                               nEnergies);
  TH1F* resolutionSummary = new TH1F("resolutionSummary", "Resolution/peak for each method, region, energy;Energy energy point",
                                     nresBins, 0, nEnergies);

  std::cout << std::endl << "Energy bin, region, Resolution/peak 4 ways: " << std::endl;
  ix = 0;
  for (int ie = 0; ie < nEnergies; ie++) {
    for (int ireg = 0; ireg < nLeakReg; ireg++) {
      std::printf("%2d %1d ", ie, ireg);
      for (int ires = 0; ires < nResType; ires++) {

        //..Store in summary histograms
        ix++;
        peakSummary->SetBinContent(ix, peakEnergy[ireg][ie][ires]);
        peakSummary->SetBinError(ix, 0.);
        resolutionSummary->SetBinContent(ix, energyRes[ireg][ie][ires] / peakEnergy[ireg][ie][ires]);
        resolutionSummary->SetBinError(ix, 0.);

        //..Print out as well
        std::printf("%6.4f ", energyRes[ireg][ie][ires] / peakEnergy[ireg][ie][ires]);
      }
      ix++;
      std::cout << std::endl;
    }
  }

  //..Write the summary histograms to disk
  histfile->cd();
  peakSummary->Write();
  resolutionSummary->Write();


  //====================================================================================
  //====================================================================================
  //..Step 9. Finish up.

  //-----------------------------------------------------------------------------------
  //..Close histogram file
  histfile->Close();

  //------------------------------------------------------------------------
  //..Create and store payload
  ECLLeakageCorrections* leakagePayload = new  ECLLeakageCorrections();
  leakagePayload->setlogEnergiesFwd(forwardVector);
  leakagePayload->setlogEnergiesBrl(barrelVector);
  leakagePayload->setlogEnergiesBwd(backwardVector);
  leakagePayload->setThetaCorrections(thetaCorrection);
  leakagePayload->setPhiCorrections(phiCorrection);
  leakagePayload->setnCrystalCorrections(nCrystalCorrection);
  saveCalibration(leakagePayload, "ECLLeakageCorrections");

  B2RESULT("eclLeakageAlgorithm: successfully stored payload ECLLeakageCorrections");
  return c_OK;
}
