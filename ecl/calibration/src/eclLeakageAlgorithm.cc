/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/calibration/eclLeakageAlgorithm.h>

/* ECL headers. */
#include <ecl/calibration/tools.h>
#include <ecl/dbobjects/ECLLeakageCorrections.h>

/* Basf2 headers. */
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>

/* ROOT headers. */
#include <TF1.h>
#include <TFile.h>
#include <TH1D.h>
#include <TTree.h>

/* C++ headers. */
#include <iostream>

using namespace std;
using namespace Belle2;
using namespace ECL;
using namespace Calibration;

/**************************************************************************
 * eclLeakageAlgorithm analyzes single photon MC to find the eclLeakage payload
 * After initial setup, the position dependent corrections are found using
 * photons with good reconstruction.
 * The correction that depends on the number of crystals has been replaced
 * by the ECLnOptimal payload.
 *
 * Major steps in the process:
 * 1. Set up (read in parameters and tree, define histogram binning)
 * 2. Fill and fit histograms of normalized reconstructed energy, one per
 *    thetaID/energy. Novosibirsk fit parameter eta is floated in this fit, then
 *    fixed for fits of data with finer location binning. Peak (i.e. overall correction
 *    for this thetaID/energy) is used to normalize position-dependent correction.
 * 3. Fill and fit histograms of normalized energy for each location (nominally 29
 *    locations in theta and phi per thetaID) to get the position-dependent correction.
 *    Nominally 69 x 8 x 29 x 3 = 48,024 histograms. 3 = theta, phi next to mech, phi not
 *    next to mech.
 * 4. Pack payload quantities into vectors and histograms
 * 5. Fill and fit summary and resolution histograms
 * 6. Finish; close histogram file and and write payload

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
// cppcheck-suppress constParameter ; TF1 fit functions cannot have const parameters
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
  auto generatedE = create2Dvector<float>(nLeakReg, nEnergies); // 3 regions forward barrel backward

  int bin = 2; // bin 1 = nPositions, bin 2 = nEnergies, bin 3 = first energy
  for (int ireg = 0; ireg < nLeakReg; ireg++) {
    B2INFO("Generated energies for ireg = " << ireg);
    for (int ie = 0; ie < nEnergies; ie++) {
      bin++;
      generatedE[ireg][ie] = inputParameters->GetBinContent(bin);
      B2INFO("  " << ie << " " << generatedE[ireg][ie] << " GeV");
    }
  }
  B2INFO("Low energy threshold = " << m_lowEnergyThreshold);

  //..Energy per thetaID (in MeV, for use in titles etc)
  auto iEnergiesMeV = create2Dvector<int>(nEnergies, nThetaID);
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
  auto nEfracBins = create2Dvector<int>(nEnergies, nThetaID);
  for (int thID = 0; thID < nThetaID; thID++) {
    B2DEBUG(25, "eFrac nBins for thetaID " << thID);
    for (int ie = 0; ie < nEnergies; ie++) {

      //..ballpark resolution
      double res_squared = 0.0001 + 0.064 / iEnergiesMeV[ie][thID];

      //..Convert this to an even integer to get number of bins
      double binNumOver2 = 3. / sqrt(res_squared);
      int tempNBin = (int)(binNumOver2 + 0.5);
      nEfracBins[ie][thID] = 2 * tempNBin;
      B2DEBUG(25, " ie = " << ie << " E = " << iEnergiesMeV[ie][thID] << " nBins = " << nEfracBins[ie][thID]);
    }
  }

  //-----------------------------------------------------------------------------------
  //..Set up Novosibirsk fit function
  TString statusString[7] = {"good", "refit", "lowStat", "lowProb", "peakAtLimit", "sigmaAtLimit", "etaAtLimit"}; /**< categories of fit status */
  const double fracEnt[2] = {0.683, 0.5}; // fit range includes 68% or 50% of entries
  const double minEntries = 100.; // don't use fits with fewer entries
  const double minMaxBin = 50.; // rebin if max bin is below this value
  const double highMaxBin = 300.; // can float eta if max bin is above this value

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

  //-----------------------------------------------------------------------------------
  //..Get current payload to help validate the new payload

  //..Set event, run, exp number
  const auto exprun =  getRunList();
  const int iEvt = 1;
  const int iRun = exprun[0].second;
  const int iExp = exprun[0].first;
  StoreObjPtr<EventMetaData> evtPtr;
  DataStore::Instance().setInitializeActive(true);
  evtPtr.registerInDataStore();
  DataStore::Instance().setInitializeActive(false);
  evtPtr.construct(iEvt, iRun, iExp);
  DBStore& dbstore = DBStore::Instance();
  dbstore.update();

  //..Existing payload
  DBObjPtr<ECLLeakageCorrections> existingCorrections("ECLLeakageCorrections");
  TH2F existingThetaCorrection = existingCorrections->getThetaCorrections();
  existingThetaCorrection.SetName("existingThetaCorrection");
  TH2F existingPhiCorrection = existingCorrections->getPhiCorrections();
  existingPhiCorrection.SetName("existingPhiCorrection");

  //..Write out the correction histograms
  histfile->cd();
  existingThetaCorrection.Write();
  existingPhiCorrection.Write();


  //====================================================================================
  //====================================================================================
  //..Step 2. First loop, fill histograms of e/eTrue for each energy and thetaID

  //-----------------------------------------------------------------------------------
  //..One histogram of e/eTrue per energy per thetaID.
  //  Used to fix eta in subsequent fits, and to get overall correction for that thetaID
  //  (which should be very close to 1).
  TString name;
  TString title;
  auto hELabUncorr = create2Dvector<TH1F*>(nEnergies, nThetaID);
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
    bool goodReco = t_thetaID >= firstUsefulThID and t_thetaID <= lastUsefulThID and t_energyBin >= 0;
    if (not goodReco) {continue;}

    //..Fill histogram for full thetaID
    hELabUncorr[t_energyBin][t_thetaID]->Fill(t_energyFrac);
  }

  //-----------------------------------------------------------------------------------
  //..Fit each thetaID/energy histogram to get peak (overall correction) and eta (fixed
  //  in subsequent fits to individual locations).
  auto peakUncorr = create2Dvector<float>(nEnergies, nThetaID); // store peak from each fit
  auto etaUncorr  = create2Dvector<float>(nEnergies, nThetaID); // store eta from each fit

  std::vector<TString> failedELabUncorr; // names of hists with failed fits
  std::vector<int> statusELabUncorr; // status of failed fits
  int payloadStatus = 0; // Overall status of payload determination

  //..Record fit status
  TH1F* statusOfhELabUncorr = new TH1F("statusOfhELabUncorr",
                                       "status of hELabUncorr fits for each thetaID/energy. 0 = good, 1 = redo fit, 2 = lowStat, 3 = lowProb, 4 = peakAtLimit, 5 = sigmaAtLimit",
                                       6, 0,
                                       6);


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
      double genE = iEnergiesMeV[ie][thID] / 1000.;
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
        B2DEBUG(40, "Fitting " << name.Data());
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
      //  Mark payload as failed if energy is above low-energy threshold.
      statusOfhELabUncorr->Fill(fitStatus + 0.000001);
      if (fitStatus == 2 or fitStatus >= 4) {
        statusELabUncorr.push_back(fitStatus);
        failedELabUncorr.push_back(hEnergy->GetName());
        if (genE > m_lowEnergyThreshold) {
          payloadStatus = 1; // algorithm failed
        } else {
          peak = -1.; // fix up later
        }
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

  //..Write out summary of fit status
  histfile->cd();
  statusOfhELabUncorr->Write();

  //-----------------------------------------------------------------------------------
  //..Quit now if one of the high-energy fits failed, since we will not get a payload.
  int nbadFit = statusELabUncorr.size();
  if (nbadFit > 0) {B2ERROR("hELabUncorr fit failures (one histogram per energy/thetaID):");}
  for (int ibad = 0; ibad < nbadFit; ibad++) {
    int badStat = statusELabUncorr[ibad];
    B2ERROR(" histogram " << failedELabUncorr[ibad].Data() << " status " << badStat << " " << statusString[badStat].Data());
  }
  if (payloadStatus != 0) {
    B2ERROR("ecLeakageAlgorithm: fit to hELabUncorr failed. ");
    histfile->Close();
    return c_Failure;
  }

  //-----------------------------------------------------------------------------------
  //..Fix up any failed (low-energy) fits by using a neighbouring thetaID
  for (int thID = firstUsefulThID; thID <= lastUsefulThID; thID++) {
    for (int ie = 0; ie < nEnergies; ie++) {
      if (peakUncorr[ie][thID] < 0.) {
        if (thID > 40) {
          for (int nextID = thID - 1; thID >= firstUsefulThID; thID--) {
            if (peakUncorr[ie][nextID] > 0.) {
              peakUncorr[ie][thID] = peakUncorr[ie][nextID];
              break;
            }
          }
        } else {
          for (int nextID = thID + 1; thID <= lastUsefulThID; thID++) {
            if (peakUncorr[ie][nextID] > 0.) {
              peakUncorr[ie][thID] = peakUncorr[ie][nextID];
              break;
            }
          }
        }
      }

      //..If we were unable to get a successful fit from a neighbour, give up
      if (peakUncorr[ie][thID] < 0.) {
        B2ERROR("ecLeakageAlgorithm: unable to correct hELabUncorr for thetaID " << thID << " ie " << ie);
        histfile->Close();
        return c_Failure;
      }
    }
  }

  //====================================================================================
  //====================================================================================
  //..Step 3. Second loop, fill histograms of e/eTrue as a function of position.
  //  29 locations in theta, 29 in phi.
  //  Crystals next to mechanical structure in phi are treated separately from
  //  crystals without mechanical structure.

  //-----------------------------------------------------------------------------------
  //..Histograms to store the energy
  const int nDir = 3;
  const TString dirName[nDir] = {"theta", "phiMech", "phiNoMech"};

  auto eFracPosition  = create4Dvector<TH1F*>(nEnergies, nThetaID, nDir, nPositions); // the histograms


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
                                         "eFrac fit status: -2 noData, -1 lowE, 0 good, 1 redo fit, 2 lowStat, 3 lowProb, 4 peakAtLimit, 5 sigmaAtLimit", 8, -2,
                                         6);
  TH1F* probOfeFracPosition = new TH1F("probOfeFracPosition", "fit probability of eFrac fits for each position;probability", 100, 0,
                                       1);
  TH1F* maxOfeFracPosition = new TH1F("maxOfeFracPosition", "max entries of eFrac histograms;maximum bin content", 100, 0, 1000);

  //-----------------------------------------------------------------------------------
  //..Loop over events and store eFrac
  for (int i = 0; i < treeEntries; i++) {
    tree->GetEntry(i);

    //..Only events with good reconstruction
    bool goodReco = t_thetaID >= firstUsefulThID and t_thetaID <= lastUsefulThID and t_energyBin >= 0;
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
  auto positionCorrection     = create4Dvector<float>(nEnergies, nThetaID, nDir, nPositions);
  auto positionCorrectionUnc  = create4Dvector<float>(nEnergies, nThetaID, nDir, nPositions);
  int nHistToFit = 0;


  //..Temp histogram of position corrections
  TH1F* thetaCorSummary = new TH1F("thetaCorSummary", "Theta dependent corrections;theta dependent correction", 100, 0.4, 1.4);
  TH1F* phiCorSummary = new TH1F("phiCorSummary", "Phi dependent corrections;phi dependent correction", 100, 0.4, 1.4);

  for (int thID = firstUsefulThID; thID <= lastUsefulThID; thID++) {
    for (int ie = 0; ie < nEnergies; ie++) {
      double genE = iEnergiesMeV[ie][thID] / 1000.;
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
          if (genE < m_lowEnergyThreshold) {fitStatus = -1;} // low energy, don't fit, use default peak value
          if (hEnergy->GetEntries() < 0.5) {fitStatus = -2;} // unused, eg barrel pos=2
          int nIter = 0; // keep track of attempts to fit this histogram
          double entries = hEnergy->Integral();
          bool fitHist = entries > minEntries and genE >= m_lowEnergyThreshold;

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
            B2DEBUG(40, "Fitting " << name.Data());
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

          //..Summary histogram for successful fits. Note that fitStatus<0 also has prob<0.
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
  B2INFO("eclLeakageAlgorithm: " << nHistToFit << " eFracPosition histograms to fit");
  nbadFit = statuseFracPosition.size();
  B2INFO("eFracPosition failed fits: " << nbadFit);
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
  //..Step 4. Store quantities needed for the payloads

  //-----------------------------------------------------------------------------------
  //..First, we need to fix up the thetaID that have been so far missed.
  //  Just copy the values from the first or last thetaID for which corrections were found.

  //..ThetaID before the first useful one
  for (int thID = firstUsefulThID - 1; thID >= 0; thID--) {
    for (int ie = 0; ie < nEnergies; ie++) {

      for (int idir = 0; idir < 3; idir++) {
        for (int ipos = 0; ipos < nPositions; ipos++) {
          positionCorrection[ie][thID][idir][ipos] = positionCorrection[ie][thID + 1][idir][ipos];
          positionCorrectionUnc[ie][thID][idir][ipos] = positionCorrectionUnc[ie][thID + 1][idir][ipos];
        }
      }
    }
  }

  //..ThetaID beyond last useful one
  for (int thID = lastUsefulThID + 1; thID < nThetaID; thID++) {
    for (int ie = 0; ie < nEnergies; ie++) {

      for (int idir = 0; idir < 3; idir++) {
        for (int ipos = 0; ipos < nPositions; ipos++) {
          positionCorrection[ie][thID][idir][ipos] = positionCorrection[ie][thID - 1][idir][ipos];
          positionCorrectionUnc[ie][thID][idir][ipos] = positionCorrectionUnc[ie][thID - 1][idir][ipos];
        }
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
  //..nCrys dependent corrections; not used since nOptimal was implemented.
  const int maxN = 21; // maximum number of crystals in a cluster
  TH2F nCrystalCorrection("nCrystalCorrection", "nCrys correction vs bin;bin = thetaID + 69*energyBin;nCrys", nbinX, 0, nbinX,
                          maxN + 1, 0, maxN + 1);

  ix = 0;
  for (int ie = 0; ie < nEnergies; ie++) {
    for (int thID = 0; thID < nThetaID; thID++) {
      ix++;
      for (int in = 0; in <= maxN; in++) {
        int iy = in + 1;
        float correction = 1.;
        float corrUnc = 0.;
        nCrystalCorrection.SetBinContent(ix, iy, correction);
        nCrystalCorrection.SetBinError(ix, iy, corrUnc);
      }
    }
  }


  //====================================================================================
  //====================================================================================
  //..Step 5. Diagnostic histograms

  //..Start by storing the payload histograms
  histfile->cd();
  thetaCorrection.Write();
  phiCorrection.Write();
  nCrystalCorrection.Write();

  //-----------------------------------------------------------------------------------
  //..One histogram of new and original reconstructed energy after leakage correction
  //  per generated energy per region. Also uncorrected.
  //  The "Corrected no nCrys" and "Corrected measured" are identical, but keep both
  //  for easier comparison with earlier results.
  const int nResType = 5;
  const TString resName[nResType] = {"Uncorrected", "Original", "Corrected no nCrys", "Corrected measured", "Corrected true"};
  const TString regName[nLeakReg] = {"forward", "barrel", "backward"};
  auto energyResolution = create3Dvector<TH1F*>(nLeakReg, nEnergies, nResType);

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
    bool goodReco = t_thetaID >= firstUsefulThID and t_thetaID <= lastUsefulThID and t_energyBin >= 0;
    if (not goodReco) {continue;}

    //-----------------------------------------------------------------------------------
    //..Corrections using true energy

    //..Position-dependent leakage corrections using true energy
    int idir = 0;
    float thetaPosCor = positionCorrection[t_energyBin][t_thetaID][idir][t_thetaBin];

    idir = t_phiMech + 1;
    float phiPosCor = positionCorrection[t_energyBin][t_thetaID][idir][t_phiBin];
    float corrTrue = thetaPosCor * phiPosCor;

    //-----------------------------------------------------------------------------------
    //..Find correction using measured energy. The correction is a function of corrected
    //  energy, so will need to iterate
    float corrMeasured = 0.96; // typical correction as starting point
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
      float cor0 = positionCorrection[ie0][t_thetaID][0][t_thetaBin] * positionCorrection[ie0][t_thetaID][t_phiMech + 1][t_phiBin];
      float cor1 = positionCorrection[ie0 + 1][t_thetaID][0][t_thetaBin] * positionCorrection[ie0 + 1][t_thetaID][t_phiMech +
                   1][t_phiBin];

      //..Interpolate in logE
      corrMeasured = cor0 + (cor1 - cor0) * (logEnergy - leakLogE[t_region][ie0]) / (leakLogE[t_region][ie0 + 1] -
                     leakLogE[t_region][ie0]);
    }

    //..No longer have a separate case that excludes the nCrys correction
    float corrNonCrys = corrMeasured;

    //-----------------------------------------------------------------------------------
    //..Fill the histograms
    energyResolution[t_region][t_energyBin][0]->Fill(t_energyFrac); // uncorrected
    energyResolution[t_region][t_energyBin][1]->Fill(t_origEnergyFrac); // original payload
    energyResolution[t_region][t_energyBin][2]->Fill(t_energyFrac / corrNonCrys); // no nCrys, measured energy
    energyResolution[t_region][t_energyBin][3]->Fill(t_energyFrac / corrMeasured); // corrected, measured energy
    energyResolution[t_region][t_energyBin][4]->Fill(t_energyFrac / corrTrue); // corrected, true energy
  }

  //-----------------------------------------------------------------------------------
  //..Fit each histogram to find peak, and extract resolution.

  //..Store the peak and resolution values for each histogram
  auto peakEnergy = create3Dvector<float>(nLeakReg, nEnergies, nResType);
  auto energyRes  = create3Dvector<float>(nLeakReg, nEnergies, nResType);

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
          B2DEBUG(40, "Fitting " << name.Data());
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
  int nresBins = nEnergies * nLeakReg * (nResType + 1); // +1 to add an empty bin after each set
  TH1F* peakSummary = new TH1F("peakSummary", "Peak E/Etrue for each method, region, energy;Energy energy point", nresBins, 0,
                               nEnergies);
  TH1F* resolutionSummary = new TH1F("resolutionSummary", "Resolution/peak for each method, region, energy;Energy energy point",
                                     nresBins, 0, nEnergies);

  B2INFO("Resolution divided by peak for each energy bin and region " << nResType << " ways");
  for (int ires = 0; ires < nResType; ires++) {B2INFO(" " << resName[ires]);}
  ix = 0;
  for (int ie = 0; ie < nEnergies; ie++) {
    B2INFO("  energy point " << ie);
    for (int ireg = 0; ireg < nLeakReg; ireg++) {
      B2INFO("    region " << ireg);
      for (int ires = 0; ires < nResType; ires++) {

        //..Store in summary histograms
        ix++;
        peakSummary->SetBinContent(ix, peakEnergy[ireg][ie][ires]);
        peakSummary->SetBinError(ix, 0.);
        resolutionSummary->SetBinContent(ix, energyRes[ireg][ie][ires] / peakEnergy[ireg][ie][ires]);
        resolutionSummary->SetBinError(ix, 0.);

        //..Print out as well
        B2INFO("      " << ires << " " << energyRes[ireg][ie][ires] / peakEnergy[ireg][ie][ires]);
      }
      ix++;
    }
  }

  //..Write the summary histograms to disk
  histfile->cd();
  peakSummary->Write();
  resolutionSummary->Write();


  //====================================================================================
  //====================================================================================
  //..Step 6. Finish up.

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

  B2INFO("eclLeakageAlgorithm: successfully stored payload ECLLeakageCorrections");
  return c_OK;
}
