/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/calibration/eclNOptimalAlgorithm.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLnOptimal.h>

/* Basf2 headers. */
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>

/* ROOT headers. */
#include <TF1.h>
#include <TH2F.h>
#include <TMath.h>
#include <TROOT.h>

using namespace std;
using namespace Belle2;
using namespace ECL;
using namespace Calibration;

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//..Structure
//  1. Setup
//    - from collector histograms, read in job parameters and group number for each crystal
//    - get the existing payload, which we will use as the starting point for nOptimal
//    - write out histograms from collector and from the existing payload
//
//  2. Create histograms to store many items from many fits
//
//  3. Loop over each energy and group to find the optimal number of crystals
//    - start by checking the current value of nOptimal, then check smaller and larger
//      values of number of summed crystals. For value, find:
//      - peak contained energy divided by generated energy by fitting the histogram
//        range that includes 50% of events;
//      - corresponding bias = sum of ECLCalDigits minus mc true energy;
//      - resolution = one-half of minimum range that includes 68.3% of events
//    - after finding the value of nOptimal with best resolution, also find the
//      resolution for the current reconstruction.
//    - store everything in diagnostic histograms.
//
//  4. Having found nOptimal for that group/energy, find the bias and peak fractional
//     contained energy for adjacent energies. This enables energy interpolation
//     when the payloads are used.
//
//  5. Generate and store the actual payload


//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//..Some helper functions

//-----------------------------------------------------------------------------------
//..Fit the minimum range of bins that contain the specified number of events.
std::vector<int> eclNOptimalFitRange(TH1D* h, const double& fraction)
{
  const double target = fraction * h->GetEntries();
  const int nBins = h->GetNbinsX();

  //..Start at the histogram maximum
  int iLo = h->GetMaximumBin();
  int iHi = iLo;
  double sum = h->Integral(iLo, iHi);

  //..Add one bin at a time
  while (sum < target and (iLo > 1 or iHi < nBins)) {
    double nextLo = 0.;
    if (iLo > 1) {nextLo = h->GetBinContent(iLo - 1);}
    double nextHi = 0.;
    if (iHi < nBins) {nextHi = h->GetBinContent(iHi + 1);}
    if (nextLo > nextHi) {
      sum += nextLo;
      iLo--;
    } else {
      sum += nextHi;
      iHi++;
    }
  }

  std::vector<int> iBins;
  iBins.push_back(iLo);
  iBins.push_back(iHi);
  return iBins;

}

//-----------------------------------------------------------------------------------
//..Resolution is the minimum range that contains 68.3% of entries
double eclNOptimalResolution(TH1D* h, int& iLo75, int& iHi75)
{

  //..Search among the bin range that contains 75% of events for the smallest
  //  range that contains at least 68.3%. If more than one, pick the one with
  //  the most events.
  const int nBins = h->GetNbinsX();
  const double target = 0.683 * h->GetEntries();

  //..Copy the histogram contents for speed of access
  //  Recall that the first histogram bin is 1, not 0.
  std::vector<double> intVector;
  intVector.push_back(h->GetBinContent(1));
  for (int iL = 2; iL <= nBins; iL++) {
    double nextIntegral = intVector[iL - 2] + h->GetBinContent(iL);
    intVector.push_back(nextIntegral);
  }

  //..Now search all possible ranges
  int iLo = iLo75;
  int iHi = iHi75;
  double maxIntegral = intVector[iHi - 1] - intVector[iLo - 2];
  for (int iL = iLo75; iL <= iHi75; iL++) {
    for (int iH = iL; iH <= iHi75; iH++) {

      // sum[iL, iH] = sum[1, iH] - sum[1,iL-1] = intVector[iH-1] - intVector[iL-2]
      double integral = intVector[iH - 1] - intVector[iL - 2];
      if ((integral > target and (iH - iL) < (iHi - iLo)) or
          (integral > target and (iH - iL) == (iHi - iLo) and integral > maxIntegral)
         ) {
        iLo = iL;
        iHi = iH;
        maxIntegral = integral;
      }
    }
  }

  //..Refine the range to exactly 68.3% by subtracting a fraction of the first or last bin.
  //  Use fit to estimate distribution of events across the bin.
  const double overage = h->Integral(iLo, iHi) - target;
  const double dx = (h->GetXaxis()->GetXmax() - h->GetXaxis()->GetXmin()) / nBins;

  //..Fraction of the first bin we could exclude
  double xLow = h->GetBinLowEdge(iLo);
  double fracEntriesToExcludeLo = overage / h->GetBinContent(iLo);
  double fracBinToExcludeLo = fracEntriesToExcludeLo;

  //..Use the slope from the fit extrapolated to this point unless it is 0
  TF1* func = (TF1*)h->GetFunction("eclNOptimalNovo");
  double f0 = func->Eval(xLow);
  double f1 = func->Eval(xLow + dx);
  if (abs(f1 - f0) > 1.) {
    fracBinToExcludeLo = (sqrt(f0 * f0 + fracEntriesToExcludeLo * (f1 * f1 - f0 * f0)) - f0) / (f1 - f0);
  }

  //..Last bin. In this case, we want the integral from the low edge of the bin to
  //  be the fraction of events to keep in the range, not the fraction to exclude.
  double xHigh = h->GetBinLowEdge(iHi + 1);
  f0 = func->Eval(xHigh - dx);
  f1 = func->Eval(xHigh);
  double fracEntriesToExcludeHi = overage / h->GetBinContent(iHi);
  double fracEntriesToInclude = 1. - fracEntriesToExcludeHi;
  double fracBinToInclude = fracEntriesToInclude;
  if (abs(f1 - f0) > 1.) {
    fracBinToInclude = (sqrt(f0 * f0 + fracEntriesToInclude * (f1 * f1 - f0 * f0)) - f0) / (f1 - f0);
  }
  double fracBinToExcludeHi = 1. - fracBinToInclude;

  //..Exclude the first bin or last bin fraction, whichever gives better resolution
  double rangeBins =  1 + iHi - iLo - max(fracBinToExcludeLo, fracBinToExcludeHi);
  return 0.5 * dx * rangeBins;

}

//--------------------------------------------------------------------
//..Novosibirsk; H. Ikeda et al. / NIM A 441 (2000) 401-426
double eclNOptimalNovo(const double* x, const double* par)
{

  double peak = par[1];
  double width = par[2];
  double sln4 = sqrt(log(4));
  double y = par[3] * sln4;
  double tail = -log(y + sqrt(1 + y * y)) / sln4;
  double qc = 0.;

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

//-----------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//..Constructor

eclNOptimalAlgorithm::eclNOptimalAlgorithm(): CalibrationAlgorithm("eclNOptimalCollector")
{
  setDescription(
    "Use single gamma MC to find the optimal number of crystals to sum for cluster energy"
  );
}

//-------------------------------------------------------------------------------
//..Calibrate
CalibrationAlgorithm::EResult eclNOptimalAlgorithm::calibrate()
{

  //-----------------------------------------------------------------------------------
  //-----------------------------------------------------------------------------------
  //..Algorithm set up

  //..Get started
  B2INFO("Starting eclNOptimalAlgorithm");
  gROOT->SetBatch();

  //-----------------------------------------------------------------------------------
  //..Read in parameter histograms created by the collector and fix normalization
  auto inputParameters = getObjectPtr<TH1F>("inputParameters");
  const int lastBin = inputParameters->GetNbinsX();
  const double scale = inputParameters->GetBinContent(lastBin); // number of times inputParameters was filled
  for (int ib = 1; ib < lastBin; ib++) {
    double param = inputParameters->GetBinContent(ib);
    inputParameters->SetBinContent(ib, param / scale);
    inputParameters->SetBinError(ib, 0.);
  }

  //..Also read in and normalize group number for each cellID
  auto groupNumberOfEachCellID = getObjectPtr<TH1F>("groupNumberOfEachCellID");
  for (int ic = 1; ic <= ECLElementNumbers::c_NCrystals; ic++) {
    const double groupNum = groupNumberOfEachCellID->GetBinContent(ic);
    groupNumberOfEachCellID->SetBinContent(ic, groupNum / scale);
    groupNumberOfEachCellID->SetBinError(ic, 0.);
  }

  //..Write these to disk.
  TFile* histFile = new TFile("eclNOptimalAlgorithm.root", "recreate");
  inputParameters->Write();
  groupNumberOfEachCellID->Write();

  //-----------------------------------------------------------------------------------
  //..Parameters from the inputParameters histogram
  const int nCrystalGroups = (int)(inputParameters->GetBinContent(1) + 0.5);
  const int nEnergies = (int)(inputParameters->GetBinContent(2) + 0.5);
  const int nLeakReg = 3; // 3 regions forward barrel backward
  float generatedE[nLeakReg][nEnergies];
  int bin = 2; // bin 1 = nCrystalGroups, bin 2 = nEnergies, bin 3 = first energy
  for (int ireg = 0; ireg < nLeakReg; ireg++) {
    for (int ie = 0; ie < nEnergies; ie++) {
      bin++;
      generatedE[ireg][ie] = inputParameters->GetBinContent(bin);
    }
  }
  B2INFO("nCrystalGroups = " << nCrystalGroups);

  //-----------------------------------------------------------------------------------
  //..Experiment and run number, for reading existing payload from database
  const auto exprun =  getRunList();
  const int iExp = exprun[0].first;
  const int iRun = exprun[0].second;
  B2INFO("Experiment, run = " << iExp << ", " << iRun);

  StoreObjPtr<EventMetaData> evtPtr;
  // simulate the initialize() phase where we can register objects in the DataStore
  DataStore::Instance().setInitializeActive(true);
  evtPtr.registerInDataStore();
  DataStore::Instance().setInitializeActive(false);
  // now construct the event metadata
  evtPtr.construct(1, iRun, iExp);
  // and update the database contents
  DBStore& dbstore = DBStore::Instance();
  dbstore.update();
  // this is only needed it the payload might be intra-run dependent,
  // that is if it might change during one run as well
  dbstore.updateEvent();

  //-----------------------------------------------------------------------------------
  //..Get existing payload
  B2INFO("eclNOptimalAlgorithm: reading previous payload");
  DBObjPtr<ECLnOptimal> existingECLnOptimal;
  std::vector<float> eUpperBoundariesFwdPrev = existingECLnOptimal->getUpperBoundariesFwd();
  std::vector<float> eUpperBoundariesBrlPrev = existingECLnOptimal->getUpperBoundariesBrl();
  std::vector<float> eUpperBoundariesBwdPrev = existingECLnOptimal->getUpperBoundariesBwd();
  TH2F nOptimal2DPrev = existingECLnOptimal->getNOptimal();
  nOptimal2DPrev.SetName("nOptimal2DPrev");

  const int nPrevE = eUpperBoundariesFwdPrev.size();
  B2INFO("Energy upper boundaries from previous payload for each region");
  for (int ie = 0; ie < nPrevE; ie++) {
    B2INFO(" " << ie << " " << eUpperBoundariesFwdPrev[ie] << " " << eUpperBoundariesBrlPrev[ie] << " " << eUpperBoundariesBwdPrev[ie]);
  }

  //..Write out
  histFile->cd();
  nOptimal2DPrev.Write();

  //-----------------------------------------------------------------------------------
  //..Use existing payload to get the initial nOptimal value for each group
  std::vector< std::vector<int> > initialNOptimal;
  initialNOptimal.resize(nCrystalGroups, std::vector<int>(nEnergies, 0));

  //..Couple of histograms of relevant quantities
  TH2F* nInitialPerGroup = new TH2F("nInitialPerGroup", "initial nCrys, energy point vs group number;group;energy point",
                                    nCrystalGroups, 0., nCrystalGroups, nEnergies, 0., nEnergies);
  TH2F* generatedEPerGroup = new TH2F("generatedEPerGroup", "Generated energy, energy point vs group number;group;energy point",
                                      nCrystalGroups, 0., nCrystalGroups, nEnergies, 0., nEnergies);
  TH1F* firstCellIDPerGroup = new TH1F("firstCellIDPerGroup", "First cellID of group;group", nCrystalGroups, 0., nCrystalGroups);
  TH1F* nCrystalsPerGroup = new TH1F("nCrystalsPerGroup", "Number of crystals per group;group", nCrystalGroups, 0., nCrystalGroups);
  std::vector<int> crystalsPerGroup;
  crystalsPerGroup.resize(nCrystalGroups, 0);

  const int iFirstBarrel = 1153; // first barrel cellID
  const int iLastBarrel = 7776; // last barrel cellID
  const int nCrystalsTotal = ECLElementNumbers::c_NCrystals;
  for (int cellID = 1; cellID <= nCrystalsTotal; cellID++) {

    //..Group number of this cellID
    int iGroup = (int)(0.5 + groupNumberOfEachCellID->GetBinContent(cellID));

    //..Energy boundaries of previous payload, which depend on the ECL region
    std::vector<float> eUpperBoundariesPrev = eUpperBoundariesBrlPrev;
    int iRegion = 1; // barrel
    if (cellID < iFirstBarrel) {
      eUpperBoundariesPrev = eUpperBoundariesFwdPrev;
      iRegion = 0;
    } else if (cellID > iLastBarrel) {
      eUpperBoundariesPrev = eUpperBoundariesBwdPrev;
      iRegion = 2;
    }

    //..For each test energy, get the nOptimal for this cellID from previous payload.
    //  iEnergy is the energy bin of the previous payload, which is equal to ie if
    //  the test energies have not changed.
    for (int ie = 0; ie < nEnergies; ie++) {
      float energy = generatedE[iRegion][ie];
      int iEnergy = 0;
      while (energy > eUpperBoundariesPrev[iEnergy]) {iEnergy++;}
      initialNOptimal[iGroup][ie] = (int)(0.5 + nOptimal2DPrev.GetBinContent(iGroup + 1, iEnergy + 1));
      if (ie != iEnergy) {
        B2INFO("ie iEnergy mismatch: cellID " << cellID << " ie " << ie << " iEnergy " << iEnergy);
      }

      //..Store in diagnostic histograms
      generatedEPerGroup->SetBinContent(iGroup + 1, ie + 1, energy);
      nInitialPerGroup->SetBinContent(iGroup + 1, ie + 1, initialNOptimal[iGroup][ie]);
    }

    //..Count crystals in this group
    if (crystalsPerGroup[iGroup] == 0) {
      firstCellIDPerGroup->SetBinContent(iGroup + 1, cellID);
      firstCellIDPerGroup->SetBinError(iGroup + 1, 0.);
    }
    crystalsPerGroup[iGroup]++;
  }

  //..Write out these histograms
  for (int ig = 0; ig < nCrystalGroups; ig++) {
    nCrystalsPerGroup->SetBinContent(ig + 1, crystalsPerGroup[ig]);
    nCrystalsPerGroup->SetBinError(ig + 1, 0.);
  }
  histFile->cd();
  nInitialPerGroup->Write();
  generatedEPerGroup->Write();
  firstCellIDPerGroup->Write();
  nCrystalsPerGroup->Write();
  B2INFO("Wrote initial diagnostic histograms");

  //-----------------------------------------------------------------------------------
  //..Write out all the 2D histograms
  for (int ig = 0; ig < nCrystalGroups; ig++) {
    for (int ie = 0; ie < nEnergies; ie++) {
      std::string name = "eSum_" + std::to_string(ig) + "_" + std::to_string(ie);
      auto eSum = getObjectPtr<TH2F>(name);
      name = "biasSum_" + std::to_string(ig) + "_" + std::to_string(ie);
      auto biasSum = getObjectPtr<TH2F>(name);
      histFile->cd();
      eSum->Write();
      biasSum->Write();
    }
  }
  B2INFO("Wrote eSum and biasSum histograms");

  //-----------------------------------------------------------------------------------
  //-----------------------------------------------------------------------------------
  //..Find nOptimal and corresponding bias for each group/energy point

  //..Histograms to store fit results
  TH2F* nOptimalPerGroup = new TH2F("nOptimalPerGroup", "nOptimal;group;energy point", nCrystalGroups, 0., nCrystalGroups, nEnergies,
                                    0., nEnergies);
  TH2F* changeInNOptimal = new TH2F("changeInNOptimal", "nOptimal minus nInitial;group;energy point", nCrystalGroups, 0.,
                                    nCrystalGroups, nEnergies, 0., nEnergies);
  TH2F* binsInFit = new TH2F("binsInFit", "Number of bins used in Novo fit, energy point vs group number;group;energy point",
                             nCrystalGroups, 0., nCrystalGroups, nEnergies, 0., nEnergies);
  TH2F* maxEntriesPerHist = new TH2F("maxEntriesPerHist",
                                     "Max entries in histogram bin, energy point vs group number;group;energy point",  nCrystalGroups, 0., nCrystalGroups, nEnergies, 0.,
                                     nEnergies);
  TH2F* peakPerGroup = new TH2F("peakPerGroup", "peak energy, energy point vs group number;group;energy point", nCrystalGroups, 0.,
                                nCrystalGroups, nEnergies, 0., nEnergies);
  TH2F* effSigmaPerGroup = new TH2F("effSigmaPerGroup", "fit effective sigma, energy point vs group number;group;energy point",
                                    nCrystalGroups, 0., nCrystalGroups, nEnergies, 0., nEnergies);
  TH2F* etaPerGroup = new TH2F("etaPerGroup", "fit eta, energy point vs group number;group;energy point", nCrystalGroups, 0.,
                               nCrystalGroups, nEnergies, 0., nEnergies);
  TH2F* fitProbPerGroup = new TH2F("fitProbPerGroup", "fit probability, energy point vs group number;group;energy point",
                                   nCrystalGroups, 0., nCrystalGroups, nEnergies, 0., nEnergies);
  TH2F* resolutionPerGroup = new TH2F("resolutionPerGroup", "resolution/(peak-bias), energy point vs group number;group;energy point",
                                      nCrystalGroups, 0., nCrystalGroups, nEnergies, 0., nEnergies);
  TH2F* fracContainedEPerGroup = new TH2F("fracContainedEPerGroup",
                                          "peak fraction of energy contained in nOpt crystals;group;energy point", nCrystalGroups, 0., nCrystalGroups, nEnergies, 0.,
                                          nEnergies);

  TH2F* biasPerGroup = new TH2F("biasPerGroup", "bias (GeV), energy point vs group number;group;energy point", nCrystalGroups, 0.,
                                nCrystalGroups, nEnergies, 0., nEnergies);
  TH2F* sigmaBiasPerGroup = new TH2F("sigmaBiasPerGroup", "sigma bias (GeV), energy point vs group number;group;energy point",
                                     nCrystalGroups, 0., nCrystalGroups, nEnergies, 0., nEnergies);

  TH2F* existingResolutionPerGroup = new TH2F("existingResolutionPerGroup",
                                              "existing resolution/peak, energy point vs group number;group;energy point", nCrystalGroups, 0., nCrystalGroups, nEnergies, 0.,
                                              nEnergies);

  //-----------------------------------------------------------------------------------
  //..Loop over all groups and energy points

  //..Fit function Novosibirsk (xmin, xmax, nparameters)
  TF1* func = new TF1("eclNOptimalNovo", eclNOptimalNovo, 0.4, 1.4, 4);
  func->SetLineColor(kRed);

  for (int ig = 0; ig < nCrystalGroups; ig++) {
    for (int ie = 0; ie < nEnergies; ie++) {

      //..Read in the energy sum vs nCrys for the group and energy point
      std::string name = "eSum_" + std::to_string(ig) + "_" + std::to_string(ie);
      auto eSum = getObjectPtr<TH2F>(name);

      //..Also the corresponding bias histgram
      std::string biasName = "biasSum_" + std::to_string(ig) + "_" + std::to_string(ie);
      auto biasSum = getObjectPtr<TH2F>(biasName);

      //..And the generated energy
      const double genEnergy = generatedEPerGroup->GetBinContent(ig + 1, ie + 1);

      //-----------------------------------------------------------------------------------
      //..Extract the eSum distributions for various nCrys (=nCrysSumToFit) and fit them
      //  to find the best resolution.
      const int nCrysBins = eSum->GetNbinsX();
      const int nCrysMax = nCrysBins - 1; // the last bin is eSum for existing reco

      //..Items to be found and stored in the following while loop
      TH1D* eSumOpt{nullptr}; // 1D projection with the best resolution
      int nFitBins = 0; // keep track of how many bins are included in the fit
      double bestFractionalResolution = 999.;
      double existingFractionalResolution = 999.;
      double biasForNOpt = 0.; // bias from backgrounds for optimal nCrys
      double biasSigmaForNOpt = 0.; // sigma bias
      int nOpt = 0;

      //..Start with the previous optimal value, then try fewer, if possible, then
      //  try more, if possible. End by finding the resolution for the
      //  reconstruction used to produce the single photon MC samples.
      const int initialnCrysSumToFit = initialNOptimal[ig][ie];
      int nCrysSumToFit = initialnCrysSumToFit;
      while (nCrysSumToFit > 0) {

        TH1D* hEnergy = (TH1D*)eSum->ProjectionY("hEnergy", nCrysSumToFit, nCrysSumToFit);
        TString newName = name + "_" + std::to_string(nCrysSumToFit);
        hEnergy->SetName(newName);

        //------------------------------------------------------------------------------
        //..Check stats, and rebin as required
        const double minESumEntries = 800.;
        if (hEnergy->GetEntries() < minESumEntries) {
          B2INFO("Insuffient entries in eSum: " << hEnergy->GetEntries() << " for group " << ig << " energy point " << ie);
          histFile->Close();
          B2INFO("closed histFile; quitting");
          return c_NotEnoughData;
        }

        //..Rebin if the maximum bin has too few entries
        const double minPeakValue = 300.;
        while (hEnergy->GetMaximum() < minPeakValue) {hEnergy->Rebin(2);}

        //------------------------------------------------------------------------------
        //..Find 50% range for Novo fit to find peak value. Rebin if there are too many bins
        double fitFraction = 0.5;
        int iLo = 1;
        int iHi = hEnergy->GetNbinsX();
        const int maxBinsToFit = 59;
        while ((iHi - iLo) > maxBinsToFit) {
          std::vector<int> iBins = eclNOptimalFitRange(hEnergy, fitFraction);
          iLo = iBins[0];
          iHi = iBins[1];
          if ((iHi - iLo) > maxBinsToFit) {hEnergy->Rebin(2);}
        }

        //..Fit range
        const int nBinsX = hEnergy->GetNbinsX();
        const double fitlow = hEnergy->GetBinLowEdge(iLo);
        const double fithigh = hEnergy->GetBinLowEdge(iHi + 1);
        double sum = hEnergy->Integral(iLo, iHi);
        B2INFO("group " << ig << " ie " << ie << " name " << name << " newName " << newName << " nBinsX " << nBinsX << " 50% target: " <<
               fitFraction * hEnergy->GetEntries() << " iLo: " << iLo << " iHi: " << iHi << " sum: " << sum);

        //------------------------------------------------------------------------------
        //..Set up the fit. Initial values
        double normalization = hEnergy->GetMaximum();
        const int iMax = hEnergy->GetMaximumBin();
        double peak = hEnergy->GetBinLowEdge(iMax);
        double effSigma = 0.5 * (fithigh - fitlow);
        double eta = 0.4; // typical value for good fits

        func->SetParameters(normalization, peak, effSigma, eta);
        func->SetParNames("normalization", "peak", "effSigma", "eta");

        //..Parameter ranges
        func->SetParLimits(1, fitlow, fithigh);
        func->SetParLimits(2, 0., 2.*effSigma);
        func->SetParLimits(3, -1, 3);

        //..If there is only one crystal, fix eta to a special value
        if (nCrysSumToFit == 1) {
          const double etaForOneCrystal = 0.95;
          func->SetParameter(3, etaForOneCrystal);
          func->SetParLimits(3, etaForOneCrystal, etaForOneCrystal);
        }

        //..Fit
        hEnergy->Fit(func, "LIEQ", "", fitlow, fithigh);

        //------------------------------------------------------------------------------
        //..Find the bias = sum of CalDigits minus sum of MC truth for this nCrys
        //  Bias is the mid point of the minimum range that contains 68.3% of events
        double bias = 0.;
        double biasSigma = 0.;
        if (nCrysSumToFit < nCrysBins) {
          TH1D* hBias = (TH1D*)biasSum->ProjectionY("hBias", nCrysSumToFit, nCrysSumToFit);
          fitFraction = 0.683;
          std::vector<int> jBins = eclNOptimalFitRange(hBias, fitFraction);
          const double lowEdge = hBias->GetBinLowEdge(jBins[0]);
          const double highEdge = hBias->GetBinLowEdge(jBins[1] + 1);
          bias = 0.5 * (lowEdge + highEdge);
          biasSigma = 0.5 * (highEdge - lowEdge);
        }

        //------------------------------------------------------------------------------
        //..Resolution is half the smallest range that contains 68.3% of events.
        //  Speed up the process by only looking among the range that contains 75%.
        fitFraction = 0.75;
        std::vector<int> iBins = eclNOptimalFitRange(hEnergy, fitFraction);
        const double resolution = eclNOptimalResolution(hEnergy, iBins[0], iBins[1]);

        //..Correct for the bias from background when calculating fractional resolution.
        //  Note that variable resolution = (E_hi - E_lo)/E_gen is dimensionless,
        //  variable peak = E_peak/E_gen is dimensionless, but variable bias has units GeV
        peak = func->GetParameter(1);
        const double fractionalResolution = resolution / (peak - bias / genEnergy);

        //..Record information if this is the best resolution (and is not the existing
        //  resolution case).
        if (fractionalResolution < bestFractionalResolution and nCrysSumToFit != nCrysBins) {
          bestFractionalResolution = fractionalResolution;
          nOpt = nCrysSumToFit;
          eSumOpt = hEnergy;
          nFitBins = 1 + iHi - iLo;
          biasForNOpt = bias;
          biasSigmaForNOpt = biasSigma;
        } else if (nCrysSumToFit == nCrysBins) {
          existingFractionalResolution = fractionalResolution;
        }

        //------------------------------------------------------------------------------
        //..Logic to decide what to do next
        if (nCrysSumToFit == initialnCrysSumToFit) {

          //..After testing the previous nOptimal, try one fewer if possible
          if (nCrysSumToFit > 1) {
            nCrysSumToFit--;
          } else {
            nCrysSumToFit++;
          }
        } else if (nCrysSumToFit < initialnCrysSumToFit) {

          //..Trying fewer crystals. If this is the best so far, try one
          //  fewer, if possible. Otherwise, try more crystals.
          if (nCrysSumToFit == nOpt and nCrysSumToFit > 1) {
            nCrysSumToFit--;
          } else if (initialnCrysSumToFit != nCrysMax) {
            nCrysSumToFit = initialnCrysSumToFit + 1;
          } else {
            nCrysSumToFit = nCrysBins;
          }
        } else if (nCrysSumToFit < nCrysBins) {  // nCrysSumToFit is always > initialnCrysSumToFit

          //..Trying more crystals. If this is the best, try one more, if
          //  possible. Otherwise, do the current reconstruction case (nCrysSumToFit = nCrysBins)
          if (nCrysSumToFit == nOpt and nCrysSumToFit < nCrysMax) {
            nCrysSumToFit++;
          } else {
            nCrysSumToFit = nCrysBins;
          }
        } else if (nCrysSumToFit == nCrysBins) {

          //..This is the current resolution case, so we are done
          nCrysSumToFit = 0;
        }

      } // while(nCrysSumToFit > 0)
      B2INFO(" ig: " << ig << " ie: " << ie << " initial nOpt: " << initialnCrysSumToFit << " final nOpt: " << nOpt);

      //-----------------------------------------------------------------------------------
      //..Store everything in diagnostic histograms

      //..Extract the function from the nOptimal histogram
      TF1* funcOpt = (TF1*)eSumOpt->GetFunction("eclNOptimalNovo");

      nOptimalPerGroup->SetBinContent(ig + 1, ie + 1, nOpt);

      changeInNOptimal->SetBinContent(ig + 1, ie + 1, nOpt - initialnCrysSumToFit);
      changeInNOptimal->SetBinError(ig + 1, ie + 1, 0.);

      biasPerGroup->SetBinContent(ig + 1, ie + 1, biasForNOpt);
      biasPerGroup->SetBinError(ig + 1, ie + 1, 0.);

      sigmaBiasPerGroup->SetBinContent(ig + 1, ie + 1, biasSigmaForNOpt);
      sigmaBiasPerGroup->SetBinError(ig + 1, ie + 1, 0.);

      binsInFit->SetBinContent(ig + 1, ie + 1, nFitBins);
      binsInFit->SetBinError(ig + 1, ie + 1, 0.);

      maxEntriesPerHist->SetBinContent(ig + 1, ie + 1, eSumOpt->GetMaximum());
      maxEntriesPerHist->SetBinError(ig + 1, ie + 1, 0.);

      const double peakOpt = funcOpt->GetParameter(1);
      const double peakOptUnc = funcOpt->GetParError(1);
      peakPerGroup->SetBinContent(ig + 1, ie + 1, peakOpt);
      peakPerGroup->SetBinError(ig + 1, ie + 1, peakOptUnc);

      const double genE = generatedEPerGroup->GetBinContent(ig + 1, ie + 1);
      const double peakCor = peakOpt - biasForNOpt / genE;
      fracContainedEPerGroup->SetBinContent(ig + 1, ie + 1, peakCor);
      fracContainedEPerGroup->SetBinError(ig + 1, ie + 1, peakOptUnc);

      effSigmaPerGroup->SetBinContent(ig + 1, ie + 1, funcOpt->GetParameter(2));
      effSigmaPerGroup->SetBinError(ig + 1, ie + 1, funcOpt->GetParError(2));

      etaPerGroup->SetBinContent(ig + 1, ie + 1, funcOpt->GetParameter(3));
      etaPerGroup->SetBinError(ig + 1, ie + 1, funcOpt->GetParError(3));

      fitProbPerGroup->SetBinContent(ig + 1, ie + 1, funcOpt->GetProb());
      fitProbPerGroup->SetBinError(ig + 1, ie + 1, 0.);

      resolutionPerGroup->SetBinContent(ig + 1, ie + 1, bestFractionalResolution);
      resolutionPerGroup->SetBinError(ig + 1, ie + 1, 0.);

      existingResolutionPerGroup->SetBinContent(ig + 1, ie + 1, existingFractionalResolution);
      existingResolutionPerGroup->SetBinError(ig + 1, ie + 1, 0.);

      //..Write out to disk
      histFile->cd();
      eSumOpt->Write();

    }
  }

  //-----------------------------------------------------------------------------------
  //..Write out fit summary histograms
  histFile->cd();
  changeInNOptimal->Write();
  biasPerGroup->Write();
  sigmaBiasPerGroup->Write();
  binsInFit->Write();
  maxEntriesPerHist->Write();
  peakPerGroup->Write();
  effSigmaPerGroup->Write();
  etaPerGroup->Write();
  fitProbPerGroup->Write();
  resolutionPerGroup->Write();
  existingResolutionPerGroup->Write();
  fracContainedEPerGroup->Write();

  B2INFO("Wrote fit summary histograms");

  //-----------------------------------------------------------------------------------
  //-----------------------------------------------------------------------------------
  //..Find bias and fraction contained energy in nOpt crystals for the energy bins
  //  adjacent to each of the group / energy points.

  //..Histograms to store results
  TH2F* fracContainedAdjacent[2];
  TH2F* biasAdjacent[2];
  const TString updown[2] = {"lower", "higher"};
  for (int ia = 0; ia < 2; ia++) {
    TString name = "fracContainedAdjacent_" + std::to_string(ia);
    TString title = "peak fraction of energy contained in nOpt crystals, " + updown[ia] + " E;group;energy point";
    fracContainedAdjacent[ia] = new TH2F(name, title, nCrystalGroups, 0., nCrystalGroups, nEnergies, 0., nEnergies);

    name = "biasAdjacent_" + std::to_string(ia);
    title = "bias (GeV) in nOpt crystals, " + updown[ia] + " E;group;energy point";
    biasAdjacent[ia] = new TH2F(name, title, nCrystalGroups, 0., nCrystalGroups, nEnergies, 0., nEnergies);
  }

  //-----------------------------------------------------------------------------------
  //..Loop over all groups and energy points
  for (int ig = 0; ig < nCrystalGroups; ig++) {
    for (int ie = 0; ie < nEnergies; ie++) {

      //..nOpt for this point
      const int nOpt = nOptimalPerGroup->GetBinContent(ig + 1, ie + 1);

      //-----------------------------------------------------------------------------------
      //..Two adjacent energy points
      for (int ia = 0; ia < 2; ia++) {
        const int ieAdj = ie + 2 * ia - 1; // ie +/- 1
        double eFracPeakAdj = fracContainedEPerGroup->GetBinContent(ig + 1, ie + 1);
        double biasAdj = biasPerGroup->GetBinContent(ig + 1, ie + 1);

        //..Need this to be a valid energy point to do anything more
        if (ieAdj >= 0 and ieAdj < nEnergies) {

          //----------------------------------------------------------------------------
          //..Find the bias
          std::string biasName = "biasSum_" + std::to_string(ig) + "_" + std::to_string(ieAdj);
          auto biasSum = getObjectPtr<TH2F>(biasName);
          TH1D* hBias = (TH1D*)biasSum->ProjectionY("hBias", nOpt, nOpt);

          //..Bias is the mid-point of the range containing 68.3% of events
          double fitFraction = 0.683;
          std::vector<int> jBins = eclNOptimalFitRange(hBias, fitFraction);
          const double lowEdge = hBias->GetBinLowEdge(jBins[0]);
          const double highEdge = hBias->GetBinLowEdge(jBins[1] + 1);
          biasAdj = 0.5 * (lowEdge + highEdge);

          //----------------------------------------------------------------------------
          //..Get the eSum distribution to be fit
          std::string name = "eSum_" + std::to_string(ig) + "_" + std::to_string(ieAdj);
          auto eSum = getObjectPtr<TH2F>(name);
          TH1D* hEnergy = (TH1D*)eSum->ProjectionY("hEnergy", nOpt, nOpt);
          TString newName = name + "_" + std::to_string(nOpt);
          hEnergy->SetName(newName);

          B2INFO("fit adj ig = " << ig << " ie = " << ie << " ia = " << ia << " " << newName << " " << " entries = " << hEnergy->GetEntries()
                 << " GetMaxiumum = " << hEnergy->GetMaximum());

          //..Rebin if the maximum bin has too few entries
          const double minPeakValue = 300.;
          while (hEnergy->GetMaximum() < minPeakValue) {hEnergy->Rebin(2);}

          //..Find 50% range for Novo fit. Rebin if there are too many bins
          fitFraction = 0.5;
          int iLo = 1;
          int iHi = hEnergy->GetNbinsX();
          const int maxBinsToFit = 59;
          while ((iHi - iLo) > maxBinsToFit) {
            std::vector<int> iBins = eclNOptimalFitRange(hEnergy, fitFraction);
            iLo = iBins[0];
            iHi = iBins[1];
            if ((iHi - iLo) > maxBinsToFit) {hEnergy->Rebin(2);}
          }

          //..Fit parameters
          const double fitlow = hEnergy->GetBinLowEdge(iLo);
          const double fithigh = hEnergy->GetBinLowEdge(iHi + 1);
          double normalization = hEnergy->GetMaximum();
          const int iMax = hEnergy->GetMaximumBin();
          double peak = hEnergy->GetBinLowEdge(iMax);
          double effSigma = 0.5 * (fithigh - fitlow);
          double eta = 0.4; // typical value for good fits

          func->SetParameters(normalization, peak, effSigma, eta);
          func->SetParNames("normalization", "peak", "effSigma", "eta");

          //..Parameter ranges
          func->SetParLimits(1, fitlow, fithigh);
          func->SetParLimits(2, 0., 2.*effSigma);
          func->SetParLimits(3, -1, 3);

          //..If there is only one crystal, fix eta to a special value
          if (nOpt == 1) {
            const double etaForOneCrystal = 0.95;
            func->SetParameter(3, etaForOneCrystal);
            func->SetParLimits(3, etaForOneCrystal, etaForOneCrystal);
          }

          //..Fit
          hEnergy->Fit(func, "LIEQ", "", fitlow, fithigh);

          //..Fraction of contained energy = peak of fit corrected for bias
          const double peakAdj = func->GetParameter(1);
          const double genE = generatedEPerGroup->GetBinContent(ig + 1, ieAdj + 1);
          eFracPeakAdj = peakAdj - biasAdj / genE;
        }

        //..Store
        fracContainedAdjacent[ia]->SetBinContent(ig + 1, ie + 1, eFracPeakAdj);
        biasAdjacent[ia]->SetBinContent(ig + 1, ie + 1, biasAdj);
      } // loop over adjacent energy points

      B2INFO(" ig " << ig << " ie " << ie << " eFrac (3): "
             << fracContainedAdjacent[0]->GetBinContent(ig + 1, ie + 1) << " "
             << fracContainedEPerGroup->GetBinContent(ig + 1, ie + 1) << " "
             << fracContainedAdjacent[1]->GetBinContent(ig + 1, ie + 1)
             << " biases (3): "
             << biasAdjacent[0]->GetBinContent(ig + 1, ie + 1) << " "
             << biasPerGroup->GetBinContent(ig + 1, ie + 1) << " "
             << biasAdjacent[1]->GetBinContent(ig + 1, ie + 1)
            );
    } // loop over ie
  } // loop over ig

  //-----------------------------------------------------------------------------------
  //..Write out histograms with results for adjacent energy points
  histFile->cd();
  fracContainedAdjacent[0]->Write();
  fracContainedAdjacent[1]->Write();
  biasAdjacent[0]->Write();
  biasAdjacent[1]->Write();

  B2INFO("Wrote adjacent energy point summary histograms");


  //-----------------------------------------------------------------------------------
  //-----------------------------------------------------------------------------------
  //..Prepare the payload contents
  //..Upper energy boundaries are the mid-point of the log energies
  float boundaryE[nLeakReg][nEnergies];
  for (int ireg = 0; ireg < nLeakReg; ireg++) {
    B2INFO("Generated energies and boundaries for region = " << ireg);
    for (int ie = 0; ie < nEnergies - 1; ie++) {
      double logE0 = log(generatedE[ireg][ie]);
      double logE1 = log(generatedE[ireg][ie + 1]);
      double logBoundary = 0.5 * (logE0 + logE1);
      boundaryE[ireg][ie] = exp(logBoundary);
      B2INFO("  " << ie << " " << generatedE[ireg][ie] <<  " " << boundaryE[ireg][ie] << " GeV");
    }

    //..Last boundary is an arbitrarily large number
    boundaryE[ireg][nEnergies - 1] = 9999.;
    B2INFO("  " << nEnergies - 1 << " " << generatedE[ireg][nEnergies - 1] <<  " " << boundaryE[ireg][nEnergies - 1] << " GeV");
  }

  //..Group number of each cellID
  std::vector<int> groupNumber;
  for (int cellID = 1; cellID <= ECLElementNumbers::c_NCrystals; cellID++) {
    const int iGroup = (int)(0.5 + groupNumberOfEachCellID->GetBinContent(cellID));
    groupNumber.push_back(iGroup);
  }

  //..Copy results for each group into the payload histogram
  TH2F nOptimal2D("nOptimal2D", "Optimal nCrys, energy bin vs group number", nCrystalGroups, 0, nCrystalGroups, nEnergies, 0.,
                  nEnergies);
  for (int ig = 0; ig < nCrystalGroups; ig++) {
    for (int ie = 0; ie < nEnergies; ie++) {
      double nOpt = nOptimalPerGroup->GetBinContent(ig + 1, ie + 1);
      nOptimal2D.SetBinContent(ig + 1, ie + 1, nOpt);
    }
  }

  //..2D histograms for peak and bias corrections.
  //  Note that these have 3 x bins per group = nominal energy plus adjacent energies.
  TH2F peakFracEnergy("peakFracEnergy", "peak contained energy over generated E, energy bin vs group number", 3 * nCrystalGroups, 0,
                      nCrystalGroups, nEnergies, 0., nEnergies);
  TH2F bias("bias", "median bias (GeV), energy bin vs group number", 3 * nCrystalGroups, 0, nCrystalGroups, nEnergies, 0., nEnergies);
  TH2F logPeakEnergy("logPeakEnergy", "log peak Energy (GeV), energy bin vs group number", 3 * nCrystalGroups, 0, nCrystalGroups,
                     nEnergies, 0., nEnergies);
  for (int ig = 0; ig < nCrystalGroups; ig++) {
    for (int ie = 0; ie < nEnergies; ie++) {
      const int iy = ie + 1;
      const int ix = 1 + 3 * ig; // three bins per group in payload histograms

      //..Peak fractional energy and log peak contained energy (GeV)

      //..Nominal ig and and ie
      double peakAdj =  fracContainedEPerGroup->GetBinContent(ig + 1, ie + 1);
      const double genE = generatedEPerGroup->GetBinContent(ig + 1, ie + 1);
      double logE = log(genE * peakAdj);

      peakFracEnergy.SetBinContent(ix, iy, peakAdj);
      logPeakEnergy.SetBinContent(ix, iy, logE);

      //..Lower E adjacent point
      double peakAdj0 = peakAdj;
      double logE0 = logE;
      if (ie > 1) {
        peakAdj0 = fracContainedAdjacent[0]->GetBinContent(ig + 1, ie + 1);
        const double genE0 = generatedEPerGroup->GetBinContent(ig + 1, ie);
        logE0 = log(genE0 * peakAdj0);
      }

      peakFracEnergy.SetBinContent(ix + 1, iy, peakAdj0);
      logPeakEnergy.SetBinContent(ix + 1, iy, logE0);

      //..Higher E adjacent point
      double peakAdj1 = peakAdj;
      double logE1 = logE;
      if (ie < nEnergies - 1) {
        peakAdj1 = fracContainedAdjacent[1]->GetBinContent(ig + 1, ie + 1);
        const double genE1 = generatedEPerGroup->GetBinContent(ig + 1, ie + 2);
        logE1 = log(genE1 * peakAdj1);
      }

      peakFracEnergy.SetBinContent(ix + 2, iy, peakAdj1);
      logPeakEnergy.SetBinContent(ix + 2, iy, logE1);

      //..Bias
      double medianBias = biasPerGroup->GetBinContent(ig + 1, ie + 1);
      bias.SetBinContent(ix, iy, medianBias);
      medianBias = biasAdjacent[0]->GetBinContent(ig + 1, ie + 1);
      bias.SetBinContent(ix + 1, iy, medianBias);
      medianBias = biasAdjacent[1]->GetBinContent(ig + 1, ie + 1);
      bias.SetBinContent(ix + 2, iy, medianBias);
    }
  }

  //..Store the histograms
  histFile->cd();
  nOptimal2D.Write();
  peakFracEnergy.Write();
  logPeakEnergy.Write();
  bias.Write();
  histFile->Close();

  //-----------------------------------------------------------------------------------
  //..The payload itself
  ECLnOptimal* optimalNCrys = new ECLnOptimal();

  //..Energy boundaries
  for (int ireg = 0; ireg < nLeakReg; ireg++) {
    std::vector<float> eUpperBoundaries;
    for (int ie = 0; ie < nEnergies; ie++) {eUpperBoundaries.push_back(boundaryE[ireg][ie]);}
    if (ireg == 0) {
      optimalNCrys->setUpperBoundariesFwd(eUpperBoundaries);
    } else if (ireg == 1) {
      optimalNCrys->setUpperBoundariesBrl(eUpperBoundaries);
    } else {
      optimalNCrys->setUpperBoundariesBwd(eUpperBoundaries);
    }
  }

  //..Group number of each cellID
  optimalNCrys->setGroupNumber(groupNumber);

  //..nOptimal histogram
  optimalNCrys->setNOptimal(nOptimal2D);

  //..Peak and bias correction histograms
  optimalNCrys->setPeakFracEnergy(peakFracEnergy);
  optimalNCrys->setBias(bias);
  optimalNCrys->setLogPeakEnergy(logPeakEnergy);

  //..Save the calibration
  saveCalibration(optimalNCrys, "ECLnOptimal");
  B2RESULT("eclNOptimalAlgorithm: successfully stored payload ECLnOptimal");

  //..Done
  return c_OK;
}


