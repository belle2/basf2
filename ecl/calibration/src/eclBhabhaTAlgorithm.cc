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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>

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
  savePrevCrysPayload(false),
  readPrevCrysPayload(false),
  debugOutput(true),
  debugFilenameBase("eclBhabhaTAlgorithm"),   // base of filename (without ".root")
  collectorName("ECLBhabhaTCollector"),
  refCrysPerCrate{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                   -1, -1, -1, -1, -1, -1, -1} //,
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
  B2INFO("savePrevCrysPayload = " << savePrevCrysPayload);
  B2INFO("readPrevCrysPayload = " << readPrevCrysPayload);
  B2INFO("refCrysPerCrate = {");
  for (int crateTest = 0; crateTest < 52 - 1; crateTest++) {
    B2INFO(refCrysPerCrate[crateTest] << ",");
  }
  B2INFO(refCrysPerCrate[52 - 1] << "}");


  /* Histogram with the data collected by eclBhabhaTCollectorModule */

  auto TimevsCrysPrevCrateCalibPrevCrystCalib = getObjectPtr<TH2F>("TimevsCrysPrevCrateCalibPrevCrystCalib");
  auto TimevsCratePrevCrateCalibPrevCrystCalib = getObjectPtr<TH2F>("TimevsCratePrevCrateCalibPrevCrystCalib");
  auto TimevsCrysNoCalibrations = getObjectPtr<TH2F>("TimevsCrysNoCalibrations");
  auto TimevsCrysPrevCrateCalibNoCrystCalib = getObjectPtr<TH2F>("TimevsCrysPrevCrateCalibNoCrystCalib");
  auto TimevsCrateNoCrateCalibPrevCrystCalib = getObjectPtr<TH2F>("TimevsCrateNoCrateCalibPrevCrystCalib");

  // Collect other plots just for reference - combines all the runs for these plots.
  auto cutflow = getObjectPtr<TH1F>("cutflow");


  // Define new plots to make
  // New calibration constant values minus older values from previous iteration plotted as a function of the crystal or crate id
  unique_ptr<TH1F> tsNew_MINUS_tsOld__cid(new TH1F("TsNew_MINUS_TsOld__cid",
                                                   ";cell id; ts(new|bhabha) - ts(previous iteration|merged)  [ns]", 8736,
                                                   1, 8736 + 1));
  unique_ptr<TH1F> tcrateNew_MINUS_tcrateOld__crateID(new TH1F("tcrateNew_MINUS_tcrateOld__crateID",
                                                      ";crate id; ts(new | bhabha) - ts(previous iteration | merged)  [ns]",
                                                      52, 1, 52 + 1));
  unique_ptr<TH1F> tsNew_MINUS_tsCustomPrev__cid(new TH1F("TsNew_MINUS_TsCustomPrev__cid",
                                                          ";cell id; ts(new|bhabha) - ts(old = 'before 1st iter'|merged)  [ns]",
                                                          8736, 1, 8736 + 1));
  unique_ptr<TH1F> tsNew_MINUS_tsOldBhabha__cid(new TH1F("TsNew_MINUS_TsOldBhabha__cid",
                                                         ";cell id; ts(new|bhabha) - ts(previous iteration|bhabha)  [ns]", 8736,
                                                         1, 8736 + 1));


  // Histogram of the new time constant values minus values from previous iteration
  unique_ptr<TH1F> tsNew_MINUS_tsOld(new TH1F("TsNew_MINUS_TsOld",
                                              ";ts(new | bhabha) - ts(previous iteration | merged)  [ns];Number of crystals",
                                              201, -10.05, 10.05));
  unique_ptr<TH1F> tcrateNew_MINUS_tcrateOld(new TH1F("tcrateNew_MINUS_tcrateOld",
                                                      ";tcrate(new) - tcrate(previous iteration)  [ns];Number of crates",
                                                      201, -10.05, 10.05));
  unique_ptr<TH1F> tsNew_MINUS_tsCustomPrev(new TH1F("TsNew_MINUS_TsCustomPrev",
                                                     ";ts(new | bhabha) - ts(old = 'before 1st iter' | merged)  [ns];Number of crystals",
                                                     285, -69.5801, 69.5801));
  unique_ptr<TH1F> tsNew_MINUS_tsOldBhabha(new TH1F("TsNew_MINUS_TsOldBhabha",
                                                    ";ts(new | bhabha) - ts(previous iteration | bhabha)  [ns];Number of crystals",
                                                    201, -10.05, 10.05));



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
  string runNumsString = string("_") + to_string(minExpNum) + "_" + to_string(minRunNum) + string("-") +
                         to_string(maxExpNum) + "_" + to_string(maxRunNum);
  string debugFilename = debugFilenameBase + runNumsString + string(".root");


  // Need to load information about the event/run/experiment to get the right database information
  // Will be used for:
  // * ECLChannelMapper (to map crystal to crates)
  // * crystal payload updating for iterating crystal and crate fits
  int eventNumberForCrates = 1;

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
  B2INFO("Reading payloads: ECLCrystalTimeOffset and ECLCrateTimeOffset");
  DBObjPtr<Belle2::ECLCrystalCalib> crystalTimeObject("ECLCrystalTimeOffset");
  DBObjPtr<Belle2::ECLCrystalCalib> crateTimeObject("ECLCrateTimeOffset");

  //..Get vectors of values from the payloads
  vector<float> currentValuesCrys = crystalTimeObject->getCalibVector();
  vector<float> currentUncCrys = crystalTimeObject->getCalibUncVector();
  vector<float> currentValuesCrate = crateTimeObject->getCalibVector();
  vector<float> currentUncCrate = crateTimeObject->getCalibUncVector();

  //..Print out a few values for quality control
  B2INFO("Values read from database.  Write out for their values for comparison against those from tcol");
  for (int ic = 0; ic < 8736; ic += 500) {
    B2INFO("ts: cellID " << ic + 1 << " " << currentValuesCrys[ic] << " +/- " << currentUncCrys[ic]);
    B2INFO("tcrate: cellID " << ic + 1 << " " << currentValuesCrate[ic] << " +/- " << currentUncCrate[ic]);
  }


  //..Read in the previous crystal payload values
  vector<float> prevValuesCrys(8736);
  if (readPrevCrysPayload) {
    DBObjPtr<Belle2::ECLCrystalCalib> customPrevCrystalTimeObject("ECLCrystalTimeOffsetPreviousValues");
    //..Get vectors of values from the payloads
    prevValuesCrys = customPrevCrystalTimeObject->getCalibVector();

    //..Print out a few values for quality control
    B2INFO("Previous values read from database.  Write out for their values for comparison against those from tcol");
    for (int ic = 0; ic < 8736; ic += 500) {
      B2INFO("ts custom previous payload: cellID " << ic + 1 << " " << prevValuesCrys[ic]);
    }
  }


  //..Read bhabha payloads from database
  B2INFO("Reading payloads: ECLCrystalTimeOffsetBhabha");
  DBObjPtr<Belle2::ECLCrystalCalib> crystalBhabhaTimeObject("ECLCrystalTimeOffsetBhabha");

  //..Get vectors of values from the payloads
  vector<float> currentBhabhaValuesCrys = crystalBhabhaTimeObject->getCalibVector();
  vector<float> currentBhabhaUncCrys = crystalBhabhaTimeObject->getCalibUncVector();

  //..Print out a few values for quality control
  for (int ic = 0; ic < 8736; ic += 500) {
    B2INFO("ts bhabha: cellID " << ic + 1 << " " << currentBhabhaValuesCrys[ic] << " +/- " << currentBhabhaUncCrys[ic]);
  }


  //------------------------------------------------------------------------
  //..Get the reference crystal information
  auto refCrysIDzeroingCrate = getObjectPtr<TH1F>("refCrysIDzeroingCrate");

  // crystal index for the crystals (one per crate) that is used as the reference crystal.  This one has
  //    ts defined as zero.  The crystal id runs 1...8636, not starting at 0.
  B2INFO("Extract reference crystals from collector histogram.");
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
  //    one reference crystal for each crate as defined by the payload/database.
  //    also fill the final vector that maps the crate id to the reference crystal id
  vector <short> crateIDsNumRefCrystalsUntested(52, 0);
  vector <short> crystalIDReferenceForZeroTs(52, 0);

  for (long unsigned int crysRefCounter = 0; crysRefCounter < crystalIDreferenceUntested.size(); crysRefCounter++) {
    int crys_id = crystalIDreferenceUntested[crysRefCounter] ;
    int crate_id_from_crystal = crystalMapper->getCrateID(crys_id);
    crateIDsNumRefCrystalsUntested[crate_id_from_crystal - 1]++;
    crystalIDReferenceForZeroTs[crate_id_from_crystal - 1] = crys_id;
  }
  B2INFO("crystalIDReferenceForZeroTs = {");
  for (int crateTest = 0; crateTest < 52 - 1; crateTest++) {
    B2INFO(crystalIDReferenceForZeroTs[crateTest] << ",");
  }
  B2INFO(crystalIDReferenceForZeroTs[52 - 1] << "}");


  // Make sure that there is only one reference crystal per crate as defined by the payload/database
  for (int crateTest = 0; crateTest < 52; crateTest++) {
    if (crateIDsNumRefCrystalsUntested[crateTest] != 1) {
      B2FATAL("Crate " << crateTest + 1 << " (base 1) has " << crateIDsNumRefCrystalsUntested[crateTest] << " reference crystals");
      return c_Failure;
    }
  }
  B2INFO("All reference crystals are reasonably mapped one crystal to one crate for all crates");



  B2INFO("Extract reference crystals from algorithm steering script if provided.  If user inputs custom values via steering script for this algorithm, they are only applied after all the tests are performed on the values from the histogram and override the histogram valuees.  User can adjust just a single crystal if desired.  Use -1 to indicate that a crystal is not to be modified.  Position of crystal in list determines the crate to which the crystal is meant to be associated.");

  /* Test if the user wants to modify the reference crystals.  This will probably be done very rarely,
     perhaps less than once per year.  If the user does want to change one or more reference
     crystals then perform the checks again to make sure that there is still just one reference crystal
     per crate after the modifications to the payload values with the user values.*/
  bool userSetRefCrysPerCrate = false ;
  for (int crateTest = 0; crateTest < 52; crateTest++) {
    if (refCrysPerCrate[crateTest] != -1) {
      crystalIDReferenceForZeroTs[crateTest] = refCrysPerCrate[crateTest] ;
      B2INFO("Crate " << crateTest + 1 << " (base 1) new reference crystal = " << crystalIDReferenceForZeroTs[crateTest]);
      userSetRefCrysPerCrate = true ;
    }
  }
  if (userSetRefCrysPerCrate) {
    B2INFO("User changed reference crystals via steering script");

    // Validate crystals per crate again with the new user set values
    fill(crateIDsNumRefCrystalsUntested.begin(), crateIDsNumRefCrystalsUntested.end(), 0);
    for (long unsigned int crysRefCounter = 0; crysRefCounter < 52; crysRefCounter++) {
      int crys_id = crystalIDReferenceForZeroTs[crysRefCounter] ;
      int crate_id_from_crystal = crystalMapper->getCrateID(crys_id);
      crateIDsNumRefCrystalsUntested[crate_id_from_crystal - 1]++;
    }
    for (int crateTest = 0; crateTest < 52; crateTest++) {
      if (crateIDsNumRefCrystalsUntested[crateTest] != 1) {
        B2FATAL("Crate " << crateTest + 1 << " (base 1) has " << crateIDsNumRefCrystalsUntested[crateTest] << " reference crystals");
        return c_Failure;
      }
    }
    B2INFO("All reference crystals are reasonably mapped one crystal to one crate for all crates after changes made by user steering script.");

    // Save the information to the payload if there is at least one crate
    //    reference crystal that has been modified by the user steering file
    ECLReferenceCrystalPerCrateCalib* refCrystalsCalib = new ECLReferenceCrystalPerCrateCalib();
    refCrystalsCalib->setCalibVector(crystalIDReferenceForZeroTs);
    saveCalibration(refCrystalsCalib, "ECLReferenceCrystalPerCrateCalib");
    B2INFO("Created reference crystal per crate payload: ECLReferenceCrystalPerCrateCalib");
  } else {
    B2INFO("User did not change reference crystals via steering script");
  }


  //------------------------------------------------------------------------
  //..Start looking at timing information

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

  /* 1/(4fRF) = 0.4913 ns/clock tick, where fRF is the accelerator RF frequency, fRF=508.889 MHz.
     Same for all crystals.  Proper accurate value*/
  const double TICKS_TO_NS = 1.0 / (4.0 * EclConfiguration::m_rf) * 1e3;

  // The ts and tcrate database values are filled once per tcol instance so count the number of times that the database values
  //    were summed together by the histogram merging process and extract out the original values again.
  auto databaseCounter = getObjectPtr<TH1I>("databaseCounter");
  float numTimesFilled = databaseCounter->GetBinContent(1);
  B2INFO("Number of times database histograms were merged = " << numTimesFilled);


  auto TsDatabase = getObjectPtr<TH1F>("TsDatabase");
  auto TsDatabaseUnc = getObjectPtr<TH1F>("TsDatabaseUnc");
  for (int i = 1; i <= 8736; i++) {
    t_offsets.push_back(TsDatabase->GetBinContent(i) / numTimesFilled);
    t_offsets_prev.push_back(TsDatabase->GetBinContent(i) / numTimesFilled);

    B2INFO("t_offsets_prev (last iter) at crysID " << i << " = " << t_offsets_prev[i - 1]);

    t_offsets_unc.push_back(TsDatabaseUnc->GetBinContent(i) / numTimesFilled);
  }


  /* CRYSTAL CORRECTIONS */

  /* Make a 1D histogram of the number of hits per crystal.  This will help with the validations
     to make sure that all the crystals had enough hits and to look for problems.*/
  TH1D* h_crysHits = TimevsCrysPrevCrateCalibNoCrystCalib->ProjectionX("h_crysHits");
  h_crysHits->SetTitle("Hits per crystal;Crystal id");

  histfile->WriteTObject(h_crysHits, "h_crysHits");


  // Loop over all the crystals for doing the crystal calibation
  for (int crys_id = cellIDLo; crys_id <= cellIDHi; crys_id++) {
    crystalCalibSaved = 0;

    double database_mean = 0;
    double database_mean_unc = 0;

    B2INFO("Crystal id = " << crys_id);


    vector<bool> ts_new_was_set(8736, false);


    /* Determining which bins to mask out for mean calculation
    */

    TH1D* h_time = TimevsCrysPrevCrateCalibNoCrystCalib->ProjectionY((string("h_time_psi__") + to_string(crys_id)).c_str(),
                   crys_id, crys_id);
    TH1D* h_timeMask = (TH1D*)h_time->Clone();
    TH1D* h_timeMasked = (TH1D*)h_time->Clone((string("h_time_psi_masked__") + to_string(crys_id)).c_str());
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
        (fit_mean_unc > 0.09)      ||
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
      if (fit_mean_unc > 0.09)       B2INFO("fit mean unc too large");
      if (fit_sigma < 0.1)           B2INFO("fit sigma too small");

    } else {
      good_fit = true;
    }



    // Set the tree_crystal values - ignore fit values  !!!!!!!!!!!!!!!!
    sigma = default_sigma;


    int numEntries = h_time->GetEntries();
    /* If number of entries in histogram is greater than X then use the statistical information from the data otherwise
       leave crystal uncalibrated.  Histograms are still shown though.  ALSO require the that fits are good.*/
    if ((numEntries >= minNumEntries)  &&  good_fit) {
      crystalCalibSaved = 1;
      database_mean = fit_mean;
      database_mean_unc = fit_mean_unc;
    } else {
      database_mean = default_mean;
      database_mean_unc = -fabs(default_mean_unc);
    }

    if (numEntries < minNumEntries)   B2INFO("Number of entries less than minimum");
    if (numEntries == 0)   B2INFO("Number of entries == 0");


    tree_cid  = crys_id;

    // For the database, convert back from ns to ADC ticks.
    t_offsets[crys_id - 1] = database_mean / TICKS_TO_NS;
    t_offsets_unc[crys_id - 1] = database_mean_unc / TICKS_TO_NS;


    histfile->WriteTObject(h_time, (string("h_time_psi") + to_string(crys_id)).c_str());
    histfile->WriteTObject(h_timeMasked, (string("h_time_psi_masked") + to_string(crys_id)).c_str());

    mean = database_mean;
    mean_unc = database_mean_unc;

    tsPrev = t_offsets_prev[crys_id - 1] * TICKS_TO_NS;

    delete gaus;
    tree_crystal->Fill();
  }


  // Shift the crystal time calibration constants by the reference crystal calibration constant values
  if (cellIDLo <= cellIDHi) {
    vector <double> tsRefCID ;
    B2INFO("crystal times before shift");
    for (int crate_id = 1; crate_id <= 52; crate_id++) {
      tsRefCID.push_back(t_offsets[ crystalIDReferenceForZeroTs[crate_id - 1] - 1 ]);
      B2INFO("crystal time [crystal = " << crystalIDReferenceForZeroTs[crate_id - 1] << ", crate = " << crate_id  << " (base 1)] = " <<
             t_offsets[ crystalIDReferenceForZeroTs[crate_id - 1] - 1 ] << " ticks");
    }

    B2INFO("crystal times after shift wrt reference crystal");
    for (int crys_id = 1; crys_id <= 8736; crys_id++) {
      int crate_id_from_crystal = crystalMapper->getCrateID(crys_id);
      B2INFO("crystal time before shift [crystal = " << crys_id << ", crate = " << crate_id_from_crystal  << " (base 1)] = " <<
             t_offsets[crys_id - 1] << " +- " << t_offsets_unc[crys_id - 1] << " ticks");

      /* Shift the crystal time constant by that of the reference crystal, but only if
         there were values to shift.  If there were no entries, ts=0 and ts_unc=0, which
         are special values so do not shift these crystals. */
      if (t_offsets[crys_id - 1] == 0  && t_offsets_unc[crys_id - 1] == 0) {
        B2INFO("crystal time after shift [crystal = " << crys_id << ", crate = " << crate_id_from_crystal  << " (base 1)] = " <<
               t_offsets[crys_id - 1] << " ticks.  No change because ts=0 and ts_unc=0 (no entries).");
      } else {
        t_offsets[crys_id - 1] = t_offsets[crys_id - 1] - tsRefCID[crate_id_from_crystal - 1];
        B2INFO("crystal time after shift [crystal = " << crys_id << ", crate = " << crate_id_from_crystal  << " (base 1)] = " <<
               t_offsets[crys_id - 1] << " ticks");
      }


      // Fill histograms with the difference in the ts values between iterations
      double tsDiff_ns = (t_offsets[crys_id - 1] - t_offsets_prev[crys_id - 1]) * TICKS_TO_NS;
      double tsDiffBhabha_ns = -999;
      if (readPrevCrysPayload) {
        tsDiffBhabha_ns = (t_offsets[crys_id - 1] - currentBhabhaValuesCrys[crys_id - 1]) * TICKS_TO_NS;
      }

      B2INFO("Crystal " << crys_id << ": ts new bhabha - old merged = (" <<
             t_offsets[crys_id - 1]  << " - " << t_offsets_prev[crys_id - 1]  <<
             ") ticks * " << TICKS_TO_NS << " ns/tick = " << tsDiff_ns << " ns");
      B2INFO("Crystal " << crys_id << ": ts new bhabha - old bhabha = (" <<
             t_offsets[crys_id - 1]  << " - " << currentBhabhaValuesCrys[crys_id - 1]  <<
             ") ticks * " << TICKS_TO_NS << " ns/tick = " << tsDiffBhabha_ns << " ns");

      tsNew_MINUS_tsOld__cid->SetBinContent(crys_id, tsDiff_ns);
      tsNew_MINUS_tsOld__cid->SetBinError(crys_id, 0);
      tsNew_MINUS_tsOld__cid->ResetStats();

      tsNew_MINUS_tsOld->Fill(tsDiff_ns);


      tsNew_MINUS_tsOldBhabha__cid->SetBinContent(crys_id, tsDiffBhabha_ns);
      tsNew_MINUS_tsOldBhabha__cid->SetBinError(crys_id, 0);
      tsNew_MINUS_tsOldBhabha__cid->ResetStats();

      tsNew_MINUS_tsOldBhabha->Fill(tsDiffBhabha_ns);



      /* Fill histograms with the difference in the ts values from this iteration
         and the previous values read in from the payload. */
      double tsDiffCustomOld_ns = -999;
      if (readPrevCrysPayload) {
        tsDiffCustomOld_ns = (t_offsets[crys_id - 1] - prevValuesCrys[crys_id - 1]) * TICKS_TO_NS;
        B2INFO("Crystal " << crys_id << ": ts new bhabha - 'before 1st iter' merged = (" <<
               t_offsets[crys_id - 1]  << " - " << prevValuesCrys[crys_id - 1]  <<
               ") ticks * " << TICKS_TO_NS << " ns/tick = " << tsDiffCustomOld_ns << " ns");
      }
      tsNew_MINUS_tsCustomPrev__cid->SetBinContent(crys_id, tsDiffCustomOld_ns);
      tsNew_MINUS_tsCustomPrev__cid->SetBinError(crys_id, 0);
      tsNew_MINUS_tsCustomPrev__cid->ResetStats();

      tsNew_MINUS_tsCustomPrev->Fill(tsDiffCustomOld_ns);

    }

    // Save the histograms to the output root file
    histfile->WriteTObject(tsNew_MINUS_tsOld__cid.get(), "tsNew_MINUS_tsOld__cid");
    histfile->WriteTObject(tsNew_MINUS_tsOld.get(), "tsNew_MINUS_tsOld");

    histfile->WriteTObject(tsNew_MINUS_tsCustomPrev__cid.get(), "tsNew_MINUS_tsCustomPrev__cid");
    histfile->WriteTObject(tsNew_MINUS_tsCustomPrev.get(), "tsNew_MINUS_tsCustomPrev");

    histfile->WriteTObject(tsNew_MINUS_tsOldBhabha__cid.get(), "tsNew_MINUS_tsOldBhabha__cid");
    histfile->WriteTObject(tsNew_MINUS_tsOldBhabha.get(), "tsNew_MINUS_tsOldBhabha");
  }


  //..Store previous crystal calibration constants to payload under different
  // names so that they can be read in for comparison later.  These are temporary
  // payloads that are only used for plotting purposes while running the calibration
  // and does not need to be added to any global tag.
  if (savePrevCrysPayload) {
    ECLCrystalCalib* crysTCalib_prev = new ECLCrystalCalib();
    crysTCalib_prev->setCalibVector(currentValuesCrys, currentUncCrys);

    ECLCrystalCalib* crysBhabhaTCalib_prev = new ECLCrystalCalib();
    crysBhabhaTCalib_prev->setCalibVector(currentBhabhaValuesCrys, currentBhabhaUncCrys);


    // Save the information to the payload if there is at least one crystal
    //    begin calibrated.
    if (cellIDLo <= cellIDHi) {
      saveCalibration(crysTCalib_prev, "ECLCrystalTimeOffsetPreviousValues");
      B2INFO("Previous overall crystal payload made");

      saveCalibration(crysBhabhaTCalib_prev, "ECLCrystalTimeOffsetBhabhaPreviousValues");
      B2INFO("Previous bhabha crystal payload made");
    }
  }


  ECLCrystalCalib* BhabhaTCalib = new ECLCrystalCalib();
  BhabhaTCalib->setCalibVector(t_offsets, t_offsets_unc);

  // Save the information to the payload if there is at least one crystal
  //    begin calibrated.
  if (cellIDLo <= cellIDHi) {
    saveCalibration(BhabhaTCalib, "ECLCrystalTimeOffset");
    saveCalibration(BhabhaTCalib, "ECLCrystalTimeOffsetBhabha");
    B2DEBUG(22, "crystal payload made");
  }


  B2DEBUG(22, "end of crystal start of crate corrections .....");


  //==============================================================
  /* CRATE CORRECTIONS */

  hist_tmin = TimevsCrateNoCrateCalibPrevCrystCalib->GetYaxis()->GetXmin();
  hist_tmax = TimevsCrateNoCrateCalibPrevCrystCalib->GetYaxis()->GetXmax();

  B2DEBUG(22, "Found min/max of X axis of TimevsCrateNoCrateCalibPrevCrystCalib");

  // Vector of time offsets to be saved in the database.

  auto TcrateDatabase = getObjectPtr<TH1F>("TcrateDatabase");


  B2DEBUG(22, "Retrieved Ts and Tcrate histograms from tcol root file");


  vector<float> tcrate_mean_new(52, 0.0);
  vector<float> tcrate_mean_unc_new(52, 0.0);
  vector<float> tcrate_sigma_new(52, 0.0);
  vector<float> tcrate_mean_prev(52, 0.0);
  // vector<float> tcrate_sigma_prev(52, 0.0); // currently not used
  vector<bool> tcrate_new_was_set(52, false);

  B2DEBUG(22, "crate vectors set");



  // Crate time calibration constants are saved per crystal so read them per crystal
  //    and save as one entry per crate in the array
  for (int crys_id = 1; crys_id <= 8736; crys_id++) {
    int crate_id_from_crystal = crystalMapper->getCrateID(crys_id);
    tcrate_mean_prev[crate_id_from_crystal - 1] = TcrateDatabase->GetBinContent(crys_id) / numTimesFilled;
  }


  B2INFO("Print out previous crate time calibration constants");
  for (int crate_id = 1; crate_id <= 52; crate_id++) {
    B2INFO("tcrate_mean_prev[crate " << crate_id << " (base 1)] = " << tcrate_mean_prev[crate_id - 1]);
  }

  for (int crate_id = crateIDLo; crate_id <= crateIDHi; crate_id++) {

    B2DEBUG(22, "Start of crate id = " << crate_id);

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
                B2DEBUG(22, "Setting bin " << nonRebinnedBinNumber << " from " << h_time_crate_masked->GetBinContent(
                          nonRebinnedBinNumber) << " to 0");
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



    B2DEBUG(22, "crate loop - projected h_time_psi_crate");


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

    B2DEBUG(22, "Crate id = " << crate_id << " with crate mean = " << default_mean_crate << " +- " << fit_mean_crate_unc);

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


    histfile->WriteTObject(h_time_crate, (string("h_time_psi_crate") + to_string(crate_id)).c_str());
    histfile->WriteTObject(h_time_crate_masked, (string("h_time_psi_crate_masked") + to_string(crate_id)).c_str());
    histfile->WriteTObject(h_time_crate_rebin, (string("h_time_psi_crate_rebinned") + to_string(crate_id)).c_str());

    delete gaus;
  }

  B2DEBUG(22, "crate histograms made");


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


  // Fill histograms with the difference in the tcrate values
  if (crateIDLo <= crateIDHi) {
    for (int crate_id = crateIDLo; crate_id <= crateIDHi; crate_id++) {
      double tCrateDiff_ns = (tcrate_mean_new[crate_id - 1] - tcrate_mean_prev[crate_id - 1]) * TICKS_TO_NS;
      B2INFO("Crate " << crate_id << ": tcrate new - old = " << tCrateDiff_ns << " ns");
      tcrateNew_MINUS_tcrateOld__crateID->SetBinContent(crate_id, tCrateDiff_ns);
      tcrateNew_MINUS_tcrateOld__crateID->SetBinError(crate_id, 0);
      tcrateNew_MINUS_tcrateOld__crateID->ResetStats();

      tcrateNew_MINUS_tcrateOld->Fill(tCrateDiff_ns);
    }

    // Save the histograms to the output root file
    histfile->WriteTObject(tcrateNew_MINUS_tcrateOld__crateID.get(), "tcrateNew_MINUS_tcrateOld__crateID");
    histfile->WriteTObject(tcrateNew_MINUS_tcrateOld.get(), "tcrateNew_MINUS_tcrateOld");
  }


  ECLCrystalCalib* BhabhaTCrateCalib = new ECLCrystalCalib();
  BhabhaTCrateCalib->setCalibVector(t_offsets_crate, t_offsets_crate_unc);


  // Save the information to the payload if there is at least one crate
  //    begin calibrated.
  if (crateIDLo <= crateIDHi) {
    saveCalibration(BhabhaTCrateCalib, "ECLCrateTimeOffset");
    B2DEBUG(22, "crate payload made");
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

  B2DEBUG(22, "end of crate corrections .....");

  tree_crystal->Write();
  tree_crate->Write();

  histfile->Close();

  B2INFO("Finished talgorithm");
  return c_OK;
}

