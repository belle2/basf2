#include <ecl/calibration/eclBhabhaTAlgorithm.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/digitization/EclConfiguration.h>
#include <framework/dataobjects/EventMetaData.h>

//#include <ecl/calibration/eclBhabhaTTools.cc>

#include "TH2F.h"
#include "TFile.h"
#include "TMath.h"
#include "TF1.h"
#include "TMinuit.h"
#include "TROOT.h"

using namespace std;
using namespace Belle2;
using namespace ECL;

eclBhabhaTAlgorithm::eclBhabhaTAlgorithm():
  // Parameters
  CalibrationAlgorithm("ECLBhabhaTCollector"),
  cellIDLo(1),
  cellIDHi(8736),
  meanCleanRebinFactor(1),
  meanCleanCutMinFactor(0),
  crateIDLo(1),
  crateIDHi(52),
  debugOutput(true),
  debugFilenameBase("eclBhabhaTAlgorithm"),   // base of filename (without ".root")
  collectorName("ECLBhabhaTCollector"),
  // Private members
  m_runCount(0)
{
  setDescription("Calculate time offsets from bhabha events by fitting gaussian function to the (t - T0) difference.");
}



eclBhabhaTAlgorithm::eclBhabhaTAlgorithm(string colName):
  // Parameters
  CalibrationAlgorithm(colName.c_str()),
  cellIDLo(1),
  cellIDHi(8736),
  meanCleanRebinFactor(1),
  meanCleanCutMinFactor(0),
  crateIDLo(1),
  crateIDHi(52),
  debugOutput(true),
  debugFilenameBase("eclBhabhaTAlgorithm"),   // base of filename (without ".root")
  collectorName("ECLBhabhaTCollector"),
  // Private members
  m_runCount(0)
{
  setDescription("Calculate time offsets from bhabha events by fitting gaussian function to the (t - T0) difference.");
  B2INFO("Setting user passed prefix");
  B2INFO("New prefix: getPrefix() = " << getPrefix());
}




eclBhabhaTAlgorithm::eclBhabhaTAlgorithm(bool is_eeGamma):
  // Parameters
  CalibrationAlgorithm("ECLBhabhaTCollector"),
  cellIDLo(1),
  cellIDHi(8736),
  meanCleanRebinFactor(1),
  meanCleanCutMinFactor(0),
  crateIDLo(1),
  crateIDHi(52),
  debugOutput(true),
  debugFilenameBase("eclBhabhaTAlgorithm"),   // base of filename (without ".root")
  collectorName("ECLBhabhaTCollector"),
  // Private members
  m_runCount(0)
{
  setDescription("Calculate time offsets from bhabha events by fitting gaussian function to the (t - T0) difference.");

  B2INFO("Default perfix: getPrefix() = " << getPrefix());
  if (is_eeGamma) {
    B2INFO("Setting ee gamma prefix");
    setPrefix("ECLBhabhaGammaTCollector");
    B2INFO("New prefix: getPrefix() = " << getPrefix());
  }
}





CalibrationAlgorithm::EResult eclBhabhaTAlgorithm::calibrate()
{
  /** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();


  /** Write out job parameters */
  B2INFO("eclBhabhaTAlgorithm parameters:");
  B2INFO("cellIDLo = " << cellIDLo);
  B2INFO("cellIDHi = " << cellIDHi);
  B2INFO("meanCleanRebinFactor = " << meanCleanRebinFactor);
  B2INFO("meanCleanCutMinFactor = " << meanCleanCutMinFactor);
  B2INFO("crateIDLo = " << crateIDLo);
  B2INFO("crateIDHi = " << crateIDHi);


  /* Histogram with the data collected by eclBhabhaTCollectorModule */

  auto TimevsCrysPrevCrateCalibPrevCrystCalib = getObjectPtr<TH2F>("TimevsCrysPrevCrateCalibPrevCrystCalib");
  auto TimevsCratePrevCrateCalibPrevCrystCalib = getObjectPtr<TH2F>("TimevsCratePrevCrateCalibPrevCrystCalib");
  auto TimevsCrysNoCalibrations = getObjectPtr<TH2F>("TimevsCrysNoCalibrations");
  auto TimevsCrysPrevCrateCalibNoCrystCalib = getObjectPtr<TH2F>("TimevsCrysPrevCrateCalibNoCrystCalib");
  auto TimevsCrateNoCrateCalibPrevCrystCalib = getObjectPtr<TH2F>("TimevsCrateNoCrateCalibPrevCrystCalib");

  // Collect other plots just for reference - combines all the runs for these plots.
  auto cutflow = getObjectPtr<TH1F>("cutflow");


  if (!TimevsCrysNoCalibrations) return c_Failure;

  /**-----------------------------------------------------------------------------------------------*/


  crystalMapper = new  ECL::ECLChannelMapper();
  string pathString = getenv("BELLE2_LOCAL_DIR");
  pathString += "/ecl/data/ecl_channels_map.txt";
  crystalMapper->initFromFile(pathString.c_str());



  TFile* histfile = 0;
  TTree* tree_crystal = 0;
  TTree* tree_crystal_final = 0;
  TTree* tree_crate = 0;
  int tree_cid;
  double chi2;

  // Vector of time offsets to be saved in the database.
  vector<float> t_offsets;
  // Vector of time offset uncertainties to be saved in the database.
  vector<float> t_offsets_unc;

  vector<float> t_offsets_prev;  // previous time offsets

  // temp copies of offsets
  vector<float> t_offsets_temp;
  vector<float> t_offsets_unc_temp;


  int minNumEntries = 40;


  double mean = 0;
  double sigma = -1;
  double mean_unc = 0;
  int crystalCalibSaved = 0;
  double tsPrev = 0;


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
  string runNumsString = string("_") + to_string(minExpNum) + "_" + to_string(minRunNum) + string("-") + to_string(
                           maxExpNum) + "_" + to_string(maxRunNum);
  string debugFilename = debugFilenameBase + runNumsString + string(".root");


  B2INFO("Debug output rootfile: " << debugFilename);
  histfile = new TFile(debugFilename.c_str(), "recreate");


  TimevsCrysPrevCrateCalibPrevCrystCalib ->Write();
  TimevsCratePrevCrateCalibPrevCrystCalib->Write();
  TimevsCrysNoCalibrations               ->Write();
  TimevsCrysPrevCrateCalibNoCrystCalib   ->Write();
  TimevsCrateNoCrateCalibPrevCrystCalib  ->Write();

  cutflow->Write();


  if (debugOutput) {
    tree_crystal = new TTree("tree_crystal", "Debug data from bhabha time calibration algorithm for crystals");
    tree_crystal->Branch("cid", &tree_cid)->SetTitle("Cell ID, 1..8736");
    tree_crystal->Branch("ts", &mean)->SetTitle("Time offset mean, ts, ns");
    tree_crystal->Branch("tsUnc", &mean_unc)->SetTitle("Error of time ts mean, ns.");
    tree_crystal->Branch("tsSigma", &sigma)->SetTitle("Sigma of time ts distribution, ns");
    tree_crystal->Branch("crystalCalibSaved",
                         &crystalCalibSaved)->SetTitle("0=crystal skipped, 1=crystal calib saved (num entries based)");
    tree_crystal->Branch("tsPrev", &tsPrev)->SetTitle("Previous crystal time offset, ts, ns");
    tree_crystal->SetAutoSave(10);
  }


  double hist_tmin = TimevsCrysNoCalibrations->GetYaxis()->GetXmin();
  double hist_tmax = TimevsCrysNoCalibrations->GetYaxis()->GetXmax();

  double time_fit_min = hist_tmax;   // Set min value to largest possible value so that it gets reset
  double time_fit_max = hist_tmin;   // Set max value to smallest possible value so that it gets reset

  cout << "hist_tmin = " << hist_tmin << "\n";
  cout << "hist_tmax = " << hist_tmax << "\n";


  //const double TICKS_TO_NS = 0.4931; // ns/clock
  const double TICKS_TO_NS = 1.0 / (4.0 * EclConfiguration::m_rf) *
                             1e3;  // 1/(4fRF) = 0.4913 ns/clock tick, where fRF is the accelerator RF frequency, fRF=508.889 MHz. Same for all crystals.  Proper accurate value

  // The ts and tcrate database values are filled once per tcol instance so count the number of times that the database values
  //    were summed together by the histogram merging process and extract out the original values again.
  auto databaseCounter = getObjectPtr<TH1F>("databaseCounter");
  float numTimesFilled = databaseCounter->GetBinContent(1);
  B2INFO("Number of times database histograms were merged = " << numTimesFilled);


  auto TsDatabase = getObjectPtr<TH1F>("TsDatabase");
  auto TsDatabaseUnc = getObjectPtr<TH1F>("TsDatabaseUnc");
  for (int i = 1; i <= 8736; i++) {
    t_offsets.push_back(TsDatabase->GetBinContent(i - 1) / numTimesFilled);
    t_offsets_prev.push_back(TsDatabase->GetBinContent(i) / numTimesFilled);

    cout << " TsDatabase->GetBinContent(i) = " << TsDatabase->GetBinContent(i) << "\n";
    cout << "t_offsets_prev = " << t_offsets_prev[i - 1] << "\n";


    t_offsets_unc.push_back(TsDatabaseUnc->GetBinContent(i - 1) / numTimesFilled);
  }


  for (int crys_id = cellIDLo; crys_id <= cellIDHi; crys_id++) {
    mean = 0;
    mean_unc = -1;
    sigma = -1;
    crystalCalibSaved = 0;

    double database_mean = 0;
    double database_mean_unc = 0;

    B2INFO("Crystal id = " << crys_id);


    vector<bool> ts_new_was_set(8736, false);


    /* Determining which bins to mask out for mean calculation
    */

    TH1D* h_time = TimevsCrysPrevCrateCalibNoCrystCalib->ProjectionY("h_time_psi", crys_id, crys_id);
    TH1D* h_timeMask = (TH1D*)h_time->Clone();
    TH1D* h_timeMasked = (TH1D*)h_time->Clone();
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
                B2INFO("Setting bin " << nonRebinnedBinNumber << " from " << h_timeMasked->GetBinContent(nonRebinnedBinNumber) << " to 0");
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
    double default_meanMasked_unc = h_timeMasked->GetMeanError();
    B2INFO("default_meanMasked = " << default_meanMasked);


    // Get the overall mean and standard deviation of the distribution within the plot.  This doesn't require a fit.
    double default_mean = h_time->GetMean();
    double default_mean_unc = h_time->GetMeanError();
    double default_sigma = h_time->GetStdDev();

    B2INFO("Fitting crystal between " << time_fit_min << " and " << time_fit_max);

    // gaus(0) is a substitute for [0]*exp(-0.5*((x-[1])/[2])**2)
    //TF1* gaus = new TF1("func", "gaus(0)", hist_tmin, hist_tmax);
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
    //gaus->SetParLimits(2, sig_min, sig_max);
    // L -- Use log likelihood method
    // I -- Use integral of function in bin instead of value at bin center
    // R -- Use the range specified in the function range     // DON'T USE ANYMORE !
    // B -- Fix one or more parameters with predefined function   /// DON'T USE ANYMORE !
    // Q -- Quiet mode
    //h_time->Fit(gaus, "LIRBQ", "", mean - stddev, mean + stddev);

    h_timeMasked->Fit(gaus, "LQR");  // L for likelihood, R for x-range, Q for fit quiet mode

    double fit_mean     = gaus->GetParameter(1);
    double fit_mean_unc = gaus->GetParError(1);
    double fit_sigma    = gaus->GetParameter(2);
    double fit_chi2     = gaus->GetChisquare();

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
      B2INFO("Crystal id = " << crys_id);
      B2INFO("fit mean, default mean = " << fit_mean << ", " << default_mean);
      B2INFO("fit mean unc, default mean unc = " << fit_mean_unc << ", " << default_mean_unc);
      B2INFO("fit sigma, default sigma = " << fit_sigma << ", " << default_sigma);

      B2INFO("crystal fit mean - hist mean = " << meanDiff);
      B2INFO("fit mean unc. - hist mean unc. = " << meanUncDiff);
      B2INFO("fit sigma - hist sigma = " << sigmaDiff);

      B2INFO("fit_mean = " << fit_mean);
      B2INFO("time_fit_min = " << time_fit_min);
      B2INFO("time_fit_max = " << time_fit_max);

      if (fabs(meanDiff) > 10)       B2INFO("fit mean diff too large\n");
      if (fabs(meanUncDiff) > 10)    B2INFO("fit mean unc diff too large\n");
      if (fabs(sigmaDiff) > 10)      B2INFO("fit mean sigma diff too large\n");
      if (fit_mean_unc > 3)          B2INFO("fit mean unc too large\n");
      if (fit_sigma < 0.1)           B2INFO("fit sigma too small\n");

    } else {
      good_fit = true;
    }



    // Set the tree_crystal values - ignore fit values  !!!!!!!!!!!!!!!!
    sigma = default_sigma;
    chi2 = -1;


    int numEntries = h_time->GetEntries();
    // If number of entries in histogram is greater than X then use the statistical information from the data otherwise leave crystal uncalibrated.  Histograms are still shown though.
    //  ALSO require the that fits are good.
    if ((numEntries >= minNumEntries)  &&  good_fit) {
      crystalCalibSaved = 1;
      database_mean = fit_mean;
      database_mean_unc = fit_mean_unc;
    } else {
      database_mean = default_mean;
      database_mean_unc = default_mean_unc;
    }

    if (numEntries < minNumEntries)   B2INFO("Number of entries less than minimum\n");
    if (numEntries == 0)   B2INFO("Number of entries == 0\n");


    tree_cid  = crys_id;

    // For the database, convert back from ns to ADC ticks.
    t_offsets[crys_id - 1] = database_mean / TICKS_TO_NS;
    t_offsets_unc[crys_id - 1] = database_mean_unc / TICKS_TO_NS;


    histfile->WriteTObject(h_time, (std::string("h_time_psi") + std::to_string(crys_id)).c_str());
    histfile->WriteTObject(h_timeMasked, (std::string("h_time_psi_masked") + std::to_string(crys_id)).c_str());

    mean = database_mean;
    mean_unc = database_mean_unc;

    tsPrev = t_offsets_prev[crys_id - 1] * TICKS_TO_NS;

    tree_crystal->Fill();
  }

  ECLCrystalCalib* BhabhaTCalib = new ECLCrystalCalib();
  BhabhaTCalib->setCalibVector(t_offsets, t_offsets_unc);
  saveCalibration(BhabhaTCalib, "ECLCrystalTimeOffset");


  B2DEBUG(30, "end of crystal start of crate corrections ..... for now");



  /* CRATE CORRECTIONS */

  hist_tmin = TimevsCrateNoCrateCalibPrevCrystCalib->GetYaxis()->GetXmin();
  hist_tmax = TimevsCrateNoCrateCalibPrevCrystCalib->GetYaxis()->GetXmax();

  B2DEBUG(30, "Found min/max of X axis of TimevsCrateNoCrateCalibPrevCrystCalib");

  // Vector of time offsets to be saved in the database.
  //t_offsets_crate.clear();

  auto TcrateDatabase = getObjectPtr<TH1F>("TcrateDatabase");


  B2DEBUG(30, "Retrieved Ts and Tcrate histograms from tcol root file");

  double database_mean_crate = 0;
  double database_mean_crate_unc = 0;

  double mean_crate = 0;
  double sigma_crate = -1;
  double mean_crate_unc = 0;
  int crateCalibSaved = 0;

  vector<float> tcrate_mean_new(52, 0.0);
  vector<float> tcrate_mean_unc_new(52, 0.0);
  vector<float> tcrate_sigma_new(52, 0.0);
  vector<float> tcrate_mean_prev(52, 0.0);
  vector<float> tcrate_sigma_prev(52, 0.0);
  vector<bool> tcrate_new_was_set(52, false);

  B2DEBUG(30, "crate vectors set");



  // Crate time calibration constants are saved per crystal so read them per crystal
  //    and save as one entry per crate in the array
  for (int crys_id = 1; crys_id <= 8736; crys_id++) {
    int crate_id_from_crystal = crystalMapper->getCrateID(crys_id);
    tcrate_mean_prev[crate_id_from_crystal - 1] = TcrateDatabase->GetBinContent(crys_id) / numTimesFilled;
  }

  for (int crate_id = crateIDLo; crate_id <= crateIDHi; crate_id++) {

    B2DEBUG(30, "Start of crate id = " << crate_id);

    TH1D* h_time_crate = TimevsCrateNoCrateCalibPrevCrystCalib->ProjectionY("h_time_psi_crate", crate_id, crate_id);
    TH1D* h_time_crate_mask = (TH1D*)h_time_crate->Clone();
    TH1D* h_time_crate_masked = (TH1D*)h_time_crate->Clone();
    TH1D* h_time_crate_rebin = (TH1D*)h_time_crate->Clone();



    // Do rebinning and cleaning of some bins but only if the user selection values call for it since it slows the code down
    if (meanCleanRebinFactor != 1 || meanCleanCutMinFactor != 1) {

      h_time_crate_rebin->Rebin(meanCleanRebinFactor);
      h_time_crate_mask->Scale(0.0);   // set all bins to being masked off

      time_fit_min = hist_tmax;   // Set min value to largest possible value so that it gets reset
      time_fit_max = hist_tmin;   // Set max value to smallest possible value so that it gets reset

      // Find value of bin with max value
      double histRebin_max = h_time_crate_rebin->GetMaximum();

      bool maskedOutNonZeroBin = false;
      // Loop over all bins to find those with content less than a certain threshold.  Mask the non-rebinned histogram for the corresponding bins
      for (int bin = 1; bin <= h_time_crate_rebin->GetNbinsX(); bin++) {
        for (int rebinCounter = 1; rebinCounter <= meanCleanRebinFactor; rebinCounter++) {
          int nonRebinnedBinNumber = (bin - 1) * meanCleanRebinFactor + rebinCounter;
          if (nonRebinnedBinNumber < h_time_crate->GetNbinsX()) {
            if (h_time_crate_rebin->GetBinContent(bin) >= histRebin_max * meanCleanCutMinFactor) {
              h_time_crate_mask->SetBinContent(nonRebinnedBinNumber, 1);

              // Save the lower and upper edges of the rebin histogram time range for fitting purposes
              double x_lower = h_time_crate_rebin->GetXaxis()->GetBinLowEdge(bin);
              double x_upper = h_time_crate_rebin->GetXaxis()->GetBinUpEdge(bin);
              if (x_lower < time_fit_min) {
                time_fit_min = x_lower;
              }
              if (x_upper > time_fit_max) {
                time_fit_max = x_upper;
              }
            } else {
              if (h_time_crate->GetBinContent(nonRebinnedBinNumber) > 0) {
                B2INFO("Setting bin " << nonRebinnedBinNumber << " from " << h_time_crate_masked->GetBinContent(nonRebinnedBinNumber) << " to 0");
                maskedOutNonZeroBin = true;
              }
              h_time_crate_masked->SetBinContent(nonRebinnedBinNumber, 0);
            }
          }
        }
      }
      B2INFO("Bins with non-zero values have been masked out: " << maskedOutNonZeroBin);
      h_time_crate_masked->ResetStats();
      h_time_crate_mask->ResetStats();

    }



    B2DEBUG(30, "crate loop - projected h_time_psi_crate");


    double default_mean_crate = h_time_crate_masked->GetMean();
    double default_mean_crate_unc = h_time_crate_masked->GetMeanError();
    double default_sigma_crate = h_time_crate_masked->GetStdDev();
    B2INFO("Fitting crate between " << time_fit_min << " and " << time_fit_max);
    TF1* gaus = new TF1("func", "gaus(0)", time_fit_min, time_fit_max);
    gaus->SetParNames("numCrateHisNormalization", "mean", "sigma");
    double hist_max = h_time_crate->GetMaximum();
    double stddev = h_time_crate->GetStdDev();
    sigma_crate = stddev;
    mean_crate = default_mean_crate;
    gaus->SetParameter(0, hist_max / 2.);
    gaus->SetParameter(1, mean_crate);
    gaus->SetParameter(2, sigma_crate);

    h_time_crate_masked->Fit(gaus, "LQR");  // L for likelihood, R for x-range, Q for fit quiet mode

    double fit_mean_crate     = gaus->GetParameter(1);
    double fit_mean_crate_unc = gaus->GetParError(1);
    double fit_sigma_crate    = gaus->GetParameter(2);
    double fit_chi2_crate     = gaus->GetChisquare();

    double meanDiff =  fit_mean_crate - default_mean_crate;
    double meanUncDiff = fit_mean_crate_unc - default_mean_crate_unc;
    double sigmaDiff = fit_sigma_crate - default_sigma_crate;

    bool good_fit = false;

    B2DEBUG(30, "Crate id = " << crate_id << " with crate mean = " << default_mean_crate << " +- " << fit_mean_crate_unc);

    if ((fabs(meanDiff) > 7)            ||
        (fabs(meanUncDiff) > 7)         ||
        (fabs(sigmaDiff) > 7)           ||
        (fit_mean_crate_unc > 3)        ||
        (fit_sigma_crate < 0.1)         ||
        (fit_mean_crate < time_fit_min) ||
        (fit_mean_crate > time_fit_max)) {
      B2INFO("Crate id = " << crate_id);
      B2INFO("fit mean, default mean = " << fit_mean_crate << ", " << default_mean_crate);
      B2INFO("fit sigma, default sigma = " << fit_sigma_crate << ", " << default_sigma_crate);

      B2INFO("crate fit mean - hist mean = " << meanDiff);
      B2INFO("fit mean unc. - hist mean unc. = " << meanUncDiff);
      B2INFO("fit sigma - hist sigma = " << sigmaDiff);
      B2INFO("fit_mean_crate = " << fit_mean_crate);
      B2INFO("time_fit_min = " << time_fit_min);
      B2INFO("time_fit_max = " << time_fit_max);
    } else {
      good_fit = true;
    }

    int numEntries = h_time_crate->GetEntries();
    B2INFO("Number entries = " << numEntries);
    if ((numEntries >= minNumEntries)  &&  good_fit) {
      crateCalibSaved = 1;

      database_mean_crate = fit_mean_crate;
      database_mean_crate_unc = fit_mean_crate_unc;

    } else {
      database_mean_crate = default_mean_crate;
      database_mean_crate_unc = default_mean_crate_unc;
    }
    if (numEntries == 0) {
      database_mean_crate = 0;
      database_mean_crate_unc = 0;
    }

    tcrate_mean_new[crate_id - 1] = database_mean_crate;
    tcrate_mean_unc_new[crate_id - 1] = database_mean_crate_unc;
    tcrate_sigma_new[crate_id - 1] = fit_sigma_crate;
    tcrate_new_was_set[crate_id - 1] = true;


    histfile->WriteTObject(h_time_crate, (std::string("h_time_psi_crate") + std::to_string(crate_id)).c_str());
    histfile->WriteTObject(h_time_crate_masked, (std::string("h_time_psi_crate_masked") + std::to_string(crate_id)).c_str());
    histfile->WriteTObject(h_time_crate_rebin, (std::string("h_time_psi_crate_rebinned") + std::to_string(crate_id)).c_str());
  }

  B2DEBUG(30, "crate histograms made");


  // Save database for crates
  // Vector of time offsets to be saved in the database.
  vector<float> t_offsets_crate;
  // Vector of time offset uncertainties to be saved in the database.
  vector<float> t_offsets_crate_unc;
  for (int i = 1; i <= 8736; i++) {
    t_offsets_crate.push_back(0);
    t_offsets_crate_unc.push_back(0);
  }



  for (int crys_id = 1; crys_id <= 8736; crys_id++) {

    int crate_id_from_crystal = crystalMapper->getCrateID(crys_id);
    if (tcrate_new_was_set[crate_id_from_crystal - 1]) {
      t_offsets_crate[crys_id - 1] = tcrate_mean_new[crate_id_from_crystal - 1] / TICKS_TO_NS;

      // USE SIGMA OR MEAN UNCERTAINTY ?!?!?!?!??!?
      t_offsets_crate_unc[crys_id - 1] = tcrate_mean_unc_new[crate_id_from_crystal - 1] / TICKS_TO_NS;

    } else {
      t_offsets_crate[crys_id - 1] = tcrate_mean_prev[crate_id_from_crystal - 1];
      cout << "used old crate mean but zeroed uncertainty since not saved in root file\n";
    }

  }

  ECLCrystalCalib* BhabhaTCrateCalib = new ECLCrystalCalib();
  BhabhaTCrateCalib->setCalibVector(t_offsets_crate, t_offsets_crate_unc);



  // While trying to run a lot of runs, use this other funny name so that I can run all the talgs in parallel and then rename the payloads later.
  // For proper running suquentially, use the first version
  saveCalibration(BhabhaTCrateCalib, "ECLCrateTimeOffset");   // sequential

  B2DEBUG(30, "crate payload made");



  int tree_crateid;
  int tree_runNum;
  double tree_tcrate_mean;
  double tree_tcrate_mean_unc;
  double tree_tcrate_sigma;
  double tree_tcrate_meanPrev;

  tree_crate = new TTree("tree_crate", "Debug data from bhabha time calibration algorithm for crates");
  tree_crate->Branch("runNum", &tree_runNum)->SetTitle("Run number, 0..infinity and beyond!");
  tree_crate->Branch("crateid", &tree_crateid)->SetTitle("Crate id, 1..52");
  tree_crate->Branch("tcrate", &tree_tcrate_mean)->SetTitle("Crate time offset mean, tcrate, ns");
  tree_crate->Branch("tcratePrev", &tree_tcrate_meanPrev)->SetTitle("Previous crate time offset mean, tcrate, ns");
  tree_crate->Branch("tcrate_unc", &tree_tcrate_mean_unc)->SetTitle("Error of time tcrate mean, ns.");
  tree_crate->Branch("tcrate_sigma", &tree_tcrate_sigma)->SetTitle("Sigma of time tcrate distribution, ns");
  tree_crate->SetAutoSave(10);


  bool firstRunInList = true;

  for (auto expRun : getRunList()) {
    // Key command to make sure your DBObjPtrs are correct
    B2INFO("run num, exp num:    " << expRun.second << ", " <<  expRun.first);
    int runNumber = expRun.second;

    for (int crate_id = 1; crate_id <= 52; crate_id++) {
      if (tcrate_new_was_set[crate_id - 1]) {
        tree_runNum = runNumber;
        tree_crateid = crate_id;
        tree_tcrate_mean = tcrate_mean_new[crate_id - 1];
        tree_tcrate_mean_unc = tcrate_mean_unc_new[crate_id - 1];
        tree_tcrate_sigma = tcrate_sigma_new[crate_id - 1];
        tree_tcrate_meanPrev = tcrate_mean_prev[crate_id - 1] * TICKS_TO_NS;
        tree_crate->Fill();
      }
    }
  }

  tree_crystal->Write();
  tree_crate->Write();


  return c_OK;
}

