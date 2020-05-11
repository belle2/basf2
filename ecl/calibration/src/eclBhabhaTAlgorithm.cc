/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <ecl/calibration/eclBhabhaTAlgorithm.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dbobjects/ECLReferenceCrystalPerCrateCalib.h>
#include <ecl/digitization/EclConfiguration.h>
#include <ecl/utility/ECLChannelMapper.h>

#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/Configuration.h>

#include "TH2F.h"
#include "TFile.h"
#include "TF1.h"
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
  collectorName("ECLBhabhaTCollector")
  // Private members
  //m_runCount(0)
{
  setDescription("Calculate time offsets from bhabha events by fitting gaussian function to the (t - T0) difference.");
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

  TFile* histfile = 0;
  unique_ptr<TTree> tree_crystal(new TTree("tree_crystal", "Debug data from bhabha time calibration algorithm for crystals"));

  unique_ptr<TTree>   tree_crate(new TTree("tree_crate",   "Debug data from bhabha time calibration algorithm for crates"));
  int tree_cid;

  // Vector of time offsets to be saved in the database.
  vector<float> t_offsets;
  // Vector of time offset uncertainties to be saved in the database.
  vector<float> t_offsets_unc;
  vector<float> t_offsets_prev;  // previous time offsets


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


  // Need to load information about the event/run/experiment to get the right database information
  // Will be used for:
  // * ECLChannelMapper (to map crystal to crates)
  // * crystal payload updating for iterating crystal and crate fits
  int eventNumberForCrates = 1;

  auto& conf = Conditions::Configuration::getInstance();
  conf.prependTestingPayloadLocation("localdb/database.txt");

  /** Create the DBObjPtr for the payloads that we want to read from the DB */
  DBObjPtr<Belle2::ECLCrystalCalib> GammaGamma("ECLCrystalEnergyGammaGamma");
  DBObjPtr<Belle2::ECLCrystalCalib> Existing("ECLCrystalEnergy");


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

  /*
    //------------------------------------------------------------------------
    std::vector<float> GammaGammaCalib;
    std::vector<float> GammaGammaCalibUnc;
    GammaGammaCalib = GammaGamma->getCalibVector();
    GammaGammaCalibUnc = GammaGamma->getCalibUncVector();

    std::vector<float> ExistingCalib;
    std::vector<float> ExistingCalibUnc;
    ExistingCalib = Existing->getCalibVector();
    ExistingCalibUnc = Existing->getCalibUncVector();

    //------------------------------------------------------------------------
    std::vector<float> NewCalib;
    std::vector<float> NewCalibUnc;
    NewCalib.resize(8736);
    NewCalibUnc.resize(8736);
    for (int ic = 0; ic < 8736; ic++) {
      if (GammaGammaCalib[ic] > 0.) {
        NewCalib[ic] = GammaGammaCalib[ic];
        NewCalibUnc[ic] = GammaGammaCalibUnc[ic];
      } else {
        NewCalib[ic] = ExistingCalib[ic];
        NewCalibUnc[ic] = ExistingCalibUnc[ic];
      }
    }
  */


  updateDBObjPtrs(eventNumberForCrates, minRunNum, minExpNum);
  unique_ptr<ECLChannelMapper> crystalMapper(new ECL::ECLChannelMapper());
  crystalMapper->initFromDB();


  //------------------------------------------------------------------------
  //..Read payloads from database
  DBObjPtr<Belle2::ECLCrystalCalib> existingObject("ECLCrystalTimeOffset");
  B2INFO("Dumping payload");

  //..Get vectors of values from the payloads
  std::vector<float> currentValues = existingObject->getCalibVector();
  std::vector<float> currentUnc = existingObject->getCalibUncVector();

  //..Print out a few values for quality control
  B2INFO("Values read from database");
  for (int ic = 0; ic < 9000; ic += 500) {
    B2INFO("cellID " << ic + 1 << " " << currentValues[ic] << " +/- " << currentUnc[ic]);
  }


  auto refCrysIDzeroingCrate = getObjectPtr<TH1F>("refCrysIDzeroingCrate");

  // crystal index for the crystals (one per crate) that is used as the reference crystal.  This one has
  //    ts defined as zero.  The crystal id runs 1...8636, not starting at 0.
  // vector <short> crystalIDReferenceForZeroTs = {2305, 2309, 2313, 2317, 2321, 2325, 2329, 2333, 2337, 2341, 2345, 2349, 2353, 2357, 2361, 2365, 2369, 2373, 2377, 2381, 2385, 2389, 2393, 2397, 2401, 2405, 2409, 2413, 2417, 2421, 2425, 2429, 2433, 2437, 2441, 2445, 667, 583, 595, 607, 619, 631, 643, 655, 8256, 8172, 8184, 8196, 8208, 8220, 8232, 8244}; // original has bad crystals
  //  vector <short> crystalIDReferenceForZeroTs = {2306, 2309, 2313, 2317, 2321, 2326, 2329, 2334, 2337, 2343, 2348, 2349, 2356, 2357, 2361, 2365, 2372, 2373, 2377, 2381, 2388, 2391, 2393, 2399, 2401, 2407, 2409, 2413, 2417, 2421, 2426, 2429, 2433, 2440, 2585, 2446, 671, 583, 595, 607, 619, 631, 643, 655, 8252, 8177, 8185, 8192, 8206, 8224, 8228, 8244};  // each crystal distribution checked and approved.

  // Extract from the histogram the list of crystals to be used as reference crystals.
  //    The crystal id runs 1...8636, not starting at 0.
  vector <short> crystalIDreferenceUntested;
  for (int bin = 1; bin <= 8736; bin++) {
    if (refCrysIDzeroingCrate->GetBinContent(bin) > 0.5) {
      crystalIDreferenceUntested.push_back(bin);
    }
  }

  // Output for the user the crystal id to be used as a reference
  //    and also state which crate the crystal is in.
  B2INFO("Reference crystals to define as having ts=0.  Base 1 counting for both crates and crystals");
  for (long unsigned int crysRefCounter = 0; crysRefCounter < crystalIDreferenceUntested.size(); crysRefCounter++) {
    int crys_id = crystalIDreferenceUntested[crysRefCounter] ;
    int crate_id_from_crystal = crystalMapper->getCrateID(crys_id);
    B2INFO("   crystal " << crys_id << " is a reference for crate " << crate_id_from_crystal);
  }

  // Check that the reference crystals make sense.  There should be exactly one per crate but
  //   since the crystal calibration executes over multiple runs, it is possible that the
  //   reference crystals could change but we don't want to allow this.
  B2INFO("Checking number of reference crystals");
  B2INFO("Number of reference crystals = " << crystalIDreferenceUntested.size());

  // Check that there are exactly 52 reference crystals
  if (crystalIDreferenceUntested.size() != 52) {
    B2FATAL("The number of reference crystals does not equal 52, which is one per crate");
    return c_Failure;
  } else {
    B2INFO("Number of reference crystals is 52 as required");
  }

  // Count the number of reference crystals for each crate to make sure that there is exactly
  //    one reference crystal for each crate.
  //    also fill the final vector that maps the crate id to the reference crystal id
  vector <short> crateIDsNumRefCrystalsUntested(52, 0);
  vector <short> crystalIDReferenceForZeroTs(52, 0);

  for (long unsigned int crysRefCounter = 0; crysRefCounter < crystalIDreferenceUntested.size(); crysRefCounter++) {
    int crys_id = crystalIDreferenceUntested[crysRefCounter] ;
    int crate_id_from_crystal = crystalMapper->getCrateID(crys_id);
    crateIDsNumRefCrystalsUntested[crate_id_from_crystal - 1]++;
    crystalIDReferenceForZeroTs[crate_id_from_crystal - 1] = crys_id;
  }
  for (int crateTest = 0; crateTest < 52; crateTest++) {
    if (crateIDsNumRefCrystalsUntested[crateTest] != 1) {
      B2FATAL("Crate " << crateTest + 1 << " (base 1) has " << crateIDsNumRefCrystalsUntested[crateTest] << " reference crystals");
      return c_Failure;
    }
  }
  B2INFO("All reference crystals are reasonably mapped one crystal to one crate for all crates");











  bool saveRefCrysPayload = true;




  ECLReferenceCrystalPerCrateCalib* refCrystalsCalib = new ECLReferenceCrystalPerCrateCalib();
  refCrystalsCalib->setCalibVector(crystalIDReferenceForZeroTs);


  // Save the information to the payload if there is at least one crate
  //    begin calibrated.
  if (saveRefCrysPayload) {
    saveCalibration(refCrystalsCalib, "ECLReferenceCrystalPerCrateCalib");
    B2INFO("Reference crystal per crate payload made");
  }

















  B2INFO("Debug output rootfile: " << debugFilename);
  histfile = new TFile(debugFilename.c_str(), "recreate");


  TimevsCrysPrevCrateCalibPrevCrystCalib ->Write();
  TimevsCratePrevCrateCalibPrevCrystCalib->Write();
  TimevsCrysNoCalibrations               ->Write();
  TimevsCrysPrevCrateCalibNoCrystCalib   ->Write();
  TimevsCrateNoCrateCalibPrevCrystCalib  ->Write();

  cutflow->Write();


  if (debugOutput) {
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

  B2INFO("hist_tmin = " << hist_tmin);
  B2INFO("hist_tmax = " << hist_tmax);


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

    B2INFO(" TsDatabase->GetBinContent(i) = " << TsDatabase->GetBinContent(i));
    B2INFO("t_offsets_prev = " << t_offsets_prev[i - 1]);


    t_offsets_unc.push_back(TsDatabaseUnc->GetBinContent(i - 1) / numTimesFilled);
  }


  for (int crys_id = cellIDLo; crys_id <= cellIDHi; crys_id++) {
    crystalCalibSaved = 0;

    double database_mean = 0;
    double database_mean_unc = 0;

    B2INFO("Crystal id = " << crys_id);


    vector<bool> ts_new_was_set(8736, false);


    /* Determining which bins to mask out for mean calculation
    */

    TH1D* h_time = TimevsCrysPrevCrateCalibNoCrystCalib->ProjectionY((std::string("h_time_psi__") + std::to_string(crys_id)).c_str(),
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

      if (fabs(meanDiff) > 10)       B2INFO("fit mean diff too large");
      if (fabs(meanUncDiff) > 10)    B2INFO("fit mean unc diff too large");
      if (fabs(sigmaDiff) > 10)      B2INFO("fit mean sigma diff too large");
      if (fit_mean_unc > 3)          B2INFO("fit mean unc too large");
      if (fit_sigma < 0.1)           B2INFO("fit sigma too small");

    } else {
      good_fit = true;
    }



    // Set the tree_crystal values - ignore fit values  !!!!!!!!!!!!!!!!
    sigma = default_sigma;


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

    if (numEntries < minNumEntries)   B2INFO("Number of entries less than minimum");
    if (numEntries == 0)   B2INFO("Number of entries == 0");


    tree_cid  = crys_id;

    // For the database, convert back from ns to ADC ticks.
    t_offsets[crys_id - 1] = database_mean / TICKS_TO_NS;
    t_offsets_unc[crys_id - 1] = database_mean_unc / TICKS_TO_NS;


    histfile->WriteTObject(h_time, (std::string("h_time_psi") + std::to_string(crys_id)).c_str());
    histfile->WriteTObject(h_timeMasked, (std::string("h_time_psi_masked") + std::to_string(crys_id)).c_str());

    mean = database_mean;
    mean_unc = database_mean_unc;

    tsPrev = t_offsets_prev[crys_id - 1] * TICKS_TO_NS;

    delete gaus;
    tree_crystal->Fill();
  }

















  vector <double> tsRefCID ;
  B2INFO("crystal times before shift");
  for (int crate_id = 1; crate_id <= 52; crate_id++) {
    tsRefCID.push_back(t_offsets[ crystalIDReferenceForZeroTs[crate_id - 1] - 1 ]);
    B2INFO("crystal time [crystal = " << crystalIDReferenceForZeroTs[crate_id - 1] << ", crate = " << crate_id  << " (base 1)] = " <<
           t_offsets[ crystalIDReferenceForZeroTs[crate_id - 1] ] << " ns");
  }

  B2INFO("crystal times after shift wrt reference crystal");
  for (int crys_id = 1; crys_id <= 8736; crys_id++) {
    int crate_id_from_crystal = crystalMapper->getCrateID(crys_id);
    B2INFO("crystal time before shift [crystal = " << crys_id << ", crate = " << crate_id_from_crystal  << " (base 1)] = " <<
           t_offsets[ crys_id - 1 ] << " ns");
    t_offsets[crys_id - 1] = t_offsets[crys_id - 1] - tsRefCID[crate_id_from_crystal - 1];
    B2INFO("crystal time after shift [crystal = " << crys_id << ", crate = " << crate_id_from_crystal  << " (base 1)] = " <<
           t_offsets[ crys_id - 1 ] << " ns");
  }



















  ECLCrystalCalib* BhabhaTCalib = new ECLCrystalCalib();
  BhabhaTCalib->setCalibVector(t_offsets, t_offsets_unc);


  // Save the information to the payload if there is at least one crystal
  //    begin calibrated.
  if (cellIDLo <= cellIDHi) {
    saveCalibration(BhabhaTCalib, "ECLCrystalTimeOffset");
    B2DEBUG(30, "crystal payload made");
  }


  B2DEBUG(30, "end of crystal start of crate corrections .....");


  /* CRATE CORRECTIONS */

  hist_tmin = TimevsCrateNoCrateCalibPrevCrystCalib->GetYaxis()->GetXmin();
  hist_tmax = TimevsCrateNoCrateCalibPrevCrystCalib->GetYaxis()->GetXmax();

  B2DEBUG(30, "Found min/max of X axis of TimevsCrateNoCrateCalibPrevCrystCalib");

  // Vector of time offsets to be saved in the database.

  auto TcrateDatabase = getObjectPtr<TH1F>("TcrateDatabase");


  B2DEBUG(30, "Retrieved Ts and Tcrate histograms from tcol root file");


  vector<float> tcrate_mean_new(52, 0.0);
  vector<float> tcrate_mean_unc_new(52, 0.0);
  vector<float> tcrate_sigma_new(52, 0.0);
  vector<float> tcrate_mean_prev(52, 0.0);
  // vector<float> tcrate_sigma_prev(52, 0.0); // currently not used
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
    double sigma_crate = stddev;
    double mean_crate = default_mean_crate;
    gaus->SetParameter(0, hist_max / 2.);
    gaus->SetParameter(1, mean_crate);
    gaus->SetParameter(2, sigma_crate);

    h_time_crate_masked->Fit(gaus, "LQR");  // L for likelihood, R for x-range, Q for fit quiet mode

    double fit_mean_crate     = gaus->GetParameter(1);
    double fit_mean_crate_unc = gaus->GetParError(1);
    double fit_sigma_crate    = gaus->GetParameter(2);

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
    double database_mean_crate = 0;
    double database_mean_crate_unc = 0;
    if ((numEntries >= minNumEntries)  &&  good_fit) {

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

    delete gaus;
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
      t_offsets_crate_unc[crys_id - 1] = tcrate_mean_unc_new[crate_id_from_crystal - 1] / TICKS_TO_NS;

    } else {
      t_offsets_crate[crys_id - 1] = tcrate_mean_prev[crate_id_from_crystal - 1];
      B2INFO("used old crate mean but zeroed uncertainty since not saved in root file");
    }

  }

  ECLCrystalCalib* BhabhaTCrateCalib = new ECLCrystalCalib();
  BhabhaTCrateCalib->setCalibVector(t_offsets_crate, t_offsets_crate_unc);


  // Save the information to the payload if there is at least one crate
  //    begin calibrated.
  if (crateIDLo <= crateIDHi) {
    saveCalibration(BhabhaTCrateCalib, "ECLCrateTimeOffset");
    B2DEBUG(30, "crate payload made");
  }


  int tree_crateid;
  int tree_runNum;
  double tree_tcrate_mean;
  double tree_tcrate_mean_unc;
  double tree_tcrate_sigma;
  double tree_tcrate_meanPrev;

  tree_crate->Branch("runNum", &tree_runNum)->SetTitle("Run number, 0..infinity and beyond!");
  tree_crate->Branch("crateid", &tree_crateid)->SetTitle("Crate id, 1..52");
  tree_crate->Branch("tcrate", &tree_tcrate_mean)->SetTitle("Crate time offset mean, tcrate, ns");
  tree_crate->Branch("tcratePrev", &tree_tcrate_meanPrev)->SetTitle("Previous crate time offset mean, tcrate, ns");
  tree_crate->Branch("tcrate_unc", &tree_tcrate_mean_unc)->SetTitle("Error of time tcrate mean, ns.");
  tree_crate->Branch("tcrate_sigma", &tree_tcrate_sigma)->SetTitle("Sigma of time tcrate distribution, ns");
  tree_crate->SetAutoSave(10);


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

  B2DEBUG(30, "end of crate corrections .....");

  tree_crystal->Write();
  tree_crate->Write();

  histfile->Close();

  B2INFO("Finished talgorithm");
  return c_OK;
}

