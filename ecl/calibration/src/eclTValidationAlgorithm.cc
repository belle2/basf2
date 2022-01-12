/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <ecl/calibration/eclTValidationAlgorithm.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dbobjects/ECLReferenceCrystalPerCrateCalib.h>
#include <ecl/digitization/EclConfiguration.h>
#include <ecl/utility/ECLChannelMapper.h>
#include <ecl/geometry/ECLGeometryPar.h>

#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>

#include <TH2F.h>
#include <TFile.h>
#include <TF1.h>
#include <TROOT.h>
#include <TGaxis.h>
#include <TLegend.h>
#include <TGraphErrors.h>
#include <TGraphAsymmErrors.h>
#include <TMultiGraph.h>
#include "TString.h"

using namespace std;
using namespace Belle2;
using namespace ECL;


/* By default assume the timing validation collector used a hadronic event selection
   but a bhabha event selection also exists and uses the same algorithm to
   analyse the results:
   The collector name should be set in the steering script.*/
eclTValidationAlgorithm::eclTValidationAlgorithm():
  // Parameters
  CalibrationAlgorithm("eclHadronTimeCalibrationValidationCollector"),
  cellIDLo(1),
  cellIDHi(8736),
  readPrevCrysPayload(false),
  meanCleanRebinFactor(1),
  meanCleanCutMinFactor(0),
  clusterTimesFractionWindow_maxtime(8),
  debugFilenameBase("eclTValidationAlgorithm")
{
  setDescription("Fit gaussian function to the cluster times to validate results.");
}




/* By default assume the timing validation collector used a hadronic event selection
   but a bhabha event selection also exists and uses the same algorithm to
   analyse the results:
   The collector name should be set in the steering script.*/
eclTValidationAlgorithm::eclTValidationAlgorithm(string physicsProcessCollectorName):
  // Parameters
  CalibrationAlgorithm(physicsProcessCollectorName.c_str()),
  cellIDLo(1),
  cellIDHi(8736),
  readPrevCrysPayload(false),
  meanCleanRebinFactor(1),
  meanCleanCutMinFactor(0),
  clusterTimesFractionWindow_maxtime(8),
  debugFilenameBase("eclTValidationAlgorithm")
{
  setDescription("Fit gaussian function to the cluster times to validate results.");
}




CalibrationAlgorithm::EResult eclTValidationAlgorithm::calibrate()
{
  /** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();


  /** Write out job parameters */
  B2INFO("eclTValidationAlgorithm parameters:");
  B2INFO("cellIDLo = " << cellIDLo);
  B2INFO("cellIDHi = " << cellIDHi);
  B2INFO("readPrevCrysPayload = " << readPrevCrysPayload);
  B2INFO("meanCleanRebinFactor = " << meanCleanRebinFactor);
  B2INFO("meanCleanCutMinFactor = " << meanCleanCutMinFactor);
  B2INFO("clusterTimesFractionWindow_maxtime = " << clusterTimesFractionWindow_maxtime);


  /* Histogram with the data collected by eclTimeCalibrationValidationCollector*/
  auto clusterTime = getObjectPtr<TH1F>("clusterTime");
  auto clusterTime_cid = getObjectPtr<TH2F>("clusterTime_cid");
  auto clusterTime_run = getObjectPtr<TH2F>("clusterTime_run");
  auto clusterTimeClusterE = getObjectPtr<TH2F>("clusterTimeClusterE");
  auto dt99_clusterE = getObjectPtr<TH2F>("dt99_clusterE");
  auto eventT0 = getObjectPtr<TH1F>("eventT0");
  auto clusterTimeE0E1diff = getObjectPtr<TH1F>("clusterTimeE0E1diff");

  // Collect other plots just for reference - combines all the runs for these plots.
  auto cutflow = getObjectPtr<TH1F>("cutflow");

  vector <int> binProjectionLeft_Time_vs_E_runDep ;
  vector <int> binProjectionRight_Time_vs_E_runDep ;

  for (int binCounter = 1; binCounter <= clusterTimeClusterE->GetNbinsX(); binCounter++) {
    binProjectionLeft_Time_vs_E_runDep.push_back(binCounter);
    binProjectionRight_Time_vs_E_runDep.push_back(binCounter);
  }

  if (!clusterTime_cid) return c_Failure;

  /**-----------------------------------------------------------------------------------------------*/

  TFile* histfile = 0;

  /* 1/(4fRF) = 0.4913 ns/clock tick, where fRF is the accelerator RF frequency.
     Same for all crystals. */
  const double TICKS_TO_NS = 1.0 / (4.0 * EclConfiguration::m_rf) * 1e3;

  // Vector of time offsets to track how far from nominal the cluster times are.
  vector<float> t_offsets(8736, 0.0);
  vector<float> t_offsets_unc(8736, 0.0);
  vector<long> numClusterPerCrys(8736, 0);
  vector<bool> crysHasGoodFitandStats(8736, false);
  vector<bool> crysHasGoodFit(8736, false);
  int numCrysWithNonZeroEntries = 0 ;
  int numCrysWithGoodFit = 0 ;

  int minNumEntries = 40;

  double mean;
  double sigma;


  bool minRunNumBool = false;
  bool maxRunNumBool = false;
  int minRunNum = -1;
  int maxRunNum = -1;
  int minExpNum = -1;
  int maxExpNum = -1;
  for (auto expRun : getRunList()) {
    int expNumber = expRun.first;
    int runNumber = expRun.second;
    if (!minRunNumBool) {
      minExpNum = expNumber;
      minRunNum = runNumber;
      minRunNumBool = true;
    }
    if (!maxRunNumBool) {
      maxExpNum = expNumber;
      maxRunNum = runNumber;
      maxRunNumBool = true;
    }
    if (((minRunNum > runNumber)   && (minExpNum >= expNumber))  ||
        (minExpNum > expNumber)) {
      minExpNum = expNumber;
      minRunNum = runNumber;
    }
    if (((maxRunNum < runNumber)   && (maxExpNum <= expNumber))  ||
        (maxExpNum < expNumber))

    {
      maxExpNum = expNumber;
      maxRunNum = runNumber;
    }
  }

  B2INFO("debugFilenameBase = " << debugFilenameBase);
  string runNumsString = string("_") + to_string(minExpNum) + "_" + to_string(minRunNum) + string("-") +
                         to_string(maxExpNum) + "_" + to_string(maxRunNum);
  string debugFilename = debugFilenameBase + runNumsString + string(".root");


  // Need to load information about the event/run/experiment to get the right database information
  // Will be used for:
  // * ECLChannelMapper (to map crystal to crates)
  // * crystal payload updating for iterating crystal and crate fits
  int eventNumberForCrates = 1;


  //-------------------------------------------------------------------
  /* Uploading older payloads for the current set of runs */

  StoreObjPtr<EventMetaData> evtPtr;
  // simulate the initialize() phase where we can register objects in the DataStore
  DataStore::Instance().setInitializeActive(true);
  evtPtr.registerInDataStore();
  DataStore::Instance().setInitializeActive(false);
  // now construct the event metadata
  evtPtr.construct(eventNumberForCrates, minRunNum, minExpNum);
  // and update the database contents
  DBStore& dbstore = DBStore::Instance();
  dbstore.update();
  // this is only needed it the payload might be intra-run dependent,
  // that is if it might change during one run as well
  dbstore.updateEvent();


  B2INFO("Uploading payload for exp " << minExpNum << ", run " << minRunNum << ", event " << eventNumberForCrates);
  updateDBObjPtrs(eventNumberForCrates, minRunNum, minExpNum);
  unique_ptr<ECLChannelMapper> crystalMapper(new ECL::ECLChannelMapper());
  crystalMapper->initFromDB();

  //------------------------------------------------------------------------
  //..Read payloads from database
  DBObjPtr<Belle2::ECLCrystalCalib> crystalTimeObject("ECLCrystalTimeOffset");
  B2INFO("Dumping payload");

  //..Get vectors of values from the payloads
  std::vector<float> currentValuesCrys = crystalTimeObject->getCalibVector();
  std::vector<float> currentUncCrys = crystalTimeObject->getCalibUncVector();

  //..Print out a few values for quality control
  B2INFO("Values read from database.  Write out for their values for comparison against those from tcol");
  for (int ic = 0; ic < 8736; ic += 500) {
    B2INFO("ts: cellID " << ic + 1 << " " << currentValuesCrys[ic] << " +/- " << currentUncCrys[ic]);
  }


  //..Read in the previous crystal payload values
  DBObjPtr<Belle2::ECLCrystalCalib> customPrevCrystalTimeObject("ECLCrystalTimeOffsetPreviousValues");
  vector<float> prevValuesCrys(8736);
  if (readPrevCrysPayload) {
    //..Get vectors of values from the payloads
    prevValuesCrys = customPrevCrystalTimeObject->getCalibVector();

    //..Print out a few values for quality control
    B2INFO("Previous values read from database.  Write out for their values for comparison against those from tcol");
    for (int ic = 0; ic < 8736; ic += 500) {
      B2INFO("ts custom previous payload: cellID " << ic + 1 << " " << prevValuesCrys[ic]);
    }
  }


  //------------------------------------------------------------------------
  //..Start looking at timing information

  B2INFO("Debug output rootfile: " << debugFilename);
  histfile = new TFile(debugFilename.c_str(), "recreate");


  clusterTime         ->Write();
  clusterTime_cid     ->Write();
  clusterTime_run     ->Write();
  clusterTimeClusterE ->Write();
  dt99_clusterE       ->Write();
  eventT0             ->Write();
  clusterTimeE0E1diff ->Write();

  cutflow->Write();


  double hist_tmin = clusterTime->GetXaxis()->GetXmin();
  double hist_tmax = clusterTime->GetXaxis()->GetXmax();
  int hist_nTbins  = clusterTime->GetNbinsX();

  B2INFO("hist_tmin = " << hist_tmin);
  B2INFO("hist_tmax = " << hist_tmax);
  B2INFO("hist_nTbins = " << hist_nTbins);

  double time_fit_min = hist_tmax;   // Set min value to largest possible value so that it gets reset
  double time_fit_max = hist_tmin;   // Set max value to smallest possible value so that it gets reset


  // define histogram for keeping track of the peak of the cluster times per crystal
  auto peakClusterTime_cid = new TH1F("peakClusterTime_cid", ";cell id;Peak cluster time [ns]", 8736, 1, 8736 + 1);
  auto peakClusterTimes = new TH1F("peakClusterTimes",
                                   "-For crystals with at least one hit-;Peak cluster time [ns];Number of crystals",
                                   hist_nTbins, hist_tmin, hist_tmax);
  auto peakClusterTimesGoodFit = new TH1F("peakClusterTimesGoodFit",
                                          "-For crystals with a good fit to distribution of hits-;Peak cluster time [ns];Number of crystals",
                                          hist_nTbins, hist_tmin, hist_tmax);

  auto peakClusterTimesGoodFit__cid = new TH1F("peakClusterTimesGoodFit__cid",
                                               "-For crystals with a good fit to distribution of hits-;cell id (only crystals with good fit);Peak cluster time [ns]",
                                               8736, 1, 8736 + 1);


  // define histograms to keep track of the difference in the new crystal times vs the old ones
  auto tsNew_MINUS_tsCustomPrev__cid = new TH1F("TsNew_MINUS_TsCustomPrev__cid",
                                                ";cell id; ts(new|merged) - ts(old = 'pre-calib'|merged)  [ns]",
                                                8736, 1, 8736 + 1);

  auto tsNew_MINUS_tsCustomPrev = new TH1F("TsNew_MINUS_TsCustomPrev",
                                           ";ts(new | merged) - ts(old = 'pre-calib' | merged)  [ns];Number of crystals",
                                           285, -69.5801, 69.5801);



  // Histogram to keep track of the fraction of cluster times within a window.
  double timeWindow_maxTime = clusterTimesFractionWindow_maxtime;
  B2INFO("timeWindow_maxTime = " << timeWindow_maxTime);
  int binyLeft = clusterTime_cid->GetYaxis()->FindBin(-timeWindow_maxTime);
  int binyRight = clusterTime_cid->GetYaxis()->FindBin(timeWindow_maxTime);
  double windowLowTimeFromBin = clusterTime_cid->GetYaxis()->GetBinLowEdge(binyLeft);
  double windowHighTimeFromBin = clusterTime_cid->GetYaxis()->GetBinLowEdge(binyRight + 1);
  std::string s_lowTime = std::to_string(windowLowTimeFromBin);
  std::string s_highTime = std::to_string(windowHighTimeFromBin);
  TString fracWindowTitle = "Fraction of cluster times in window [" + s_lowTime + ", " + s_highTime +
                            "] ns;cell id;Fraction of cluster times in window";
  B2INFO("fracWindowTitle = " << fracWindowTitle);
  TString fracWindowInGoodECLRingsTitle = "Fraction of cluster times in window [" + s_lowTime + ", " + s_highTime +
                                          "] ns and in good ECL theta rings;cell id;Fraction cluster times in window + good ECL rings";
  B2INFO("fracWindowInGoodECLRingsTitle = " << fracWindowInGoodECLRingsTitle);
  B2INFO("Good ECL rings skip gaps in the acceptance, and includes ECL theta IDs: 3-10, 15-39, 44-56, 61-66.");

  TString fracWindowHistTitle = "Fraction of cluster times in window [" + s_lowTime + ", " + s_highTime +
                                "] ns;Fraction of cluster times in window;Number of crystals";

  auto clusterTimeNumberInWindow__cid = new TH1F("clusterTimeNumberInWindow__cid", fracWindowTitle, 8736, 1, 8736 + 1);
  auto clusterTimeNumberInWindowInGoodECLRings__cid = new TH1F("clusterTimeNumberInWindowInGoodECLRings__cid", fracWindowTitle, 8736,
      1, 8736 + 1);
  auto clusterTimeNumber__cid = new TH1F("clusterTimeNumber_cid", fracWindowTitle, 8736, 1, 8736 + 1);
  auto clusterTimeFractionInWindow = new TH1F("clusterTimeFractionInWindow", fracWindowHistTitle, 110, 0.0, 1.1);

  clusterTimeNumberInWindow__cid->Sumw2();
  clusterTimeNumberInWindowInGoodECLRings__cid->Sumw2();
  clusterTimeNumber__cid->Sumw2();



  /* CRYSTAL BY CRYSTAL VALIDATION */

  ECLGeometryPar* eclgeo = ECLGeometryPar::Instance();

  // Loop over all the crystals for doing the crystal calibation
  for (int crys_id = cellIDLo; crys_id <= cellIDHi; crys_id++) {
    double clusterTime_mean = 0;
    double clusterTime_mean_unc = 0;

    B2INFO("Crystal cell id = " << crys_id);

    eclgeo->Mapping(crys_id - 1);
    int thetaID = eclgeo->GetThetaID();


    /* Determining which bins to mask out for mean calculation
    */

    TH1D* h_time = clusterTime_cid->ProjectionY((std::string("h_time_psi__") + std::to_string(crys_id)).c_str(),
                                                crys_id, crys_id);
    TH1D* h_timeMask = (TH1D*)h_time->Clone();
    TH1D* h_timeMasked = (TH1D*)h_time->Clone((std::string("h_time_psi_masked__") + std::to_string(crys_id)).c_str());
    TH1D* h_timeRebin = (TH1D*)h_time->Clone();

    // Do rebinning and cleaning of some bins but only if the user selection values call for it since it slows the code down
    if (meanCleanRebinFactor != 1 || meanCleanCutMinFactor != 1) {

      h_timeRebin->Rebin(meanCleanRebinFactor);

      h_timeMask->Scale(0.0);   // set all bins to being masked off

      time_fit_min = hist_tmax;   // Set min value to largest possible value so that it gets reset
      time_fit_max = hist_tmin;   // Set max value to smallest possible value so that it gets reset

      // Find value of bin with max value
      double histRebin_max = h_timeRebin->GetMaximum();

      bool maskedOutNonZeroBin = false;
      // Loop over all bins to find those with content less than a certain threshold.  Mask the non-rebinned histogram for the corresponding bins
      for (int bin = 1; bin <= h_timeRebin->GetNbinsX(); bin++) {
        for (int rebinCounter = 1; rebinCounter <= meanCleanRebinFactor; rebinCounter++) {
          int nonRebinnedBinNumber = (bin - 1) * meanCleanRebinFactor + rebinCounter;
          if (nonRebinnedBinNumber < h_time->GetNbinsX()) {
            if (h_timeRebin->GetBinContent(bin) >= histRebin_max * meanCleanCutMinFactor) {
              h_timeMask->SetBinContent(nonRebinnedBinNumber, 1);

              // Save the lower and upper edges of the rebin histogram time range for fitting purposes
              double x_lower = h_timeRebin->GetXaxis()->GetBinLowEdge(bin);
              double x_upper = h_timeRebin->GetXaxis()->GetBinUpEdge(bin);
              if (x_lower < time_fit_min) {
                time_fit_min = x_lower;
              }
              if (x_upper > time_fit_max) {
                time_fit_max = x_upper;
              }

            } else {
              if (h_time->GetBinContent(nonRebinnedBinNumber) > 0) {
                B2DEBUG(22, "Setting bin " << nonRebinnedBinNumber << " from " << h_timeMasked->GetBinContent(nonRebinnedBinNumber) << " to 0");
                maskedOutNonZeroBin = true;
              }
              h_timeMasked->SetBinContent(nonRebinnedBinNumber, 0);
            }
          }
        }
      }
      B2INFO("Bins with non-zero values have been masked out: " << maskedOutNonZeroBin);
      h_timeMasked->ResetStats();
      h_timeMask->ResetStats();

    }

    // Calculate mean from masked histogram
    double default_meanMasked = h_timeMasked->GetMean();
    //double default_meanMasked_unc = h_timeMasked->GetMeanError();
    B2INFO("default_meanMasked = " << default_meanMasked);


    // Get the overall mean and standard deviation of the distribution within the plot.  This doesn't require a fit.
    double default_mean = h_time->GetMean();
    double default_mean_unc = h_time->GetMeanError();
    double default_sigma = h_time->GetStdDev();

    B2INFO("Fitting crystal between " << time_fit_min << " and " << time_fit_max);

    // gaus(0) is a substitute for [0]*exp(-0.5*((x-[1])/[2])**2)
    TF1* gaus = new TF1("func", "gaus(0)", time_fit_min, time_fit_max);
    gaus->SetParNames("numCrystalHitsNormalization", "mean", "sigma");
    /*
       gaus->ReleaseParameter(0);  // number of crystals
       gaus->ReleaseParameter(1);  // mean
       gaus->ReleaseParameter(2);  // standard deviation
    */

    double hist_max = h_time->GetMaximum();

    //=== Estimate initial value of sigma as std dev.
    double stddev = h_time->GetStdDev();
    sigma = stddev;
    mean = default_mean;

    //=== Setting parameters for initial iteration
    gaus->SetParameter(0, hist_max / 2.);
    gaus->SetParameter(1, mean);
    gaus->SetParameter(2, sigma);
    // L -- Use log likelihood method
    // I -- Use integral of function in bin instead of value at bin center  // not using
    // R -- Use the range specified in the function range
    // B -- Fix one or more parameters with predefined function   // not using
    // Q -- Quiet mode

    h_timeMasked->Fit(gaus, "LQR");  // L for likelihood, R for x-range, Q for fit quiet mode

    double fit_mean     = gaus->GetParameter(1);
    double fit_mean_unc = gaus->GetParError(1);
    double fit_sigma    = gaus->GetParameter(2);

    double meanDiff =  fit_mean - default_mean;
    double meanUncDiff = fit_mean_unc - default_mean_unc;
    double sigmaDiff = fit_sigma - default_sigma;

    bool good_fit = false;

    if ((fabs(meanDiff) > 10)      ||
        (fabs(meanUncDiff) > 10)   ||
        (fabs(sigmaDiff) > 10)     ||
        (fit_mean_unc > 3)         ||
        (fit_sigma < 0.1)          ||
        (fit_mean < time_fit_min)  ||
        (fit_mean > time_fit_max)) {
      B2INFO("Crystal cell id = " << crys_id);
      B2INFO("fit mean, default mean = " << fit_mean << ", " << default_mean);
      B2INFO("fit mean unc, default mean unc = " << fit_mean_unc << ", " << default_mean_unc);
      B2INFO("fit sigma, default sigma = " << fit_sigma << ", " << default_sigma);

      B2INFO("crystal fit mean - hist mean = " << meanDiff);
      B2INFO("fit mean unc. - hist mean unc. = " << meanUncDiff);
      B2INFO("fit sigma - hist sigma = " << sigmaDiff);

      B2INFO("fit_mean = " << fit_mean);
      B2INFO("time_fit_min = " << time_fit_min);
      B2INFO("time_fit_max = " << time_fit_max);

      if (fabs(meanDiff) > 10)       B2INFO("fit mean diff too large");
      if (fabs(meanUncDiff) > 10)    B2INFO("fit mean unc diff too large");
      if (fabs(sigmaDiff) > 10)      B2INFO("fit mean sigma diff too large");
      if (fit_mean_unc > 3)          B2INFO("fit mean unc too large");
      if (fit_sigma < 0.1)           B2INFO("fit sigma too small");

    } else {
      good_fit = true;
      numCrysWithGoodFit++;
      crysHasGoodFit[crys_id - 1] = true ;
    }


    int numEntries = h_time->GetEntries();
    // If number of entries in histogram is greater than X then use the statistical information from the data otherwise leave crystal uncalibrated.  Histograms are still shown though.
    //  ALSO require the that fits are good.
    if ((numEntries >= minNumEntries)  &&  good_fit) {
      clusterTime_mean = fit_mean;
      clusterTime_mean_unc = fit_mean_unc;
      crysHasGoodFitandStats[crys_id - 1] = true ;
    } else {
      clusterTime_mean = default_mean;
      clusterTime_mean_unc = default_mean_unc;
    }

    if (numEntries < minNumEntries)   B2INFO("Number of entries less than minimum");
    if (numEntries == 0)   B2INFO("Number of entries == 0");


    t_offsets[crys_id - 1] = clusterTime_mean ;
    t_offsets_unc[crys_id - 1] = clusterTime_mean_unc ;
    numClusterPerCrys[crys_id - 1] = numEntries ;

    histfile->WriteTObject(h_time, (std::string("h_time_psi") + std::to_string(crys_id)).c_str());
    histfile->WriteTObject(h_timeMasked, (std::string("h_time_psi_masked") + std::to_string(crys_id)).c_str());

    // Set this for each crystal even if there are zero entries
    peakClusterTime_cid->SetBinContent(crys_id, t_offsets[crys_id - 1]);
    peakClusterTime_cid->SetBinError(crys_id, t_offsets_unc[crys_id - 1]);

    /* Store mean cluster time info in a separate histogram but only if there is at
       least one entry for that crystal.  */
    if (numEntries > 0) {
      peakClusterTimes->Fill(t_offsets[crys_id - 1]);
      numCrysWithNonZeroEntries++ ;
    }
    if ((numEntries >= minNumEntries)  &&  good_fit) {
      peakClusterTimesGoodFit->Fill(t_offsets[crys_id - 1]);
      peakClusterTimesGoodFit__cid->SetBinContent(crys_id, t_offsets[crys_id - 1]);
      peakClusterTimesGoodFit__cid->SetBinError(crys_id, t_offsets_unc[crys_id - 1]);
    }


    // Find the fraction of cluster times within +-X ns and fill histograms
    double numClusterTimesWithinWindowFraction = h_time->Integral(binyLeft, binyRight) ;
    double clusterTimesWithinWindowFraction = numClusterTimesWithinWindowFraction;
    if (numEntries > 0) {
      clusterTimesWithinWindowFraction /= numEntries;
    } else {
      clusterTimesWithinWindowFraction = -0.1;
    }

    B2INFO("Crystal cell id = " << crys_id << ", theta id = " <<
           thetaID << ", clusterTimesWithinWindowFraction = " <<
           numClusterTimesWithinWindowFraction << " / " << numEntries << " = " <<
           clusterTimesWithinWindowFraction);

    clusterTimeFractionInWindow->Fill(clusterTimesWithinWindowFraction);
    clusterTimeNumberInWindow__cid->SetBinContent(crys_id, numClusterTimesWithinWindowFraction);
    clusterTimeNumber__cid->SetBinContent(crys_id, numEntries);

    if ((thetaID >= 3   &&  thetaID <= 10)    ||
        (thetaID >= 15  &&  thetaID <= 39)    ||
        (thetaID >= 44  &&  thetaID <= 56)    ||
        (thetaID >= 61  &&  thetaID <= 66)) {
      clusterTimeNumberInWindowInGoodECLRings__cid->SetBinContent(crys_id, numClusterTimesWithinWindowFraction);
    }


    delete gaus;
  }

  // Find the fraction of cluster times within +-X ns and fill histogram
  auto g_clusterTimeFractionInWindow__cid = new TGraphAsymmErrors(clusterTimeNumberInWindow__cid, clusterTimeNumber__cid, "w");
  auto g_clusterTimeFractionInWindowInGoodECLRings__cid = new TGraphAsymmErrors(clusterTimeNumberInWindowInGoodECLRings__cid,
      clusterTimeNumber__cid, "w");
  g_clusterTimeFractionInWindow__cid->SetTitle(fracWindowTitle);
  g_clusterTimeFractionInWindowInGoodECLRings__cid->SetTitle(fracWindowInGoodECLRingsTitle);


  peakClusterTime_cid->ResetStats();
  peakClusterTimesGoodFit__cid->ResetStats();

  histfile->WriteTObject(peakClusterTime_cid, "peakClusterTime_cid");
  histfile->WriteTObject(peakClusterTimes, "peakClusterTimes");
  histfile->WriteTObject(peakClusterTimesGoodFit__cid, "peakClusterTimesGoodFit__cid");
  histfile->WriteTObject(peakClusterTimesGoodFit, "peakClusterTimesGoodFit");
  histfile->WriteTObject(g_clusterTimeFractionInWindow__cid, "g_clusterTimeFractionInWindow__cid");
  histfile->WriteTObject(g_clusterTimeFractionInWindowInGoodECLRings__cid, "g_clusterTimeFractionInWindowInGoodECLRings__cid");
  histfile->WriteTObject(clusterTimeFractionInWindow, "clusterTimeFractionInWindow");



  /*  -----------------------------------------------------------
      Fit the time histograms for different energy slices  */

  vector <int> binProjectionLeft  = binProjectionLeft_Time_vs_E_runDep;
  vector <int> binProjectionRight = binProjectionRight_Time_vs_E_runDep;

  auto h2 = clusterTimeClusterE;


  double max_E = h2->GetXaxis()->GetXmax();

  // Determine the energy bins.  Save the left edge for histogram purposes
  vector <double> E_binEdges(binProjectionLeft.size() + 1);
  for (long unsigned int x_bin = 0; x_bin < binProjectionLeft.size(); x_bin++) {
    TH1D* h_E_t_slice = h2->ProjectionX("h_E_t_slice", 1, 1) ;
    E_binEdges[x_bin] = h_E_t_slice->GetXaxis()->GetBinLowEdge(binProjectionLeft[x_bin]) ;
    B2INFO("E_binEdges[" << x_bin << "] = " << E_binEdges[x_bin]);
    if (x_bin == binProjectionLeft.size() - 1) {
      E_binEdges[x_bin + 1] = max_E ;
      B2INFO("E_binEdges[" << x_bin + 1 << "] = " << E_binEdges[x_bin + 1]);
    }
  }


  auto clusterTimePeak_ClusterEnergy_varBin = new TH1F("clusterTimePeak_ClusterEnergy_varBin",
                                                       ";ECL cluster energy [GeV];Cluster time fit position [ns]", E_binEdges.size() - 1, &(E_binEdges[0]));
  auto clusterTimePeakWidth_ClusterEnergy_varBin = new TH1F("clusterTimePeakWidth_ClusterEnergy_varBin",
                                                            ";ECL cluster energy [GeV];Cluster time fit width [ns]", E_binEdges.size() - 1, &(E_binEdges[0]));

  int Ebin_counter = 1 ;

  // Loop over all the energy bins
  for (long unsigned int x_bin = 0; x_bin < binProjectionLeft.size(); x_bin++) {
    double clusterTime_mean = 0;
    double clusterTime_mean_unc = 0;
    double clusterTime_sigma = 0;

    B2INFO("x_bin = " << x_bin);

    /* Determining which bins to mask out for mean calculation
    */
    TH1D* h_time = h2->ProjectionY(("h_time_E_slice_" + std::to_string(x_bin)).c_str(), binProjectionLeft[x_bin],
                                   binProjectionRight[x_bin]) ;


    TH1D* h_E_t_slice = h2->ProjectionX("h_E_t_slice", 1, 1) ;
    double lowE  = h_E_t_slice->GetXaxis()->GetBinLowEdge(binProjectionLeft[x_bin]) ;
    double highE = h_E_t_slice->GetXaxis()->GetBinUpEdge(binProjectionRight[x_bin]) ;
    double meanE = (lowE + highE) / 2.0 ;

    B2INFO("bin " << Ebin_counter << ": low E = " << lowE << ", high E = " << highE << " GeV");

    TH1D* h_timeMask = (TH1D*)h_time->Clone();
    TH1D* h_timeMasked = (TH1D*)h_time->Clone((std::string("h_time_E_slice_masked__") + std::to_string(meanE)).c_str());
    TH1D* h_timeRebin = (TH1D*)h_time->Clone();


    if (meanCleanRebinFactor != 1 || meanCleanCutMinFactor != 1) {

      h_timeRebin->Rebin(meanCleanRebinFactor);

      h_timeMask->Scale(0.0);   // set all bins to being masked off

      time_fit_min = hist_tmax;   // Set min value to largest possible value so that it gets reset
      time_fit_max = hist_tmin;   // Set max value to smallest possible value so that it gets reset

      // Find value of bin with max value
      double histRebin_max = h_timeRebin->GetMaximum();

      bool maskedOutNonZeroBin = false;
      // Loop over all bins to find those with content less than a certain threshold.  Mask the non-rebinned histogram for the corresponding bins
      for (int bin = 1; bin <= h_timeRebin->GetNbinsX(); bin++) {
        for (int rebinCounter = 1; rebinCounter <= meanCleanRebinFactor; rebinCounter++) {
          int nonRebinnedBinNumber = (bin - 1) * meanCleanRebinFactor + rebinCounter;
          if (nonRebinnedBinNumber < h_time->GetNbinsX()) {
            if (h_timeRebin->GetBinContent(bin) >= histRebin_max * meanCleanCutMinFactor) {
              h_timeMask->SetBinContent(nonRebinnedBinNumber, 1);

              // Save the lower and upper edges of the rebin histogram time range for fitting purposes
              double x_lower = h_timeRebin->GetXaxis()->GetBinLowEdge(bin);
              double x_upper = h_timeRebin->GetXaxis()->GetBinUpEdge(bin);
              if (x_lower < time_fit_min) {
                time_fit_min = x_lower;
              }
              if (x_upper > time_fit_max) {
                time_fit_max = x_upper;
              }

            } else {
              if (h_time->GetBinContent(nonRebinnedBinNumber) > 0) {
                B2DEBUG(22, "Setting bin " << nonRebinnedBinNumber << " from " << h_timeMasked->GetBinContent(nonRebinnedBinNumber) << " to 0");
                maskedOutNonZeroBin = true;
              }
              h_timeMasked->SetBinContent(nonRebinnedBinNumber, 0);
            }
          }
        }
      }
      B2INFO("Bins with non-zero values have been masked out: " << maskedOutNonZeroBin);
      h_timeMasked->ResetStats();
      h_timeMask->ResetStats();

    }


    // Calculate mean from masked histogram
    double default_meanMasked = h_timeMasked->GetMean();
    //double default_meanMasked_unc = h_timeMasked->GetMeanError();
    B2INFO("default_meanMasked = " << default_meanMasked);


    // Get the overall mean and standard deviation of the distribution within the plot.  This doesn't require a fit.
    double default_mean = h_time->GetMean();
    double default_mean_unc = h_time->GetMeanError();
    double default_sigma = h_time->GetStdDev();

    B2INFO("Fitting crystal between " << time_fit_min << " and " << time_fit_max);

    // gaus(0) is a substitute for [0]*exp(-0.5*((x-[1])/[2])**2)
    TF1* gaus = new TF1("func", "gaus(0)", time_fit_min, time_fit_max);
    gaus->SetParNames("numCrystalHitsNormalization", "mean", "sigma");
    /*
       gaus->ReleaseParameter(0);  // number of crystals
       gaus->ReleaseParameter(1);  // mean
       gaus->ReleaseParameter(2);  // standard deviation
    */

    double hist_max = h_time->GetMaximum();

    //=== Estimate initial value of sigma as std dev.
    double stddev = h_time->GetStdDev();
    sigma = stddev;
    mean = default_mean;

    //=== Setting parameters for initial iteration
    gaus->SetParameter(0, hist_max / 2.);
    gaus->SetParameter(1, mean);
    gaus->SetParameter(2, sigma);
    // L -- Use log likelihood method
    // I -- Use integral of function in bin instead of value at bin center  // not using
    // R -- Use the range specified in the function range
    // B -- Fix one or more parameters with predefined function   // not using
    // Q -- Quiet mode

    h_timeMasked->Fit(gaus, "LQR");  // L for likelihood, R for x-range, Q for fit quiet mode

    double fit_mean     = gaus->GetParameter(1);
    double fit_mean_unc = gaus->GetParError(1);
    double fit_sigma    = gaus->GetParameter(2);

    double meanDiff =  fit_mean - default_mean;
    double meanUncDiff = fit_mean_unc - default_mean_unc;
    double sigmaDiff = fit_sigma - default_sigma;

    bool good_fit = false;

    if ((fabs(meanDiff) > 10)      ||
        (fabs(meanUncDiff) > 10)   ||
        (fabs(sigmaDiff) > 10)     ||
        (fit_mean_unc > 3)         ||
        (fit_sigma < 0.1)          ||
        (fit_mean < time_fit_min)  ||
        (fit_mean > time_fit_max)) {
      B2INFO("x_bin = " << x_bin);
      B2INFO("fit mean, default mean = " << fit_mean << ", " << default_mean);
      B2INFO("fit mean unc, default mean unc = " << fit_mean_unc << ", " << default_mean_unc);
      B2INFO("fit sigma, default sigma = " << fit_sigma << ", " << default_sigma);

      B2INFO("crystal fit mean - hist mean = " << meanDiff);
      B2INFO("fit mean unc. - hist mean unc. = " << meanUncDiff);
      B2INFO("fit sigma - hist sigma = " << sigmaDiff);

      B2INFO("fit_mean = " << fit_mean);
      B2INFO("time_fit_min = " << time_fit_min);
      B2INFO("time_fit_max = " << time_fit_max);

      if (fabs(meanDiff) > 10)       B2INFO("fit mean diff too large");
      if (fabs(meanUncDiff) > 10)    B2INFO("fit mean unc diff too large");
      if (fabs(sigmaDiff) > 10)      B2INFO("fit mean sigma diff too large");
      if (fit_mean_unc > 3)          B2INFO("fit mean unc too large");
      if (fit_sigma < 0.1)           B2INFO("fit sigma too small");

    } else {
      good_fit = true;
    }


    int numEntries = h_time->GetEntries();
    /* If number of entries in histogram is greater than X then use the statistical information
       from the data otherwise leave crystal uncalibrated.  Histograms are still shown though.
       ALSO require the that fits are good. */
    if ((numEntries >= minNumEntries)  &&  good_fit) {
      clusterTime_mean = fit_mean;
      clusterTime_mean_unc = fit_mean_unc;
      clusterTime_sigma = fit_sigma;
    } else {
      clusterTime_mean = default_mean;
      clusterTime_mean_unc = default_mean_unc;
      clusterTime_sigma = default_sigma;
    }

    if (numEntries < minNumEntries)   B2INFO("Number of entries less than minimum");
    if (numEntries == 0)   B2INFO("Number of entries == 0");

    histfile->WriteTObject(h_time, (std::string("h_time_E_slice") + std::to_string(meanE)).c_str());
    histfile->WriteTObject(h_timeMasked, (std::string("h_time_E_slice_masked") + std::to_string(meanE)).c_str());

    // store mean cluster time info in a separate histogram
    clusterTimePeak_ClusterEnergy_varBin->SetBinContent(Ebin_counter, clusterTime_mean);
    clusterTimePeak_ClusterEnergy_varBin->SetBinError(Ebin_counter, clusterTime_mean_unc);

    clusterTimePeakWidth_ClusterEnergy_varBin->SetBinContent(Ebin_counter, clusterTime_sigma);
    clusterTimePeakWidth_ClusterEnergy_varBin->SetBinError(Ebin_counter, 0);

    Ebin_counter++;

    delete gaus;
  }



  /***************************************************************************
  For the user, print out some information about the peak cluster times.
  It is sorted by the absolute value of the peak cluster time so that the
  worst times are at the end.
  ***************************************************************************/

  // Vector to store element with respective present index
  vector< pair<double, int> > fitClusterTime__crystalIDBase0__pairs;

  // Prepare a vector of pairs containing the fitted cluster time and cell ID (base 0)
  for (int cid = 0; cid < 8736; cid++) {
    fitClusterTime__crystalIDBase0__pairs.push_back(make_pair(0.0, cid));
  }

  // Inserting element in pair vector to keep track of crystal id.
  for (int crys_id = cellIDLo; crys_id <= cellIDHi; crys_id++) {
    fitClusterTime__crystalIDBase0__pairs[crys_id - 1] = make_pair(fabs(t_offsets[crys_id - 1]), crys_id - 1) ;
  }

  // Sorting by the absolute value of the fitted cluster time for the crystal
  sort(fitClusterTime__crystalIDBase0__pairs.begin(), fitClusterTime__crystalIDBase0__pairs.end());


  // Print out the fitted peak cluster time values sorted by their absolute value
  B2INFO("-------- List of the (fitted) peak cluster times sorted by their absolute value ----------");
  B2INFO("------------------------------------------------------------------------------------------");
  B2INFO("------------------------------------------------------------------------------------------");
  B2INFO("Quoted # of clusters is before the cutting off of the distribution tails, cellID=1..8736, crysID=0..8735");

  bool hasHitThresholdBadTimes = false ;
  for (int iSortedTimes = 0; iSortedTimes < 8736; iSortedTimes++) {
    int cid = fitClusterTime__crystalIDBase0__pairs[iSortedTimes].second ;
    if (!hasHitThresholdBadTimes && fitClusterTime__crystalIDBase0__pairs[iSortedTimes].first > 2) {
      B2INFO("======== |t_fit| > Xns threshold ======");
      hasHitThresholdBadTimes = true;
    }
    //B2INFO("crystal ID = " << cid << ", peak clust t = " << t_offsets[cid] << " +- " << t_offsets_unc[cid] << ", # clusters = " << numClusterPerCrys[cid] << ", fabs(t) = " << fitClusterTime__crystalIDBase0__pairs[iSortedTimes].first );
    B2INFO("cid = " << cid << ", peak clust t = " << t_offsets[cid] << " +- " << t_offsets_unc[cid] << " ns, # clust = " <<
           numClusterPerCrys[cid] << ", good fit = " << crysHasGoodFit[cid] << ", good fit & stats = " << crysHasGoodFitandStats[cid]);
  }





  // Print out just a subset that definitely don't look good even though they have good stats.
  B2INFO("######## List of poor (fitted) peak cluster times sorted by their absolute value #########");
  B2INFO("##########################################################################################");
  B2INFO("##########################################################################################");

  for (int iSortedTimes = 0; iSortedTimes < 8736; iSortedTimes++) {
    int cid = fitClusterTime__crystalIDBase0__pairs[iSortedTimes].second ;
    if (fitClusterTime__crystalIDBase0__pairs[iSortedTimes].first > 2  && crysHasGoodFitandStats[cid]) {
      B2INFO("WARNING: cid = " << cid << ", peak clust t = " << t_offsets[cid] << " +- " << t_offsets_unc[cid] << " ns, # clust = " <<
             numClusterPerCrys[cid] << ", good fit = " << crysHasGoodFit[cid] << ", good fit & stats = " << crysHasGoodFitandStats[cid]);
    }
  }


  B2INFO("~~~~~~~~");
  B2INFO("Number of crystals with non-zero number of hits = " << numCrysWithNonZeroEntries);
  B2INFO("Number of crystals with good quality fits = " << numCrysWithGoodFit);


  clusterTimePeak_ClusterEnergy_varBin->ResetStats();
  clusterTimePeakWidth_ClusterEnergy_varBin->ResetStats();

  histfile->WriteTObject(clusterTimePeak_ClusterEnergy_varBin, "clusterTimePeak_ClusterEnergy_varBin");
  histfile->WriteTObject(clusterTimePeakWidth_ClusterEnergy_varBin, "clusterTimePeakWidth_ClusterEnergy_varBin");



  /* Fill histograms with the difference in the ts values from this iteration
     and the previous values read in from the payload. */
  B2INFO("Filling histograms for difference in crystal payload values and the pre-calibration values.  These older values may be from a previous bucket or older reprocessing of the data.");
  for (int crys_id = 1; crys_id <= 8736; crys_id++) {
    double tsDiffCustomOld_ns = -999;
    if (readPrevCrysPayload) {
      tsDiffCustomOld_ns = (currentValuesCrys[crys_id - 1] - prevValuesCrys[crys_id - 1]) * TICKS_TO_NS;
      B2INFO("Crystal " << crys_id << ": ts new merged - 'before 1st iter' merged = (" <<
             currentValuesCrys[crys_id - 1]  << " - " << prevValuesCrys[crys_id - 1]  <<
             ") ticks * " << TICKS_TO_NS << " ns/tick = " << tsDiffCustomOld_ns << " ns");

    }
    tsNew_MINUS_tsCustomPrev__cid->SetBinContent(crys_id, tsDiffCustomOld_ns);
    tsNew_MINUS_tsCustomPrev__cid->SetBinError(crys_id, 0);
    tsNew_MINUS_tsCustomPrev__cid->ResetStats();

    tsNew_MINUS_tsCustomPrev->Fill(tsDiffCustomOld_ns);
    tsNew_MINUS_tsCustomPrev->ResetStats();
  }

  histfile->WriteTObject(tsNew_MINUS_tsCustomPrev__cid, "tsNew_MINUS_tsCustomPrev__cid");
  histfile->WriteTObject(tsNew_MINUS_tsCustomPrev, "tsNew_MINUS_tsCustomPrev");


  histfile->Close();

  B2INFO("Finished validations algorithm");
  return c_OK;
}

