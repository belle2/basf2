#include <ecl/calibration/eclTimeShiftsAlgorithm.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dbobjects/ECLReferenceCrystalPerCrateCalib.h>
#include <ecl/digitization/EclConfiguration.h>
#include <ecl/utility/ECLChannelMapper.h>
#include "TH1F.h"
#include "TString.h"
#include "TFile.h"
#include "TDirectory.h"
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TLatex.h>
#include <sstream>
#include <iomanip>
#include <TLatex.h>

using namespace std;
using namespace Belle2;
using namespace ECL;
using namespace Calibration;

/**-----------------------------------------------------------------------------------------------*/
//eclTimeShiftsAlgorithm::eclTimeShiftsAlgorithm(): CalibrationAlgorithm("DummyCollector"),
eclTimeShiftsAlgorithm::eclTimeShiftsAlgorithm():
  CalibrationAlgorithm("eclTimeShiftsPlottingCollector"),
  debugFilenameBase("ECL_time_offsets"),
  timeShiftForPlotStyle{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  crysCrateShift_min(-20),
  crysCrateShift_max(20),
  m_ECLCrystalTimeOffset("ECLCrystalTimeOffset"),
  m_ECLCrateTimeOffset("ECLCrateTimeOffset"),
  m_refCrysIDzeroingCrate("ECLReferenceCrystalPerCrateCalib"),
  algorithmReadPayloads(false),
  forcePayloadIOVnotOpenEndedAndSequentialRevision(false)//,
{
  setDescription(
    "Perform time calibration of ecl crystals by combining previous values from the DB for different calibrations."
  );
}

CalibrationAlgorithm::EResult eclTimeShiftsAlgorithm::calibrate()
{
  /** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();

  B2INFO("eclTimeShiftsAlgorithm parameters:");
  B2INFO("debugFilenameBase = " << debugFilenameBase);
  B2INFO("algorithmReadPayloads = " << algorithmReadPayloads);
  B2INFO("forcePayloadIOVnotOpenEndedAndSequentialRevision = " << forcePayloadIOVnotOpenEndedAndSequentialRevision);
  B2INFO("timeShiftForPlotStyle = {");
  for (int crateTest = 0; crateTest < 51; crateTest++) {
    B2INFO(timeShiftForPlotStyle[crateTest] << ",");
  }
  B2INFO(timeShiftForPlotStyle[51] << "}");


  //------------------------------------------------------------------------
  /* Conversion coefficient from ADC ticks to nanoseconds
     1/(4fRF) = 0.4913 ns/clock tick, where fRF is the accelerator RF frequency,
     fRF=508.889 MHz. Same for all crystals.  Proper accurate value */
  const double TICKS_TO_NS = 1.0 / (4.0 * EclConfiguration::m_rf) * 1e3;


  //------------------------------------------------------------------------
  /* Set up variables for storing timing information and cutting on
     timing quality */

  vector< vector<double> > allCrates_crate_times ;
  vector< vector<double> > allCrates_run_nums ;  // not an integer for plotting purposes
  vector< vector<double> > allCrates_time_unc ;
  vector< vector<double> > allCrates_crystalCrate_times ;
  vector< vector<double> > allCrates_crystalCrate_times_unc ;

  vector<int> allRunNums;

  vector<double> mean_crystalCrate_time_ns(m_numCrates, 0);

  vector< double > blank_vector = {} ;
  vector< int > blank_vector_int = {} ;
  for (int temp_crate_id = 0; temp_crate_id < m_numCrates; temp_crate_id++) {
    allCrates_crate_times.push_back(blank_vector) ;
    allCrates_run_nums.push_back(blank_vector) ;
    allCrates_time_unc.push_back(blank_vector) ;
    allCrates_crystalCrate_times.push_back(blank_vector) ;
    allCrates_crystalCrate_times_unc.push_back(blank_vector) ;
  }
  // This results in : allCrates_crate_time[index for crate number][index for run number]



  //------------------------------------------------------------------------
  /* Extract the crystal and crate calibration constant information from the
     tree as extracted by the collector. */

  // Pulling in data from collector output. It now returns shared_ptr<T> so the underlying pointer
  // will delete itself automatically at the end of this scope unless you do something
  auto tree_perCrys = getObjectPtr<TTree>("tree_perCrystal");
  if (!tree_perCrys) {
    B2ERROR("Tree of calibration constants does not exist.");
    return c_Failure;
  }
  B2INFO("Number of Entries in tree_perCrystal was " << tree_perCrys->GetEntries());
  B2INFO("Number of Entries in tree_perCrystal / 8736 = " << float(tree_perCrys->GetEntries()) / 8736.0);


  // Define the variables to be read in from the tree
  tree_perCrys->SetBranchAddress("run", &m_run_perCrystal);
  tree_perCrys->SetBranchAddress("exp", &m_exp_perCrystal);
  tree_perCrys->SetBranchAddress("crystalID", &m_crystalID);
  tree_perCrys->SetBranchAddress("crateID", &m_crateID);
  tree_perCrys->SetBranchAddress("crateTimeConst", &m_crateTimeConst);
  tree_perCrys->SetBranchAddress("crateTimeUnc", &m_crateTimeUnc);
  tree_perCrys->SetBranchAddress("crystalTimeConst", &m_crystalTimeConst);
  tree_perCrys->SetBranchAddress("crystalTimeUnc", &m_crystalTimeUnc);
  tree_perCrys->SetBranchAddress("refCrystalID", &m_refCrystalID);


  int referenceRunNum = -1;
  int referenceExpNum = -1;
  //int numAnalysedRuns = 0 ;
  int previousRunNumTree = -1 ;
  vector<double> Crate_time_ns_tree(m_numCrates) ;
  vector<double> Crate_time_tick_tree(m_numCrates) ;
  vector<double> Crate_time_unc_ns_tree(m_numCrates) ;
  vector<double> crystalCrate_time_ns_tree(m_numCrates);
  vector<double> crystalCrate_time_unc_ns_tree(m_numCrates);


  Int_t numEntriesCrysTree = (Int_t)tree_perCrys->GetEntries();

  // Loop through the entire tree
  for (Int_t tree_crys_i = 0; tree_crys_i < numEntriesCrysTree; tree_crys_i++) {
    for (Int_t tree_crys_j = 0; tree_crys_j < m_numCrystals; tree_crys_j++) {
      tree_perCrys->GetEntry(tree_crys_i);
      //B2INFO("tree_crys_i, tree_crys_j = " << tree_crys_i << ", " << tree_crys_j);
      if (tree_crys_j != m_numCrystals - 1) {
        tree_crys_i++;
      }

      // Make sure that all the information read in for 8736 crystals are all from one (exp,run).
      if (tree_crys_j == 0) {
        referenceExpNum = m_exp_perCrystal;
        referenceRunNum = m_run_perCrystal;
        B2INFO("Looking at exp,run " << m_exp_perCrystal << ", " << m_run_perCrystal);
      }
      if ((m_exp_perCrystal != referenceExpNum)      or
          (m_run_perCrystal != referenceRunNum)      or
          (m_run_perCrystal == previousRunNumTree)) {

        B2ERROR("m_exp_perCrystal, referenceExpNum" << m_exp_perCrystal << ", " << referenceExpNum);
        B2ERROR("m_run_perCrystal, referenceRunNum" << m_run_perCrystal << ", " << referenceRunNum);
        B2ERROR("m_run_perCrystal, previousRunNumTree" << m_run_perCrystal << ", " << previousRunNumTree);
        B2ERROR("Exp/run number problem");
        return c_Failure;
      }


      int crateID_temp = m_crateID;
      Crate_time_ns_tree[crateID_temp - 1] = m_crateTimeConst * TICKS_TO_NS ;
      Crate_time_tick_tree[crateID_temp - 1] = m_crateTimeConst ;
      Crate_time_unc_ns_tree[crateID_temp - 1] = m_crateTimeUnc * TICKS_TO_NS ;

      if (m_crystalID == m_refCrystalID) {
        B2INFO("m_exp_perCrystal, m_run_perCrystal, cell ID (0..8735), m_crateID, m_crateTimeConst = " << m_exp_perCrystal << ", " <<
               m_run_perCrystal << ", " << tree_crys_j << ", " << m_crateID << ", " << m_crateTimeConst << " ns") ;
        crystalCrate_time_ns_tree[crateID_temp - 1] = (m_crystalTimeConst + m_crateTimeConst) * TICKS_TO_NS;

        crystalCrate_time_unc_ns_tree[crateID_temp - 1] = TICKS_TO_NS * sqrt(
                                                            (m_crateTimeUnc * m_crateTimeUnc) +
                                                            (m_crystalTimeUnc * m_crystalTimeUnc)) ;
      } else if (tree_crys_j == 0 || tree_crys_j == 8735) {
        B2INFO("m_exp_perCrystal, m_run_perCrystal, cell ID (0..8735), m_crateID, m_crateTimeConst = " << m_exp_perCrystal << ", " <<
               m_run_perCrystal << ", " << tree_crys_j << ", " << m_crateID << ", " << m_crateTimeConst << " ns") ;
      } else {
        B2DEBUG(10, "m_exp_perCrystal, m_run_perCrystal, cell ID (0..8735), m_crateID, m_crateTimeConst = " << m_exp_perCrystal << ", " <<
                m_run_perCrystal << ", " << tree_crys_j << ", " << m_crateID << ", " << m_crateTimeConst << " ns") ;
        //B2INFO("m_exp_perCrystal, m_run_perCrystal, cell ID (0..8735), m_crateID, m_crateTimeConst = " << m_exp_perCrystal << ", " << m_run_perCrystal << ", " << tree_crys_j << ", " << m_crateID << ", " << m_crateTimeConst << " ns") ;
      }

    }

    //------------------------------------------------------------------------
    /** Apply the cuts to the values extracted from the tree */

    bool savedThisRunNum = false;
    for (int iCrate = 0; iCrate < m_numCrates; iCrate++) {
      double tcrate = Crate_time_ns_tree[iCrate] ;
      double tcrate_unc = Crate_time_unc_ns_tree[iCrate];
      double tcrystalCrate = crystalCrate_time_ns_tree[iCrate];
      double tcrystalCrate_unc = crystalCrate_time_unc_ns_tree[iCrate];

      if ((tcrate < m_tcrate_max_cut) &&
          (tcrate > m_tcrate_min_cut) &&
          (fabs(tcrate_unc) > m_tcrate_unc_min_cut) &&
          (fabs(tcrate_unc) < m_tcrate_unc_max_cut)) {
        allCrates_crate_times[iCrate].push_back(tcrate) ;
        allCrates_run_nums[iCrate].push_back(m_run_perCrystal) ;
        allCrates_time_unc[iCrate].push_back(tcrate_unc) ;
        allCrates_crystalCrate_times[iCrate].push_back(tcrystalCrate) ;
        allCrates_crystalCrate_times_unc[iCrate].push_back(tcrystalCrate_unc) ;

        mean_crystalCrate_time_ns[iCrate] += tcrystalCrate ;

        if (!savedThisRunNum) {
          allRunNums.push_back(m_run_perCrystal);
          savedThisRunNum = true;
        }
      }
    }

    //------------------------------------------------------------------------
    /** Write out a few values for quality control purposes */
    for (int ic = 0; ic < m_numCrates; ic++) {
      B2INFO("Crate " << ic + 1 << ", t_crate = " << Crate_time_tick_tree[ic] << " ticks = "
             << Crate_time_ns_tree[ic] << " +- " << Crate_time_unc_ns_tree[ic]
             << " ns;    t crys+crate (no shifts) = " << crystalCrate_time_ns_tree[ic] << " +- "
             << crystalCrate_time_unc_ns_tree[ic] << " ns") ;
    }

    previousRunNumTree = m_run_perCrystal;

  }


  B2INFO("Finished reading tree calibration constants.  Now extracting here by stepping through runs.");





  //------------------------------------------------------------------------
  /** Determine the minimum and maximum run numbers for labelling purposes */


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
        (maxExpNum < expNumber)) {
      maxExpNum = expNumber;
      maxRunNum = runNumber;
    }
  }

  B2INFO("minExpNum = " << minExpNum) ;
  B2INFO("minRunNum = " << minRunNum) ;
  B2INFO("maxExpNum = " << maxExpNum) ;
  B2INFO("maxRunNum = " << maxRunNum) ;


  if (minExpNum != maxExpNum) {
    B2ERROR("The runs must all come from the same experiment");
    return c_Failure;
  }

  int experiment = minExpNum;

  // Need to load information about the event/run/experiment to get the right database information
  // Will be used for:
  // * ECLChannelMapper (to map crystal to crates)
  // * crystal payload updating for iterating crystal and crate fits
  int eventNumberForCrates = 1;
  int previousRevNum = -1 ;



  //------------------------------------------------------------------------
  //------------------------------------------------------------------------
  //------------------------------------------------------------------------
  //------------------------------------------------------------------------
  /* Extract out the time offset information from the database directly. */

  if (algorithmReadPayloads) {
    //------------------------------------------------------------------------
    // Get the input run list (should be only 1) for us to use to update the DBObjectPtrs
    auto runs = getRunList();
    /* Take the first run.  For the crystal cosmic calibrations, because of the crate
       calibrations, there is not a known correct run to use within the range. */
    ExpRun chosenRun = runs.front();
    B2INFO("merging using the ExpRun (" << chosenRun.second << "," << chosenRun.first << ")");
    // After here your DBObjPtrs are correct
    updateDBObjPtrs(1, chosenRun.second, chosenRun.first);

    //------------------------------------------------------------------------
    // Test the DBObjects we want to exist and fail if not all of them do.
    bool allObjectsFound = true;

    /** Test that the DBObjects are valid */
    // Check that the payloads we want to merge are sufficiently loaded
    if (!m_ECLCrystalTimeOffset) {
      allObjectsFound = false;
      B2ERROR("No valid DBObject found for 'ECLCrystalTimeOffset'");
    }

    // Check that the crate payload is loaded (used for transforming cosmic payload)
    if (!m_ECLCrateTimeOffset) {
      allObjectsFound = false;
      B2ERROR("No valid DBObject found for 'ECLCrateTimeOffset'");
    }

    if (!m_refCrysIDzeroingCrate) {
      allObjectsFound = false;
      B2ERROR("No valid DBObject found for 'refCrysIDzeroingCrate'");
    }


    if (allObjectsFound) {
      B2INFO("Valid objects found for 'ECLCrystalTimeOffset'");
      B2INFO("Valid object found for 'ECLCrateTimeOffset'");
      B2INFO("Valid object found for 'refCrysIDzeroingCrate'");
    } else {
      B2INFO("eclTimeShiftsAlgorithm: Exiting with failure.  Some missing valid objects.");
      return c_Failure;
    }


    //------------------------------------------------------------------------
    /** Get the vectors from the input payloads */
    vector<float> crystalCalib = m_ECLCrystalTimeOffset->getCalibVector();
    vector<float> crystalCalibUnc = m_ECLCrystalTimeOffset->getCalibUncVector();
    B2INFO("Loaded 'ECLCrystalTimeOffset' calibrations");

    vector<float> crateCalib = m_ECLCrateTimeOffset->getCalibVector();
    vector<float> crateCalibUnc = m_ECLCrateTimeOffset->getCalibUncVector();

    B2INFO("Loaded 'ECLCrateTimeOffset' calibration with default exp/run");

    B2INFO("eclTimeShiftsAlgorithm:: loaded ECLCrateTimeOffset from the database"
           << LogVar("IoV", m_ECLCrateTimeOffset.getIoV())
           << LogVar("Revision", m_ECLCrateTimeOffset.getRevision()));

    for (int cellID = 1; cellID <= m_numCrystals; cellID += 511) {
      B2INFO("crystalCalib = " << crystalCalib[cellID - 1]);
      B2INFO("crateCalib = " << crateCalib[cellID - 1]);
    }

    vector<short> refCrystals = m_refCrysIDzeroingCrate->getReferenceCrystals();
    for (int icrate = 0; icrate < m_numCrates; icrate++) {
      B2INFO("reference crystal for crate " << icrate + 1 << " = " << refCrystals[icrate]);
    }


    //------------------------------------------------------------------------
    /** Loop over all the experiments and runs and extract the crate times*/
    for (int run = minRunNum; run <= maxRunNum; run++) {
      B2INFO("---------") ;
      B2INFO("Looking at run " << run) ;

      vector<int>::iterator it = find(allRunNums.begin(), allRunNums.end(), run);
      if (it != allRunNums.end()) {
        int pos = it - allRunNums.begin() ;
        B2INFO("allRunNums[" << pos << "] = " << allRunNums[pos]);
        B2INFO("Run " << run << " already processed so skipping it.");
        continue;
      } else {
        B2INFO("New run.  Starting to extract information");
      }


      StoreObjPtr<EventMetaData> evtPtr;
      // simulate the initialize() phase where we can register objects in the DataStore
      DataStore::Instance().setInitializeActive(true);
      evtPtr.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);
      // now construct the event metadata
      evtPtr.construct(eventNumberForCrates, run, experiment);
      // and update the database contents
      DBStore& dbstore = DBStore::Instance();
      dbstore.update();
      // this is only needed it the payload might be intra-run dependent,
      // that is if it might change during one run as well
      dbstore.updateEvent();
      updateDBObjPtrs(eventNumberForCrates, run, experiment);


      //------------------------------------------------------------------------
      /** Set up ECL channel mapper and determine the payload IoV and Revision */

      shared_ptr< ECL::ECLChannelMapper > crystalMapper(new ECL::ECLChannelMapper()) ;
      crystalMapper->initFromDB();

      /** Populate database contents */
      B2INFO("eclTimeShiftsAlgorithm:: loaded ECLCrystalTimeOffset from the database"
             << LogVar("IoV", m_ECLCrystalTimeOffset.getIoV())
             << LogVar("Revision", m_ECLCrystalTimeOffset.getRevision()));
      B2INFO("eclTimeShiftsAlgorithm:: loaded ECLCrateTimeOffset from the database"
             << LogVar("IoV", m_ECLCrateTimeOffset.getIoV())
             << LogVar("Revision", m_ECLCrateTimeOffset.getRevision()));


      //------------------------------------------------------------------------
      /** If requested by the user, check that the payload revision is
          increasing by +1 and that the payload iov does not have a -1 as
          the high run number.  This should hopefully skip the runs that
          are gaps in the iov and use older payloads*/

      if (forcePayloadIOVnotOpenEndedAndSequentialRevision) {
        int revNumber = m_ECLCrateTimeOffset.getRevision();
        if ((m_ECLCrateTimeOffset.getIoV().getExperimentHigh() == -1 && m_ECLCrateTimeOffset.getIoV().getRunHigh() == -1)
            || (revNumber != previousRevNum + 1 && previousRevNum != -1)) {
          // skip this run because it is using an older payload - probably an iov hole
          B2INFO("Skipping run since payload has (*,*,-1,-1) and revision number is not incrementing");
          continue;
        } else {
          previousRevNum = revNumber;
        }
      }


      //------------------------------------------------------------------------
      /** Get the vectors from the input payload */

      vector<float> crystalTimeOffsetsCalib;
      vector<float> crystalTimeOffsetsCalibUnc;
      crystalTimeOffsetsCalib = m_ECLCrystalTimeOffset->getCalibVector();
      crystalTimeOffsetsCalibUnc = m_ECLCrystalTimeOffset->getCalibUncVector();

      vector<float> crateTimeOffsetsCalib;
      vector<float> crateTimeOffsetsCalibUnc;
      crateTimeOffsetsCalib = m_ECLCrateTimeOffset->getCalibVector();
      crateTimeOffsetsCalibUnc = m_ECLCrateTimeOffset->getCalibUncVector();

      //------------------------------------------------------------------------
      /**  Make a crate time offset vector with an entry per crate (instead of per crystal)
           and convert from ADC counts to ns. Also store the sum of the crystal and
           crate times */
      vector<double> Crate_time_ns(m_numCrates) ;
      vector<double> Crate_time_tick(m_numCrates) ;
      vector<double> Crate_time_unc_ns(m_numCrates) ;
      vector<double> crystalCrate_time_ns(m_numCrates);
      vector<double> crystalCrate_time_unc_ns(m_numCrates);

      for (int crysID = 1; crysID <= m_numCrystals; crysID++) {
        int crateID_temp = crystalMapper->getCrateID(crysID) ;
        Crate_time_ns[crateID_temp - 1] = crateTimeOffsetsCalib[crysID - 1] * TICKS_TO_NS ;
        Crate_time_tick[crateID_temp - 1] = crateTimeOffsetsCalib[crysID - 1] ;
        Crate_time_unc_ns[crateID_temp - 1] = crateTimeOffsetsCalibUnc[crysID - 1] * TICKS_TO_NS ;

        if (crysID == refCrystals[crateID_temp - 1]) {
          crystalCrate_time_ns[crateID_temp - 1] = (crystalTimeOffsetsCalib[crysID - 1] +
                                                    crateTimeOffsetsCalib[crysID - 1]) * TICKS_TO_NS;

          crystalCrate_time_unc_ns[crateID_temp - 1] = TICKS_TO_NS * sqrt(
                                                         (crateTimeOffsetsCalibUnc[crysID - 1] * crateTimeOffsetsCalibUnc[crysID - 1]) +
                                                         (crystalTimeOffsetsCalibUnc[crysID - 1] * crystalTimeOffsetsCalibUnc[crysID - 1])) ;
        }
      }


      for (int iCrate = 0; iCrate < m_numCrates; iCrate++) {
        double tcrate = Crate_time_ns[iCrate] ;
        double tcrate_unc = Crate_time_unc_ns[iCrate];
        double tcrystalCrate = crystalCrate_time_ns[iCrate];
        double tcrystalCrate_unc = crystalCrate_time_unc_ns[iCrate];

        if ((tcrate < m_tcrate_max_cut) &&
            (tcrate > m_tcrate_min_cut) &&
            (fabs(tcrate_unc) > m_tcrate_unc_min_cut) &&
            (fabs(tcrate_unc) < m_tcrate_unc_max_cut)) {
          allCrates_crate_times[iCrate].push_back(tcrate) ;
          allCrates_run_nums[iCrate].push_back(run) ;
          allCrates_time_unc[iCrate].push_back(tcrate_unc) ;
          allCrates_crystalCrate_times[iCrate].push_back(tcrystalCrate) ;
          allCrates_crystalCrate_times_unc[iCrate].push_back(tcrystalCrate_unc) ;

          mean_crystalCrate_time_ns[iCrate] += tcrystalCrate ;
        }
      }


      //------------------------------------------------------------------------
      /** Write out a few values for quality control purposes */
      for (int ic = 0; ic < m_numCrates; ic++) {
        B2INFO("Crate " << ic + 1 << ", t_crate = " << Crate_time_tick[ic] << " ticks = "
               << Crate_time_ns[ic] << " +- " << Crate_time_unc_ns[ic]
               << " ns;    t crys+crate (no shift) = " << crystalCrate_time_ns[ic] << " +- "
               << crystalCrate_time_unc_ns[ic] << " ns") ;
      }

      /* Shift the run number to the end of the iov so that we can skip runs
         that have the payload with the same revision number */
      int IOV_exp_high = m_ECLCrateTimeOffset.getIoV().getExperimentHigh() ;
      int IOV_run_high = m_ECLCrateTimeOffset.getIoV().getRunHigh() ;
      B2INFO("IOV_exp_high = " << IOV_exp_high);
      B2INFO("IOV_run_high = " << IOV_run_high);
      if (IOV_run_high == -1) {
        B2INFO("IOV_run_high is -1 so stop looping over all runs");
        break;
      } else {
        B2INFO("Set run number to higher iov run number");
        run = IOV_run_high;
      }
      B2INFO("now set run = " << run);
    }
  }




  //------------------------------------------------------------------------
  //------------------------------------------------------------------------
  //------------------------------------------------------------------------
  //------------------------------------------------------------------------
  /** Shift all the crystal+crate times by the mean time to naturally
      roughly centre all the crys+crate+shift times*/
  B2INFO("Shift all run crys+crate+off times.  Show the results for a subset of crates/runs:");
  for (int iCrate = 0; iCrate < m_numCrates; iCrate++) {
    double mean_time = mean_crystalCrate_time_ns[iCrate] / allCrates_crate_times[iCrate].size() ;
    B2INFO("Mean crys+crate times for all runs used as offset (crate " << iCrate + 1 << ") = " << mean_time);

    for (long unsigned int jRun = 0; jRun < allCrates_crate_times[iCrate].size(); jRun++) {
      allCrates_crystalCrate_times[iCrate][jRun] += -mean_time + timeShiftForPlotStyle[iCrate] ;
      if (jRun < 50 || iCrate == 1 || iCrate == 40 || iCrate == 51) {
        B2INFO("allCrates_crystalCrate_times(crate " << iCrate + 1 << ", run counter " << jRun + 1 << ", runNum " <<
               allCrates_run_nums[iCrate][jRun] << " | after shifting mean) = " <<
               allCrates_crystalCrate_times[iCrate][jRun]);
      }
    }
  }



  //------------------------------------------------------------------------
  //------------------------------------------------------------------------
  /** Now that the timing information has be read in, fill the crate
      time offsets and general time shifts into the histograms */
  TFile* tcratefile = 0;

  B2INFO("Debug output rootfile: " << debugFilenameBase);
  TString fname = debugFilenameBase;
  fname += ".root";
  tcratefile = new TFile(fname, "recreate");
  tcratefile->cd();
  B2INFO("Debugging histograms written to " << fname);


  TGraphErrors* g_tcrate_vs_runNum ;
  TGraphErrors* g_crateCrystalTime_vs_runNum ;
  TGraphErrors* g_crateCrystalTime_vs_runCounter ;


  TCanvas* c1 = new TCanvas("c1", "");
  for (int i = 0; i < m_numCrates; i++) {
    Double_t* single_crate_crate_times = &allCrates_crate_times[i][0] ;
    Double_t* single_crate_run_nums = &allCrates_run_nums[i][0] ;
    Double_t* single_crate_time_unc = &allCrates_time_unc[i][0] ;
    Double_t* single_crate_crystalCrate_times = &allCrates_crystalCrate_times[i][0] ;
    Double_t* single_crate_crystalCrate_times_unc = &allCrates_crystalCrate_times_unc[i][0] ;

    ostringstream ss;
    ss << setw(2) << setfill('0') << i + 1 ;
    string paddedCrateID(ss.str());

    // ----- crate time constants vs run number ------
    g_tcrate_vs_runNum = new TGraphErrors(allCrates_crate_times[i].size(), single_crate_run_nums,
                                          single_crate_crate_times, NULL, single_crate_time_unc) ;   // NULL for run number errors = 0 for all

    string tgraph_title = string("e") + to_string(minExpNum) + string("r") + to_string(minRunNum) +
                          string("-e") + to_string(maxExpNum) + string("r") + to_string(maxRunNum) ;
    string tgraph_name_short = "crateTimeVSrunNum_crate" ;
    tgraph_title = tgraph_title + string("_crate") + paddedCrateID ;
    tgraph_name_short = tgraph_name_short + paddedCrateID ;
    tgraph_title = tgraph_title + string(" (") + to_string(m_tcrate_min_cut) + string(" < tcrate < ") +
                   to_string(m_tcrate_max_cut) + string(" ns, ") + to_string(m_tcrate_unc_min_cut) +
                   string(" < tcrate unc. < ") + to_string(m_tcrate_unc_max_cut) + string(" ns cuts)")  ;

    g_tcrate_vs_runNum->SetName(tgraph_name_short.c_str()) ;
    g_tcrate_vs_runNum->SetTitle(tgraph_title.c_str()) ;
    g_tcrate_vs_runNum->GetXaxis()->SetTitle("Run number") ;
    g_tcrate_vs_runNum->GetYaxis()->SetTitle("Crate time [ns]") ;

    g_tcrate_vs_runNum->GetYaxis()->SetRangeUser(m_tcrate_min_cut, m_tcrate_max_cut) ;

    g_tcrate_vs_runNum->Draw("AP") ;
    g_tcrate_vs_runNum->SetMarkerSize(0.8) ;
    g_tcrate_vs_runNum->Draw("AP") ;

    TLatex* Leg1 = new TLatex();
    Leg1->SetNDC();
    Leg1->SetTextAlign(11);
    Leg1->SetTextFont(42);
    Leg1->SetTextSize(0.035);
    Leg1->SetTextColor(1);
    //string crateLabel = "Phase 2, all runs, Crate " ;
    //crateLabel += paddedCrateID ;
    //Leg1->DrawLatex( 0.18, 0.96, paddedCrateID.c_str() ) ;
    Leg1->AppendPad();

    g_tcrate_vs_runNum->Write() ;
    c1->SaveAs((tgraph_name_short + string(".pdf")).c_str()) ;



    // ----- crystal + crate time constants + offset vs run number ------

    // NULL for run number errors = 0 for all
    g_crateCrystalTime_vs_runNum = new TGraphErrors(allCrates_crystalCrate_times[i].size(), single_crate_run_nums,
                                                    single_crate_crystalCrate_times, NULL, single_crate_crystalCrate_times_unc) ;

    tgraph_title = string("e") + to_string(minExpNum) + string("r") + to_string(minRunNum) +
                   string("-e") + to_string(maxExpNum) + string("r") + to_string(maxRunNum) ;
    tgraph_name_short = "crystalCrateTimeVSrunNum_crate" ;
    tgraph_title = tgraph_title + string("_crate") + paddedCrateID ;
    tgraph_name_short = tgraph_name_short + paddedCrateID ;
    tgraph_title = tgraph_title + string(" (") + to_string(m_tcrate_min_cut) + string(" < tcrate < ") +
                   to_string(m_tcrate_max_cut) + string(" ns, ") + to_string(m_tcrate_unc_min_cut) +
                   string(" < tcrate unc. < ") + to_string(m_tcrate_unc_max_cut) + string(" ns cuts)")  ;


    g_crateCrystalTime_vs_runNum->SetName(tgraph_name_short.c_str()) ;
    g_crateCrystalTime_vs_runNum->SetTitle(tgraph_title.c_str()) ;
    g_crateCrystalTime_vs_runNum->GetXaxis()->SetTitle("Run number") ;
    g_crateCrystalTime_vs_runNum->GetYaxis()->SetTitle("Crate time + Crystal time + centring overall offset [ns]") ;

    g_crateCrystalTime_vs_runNum->GetYaxis()->SetRangeUser(crysCrateShift_min, crysCrateShift_max) ;

    g_crateCrystalTime_vs_runNum->Draw("AP") ;
    g_crateCrystalTime_vs_runNum->SetMarkerSize(0.8) ;
    g_crateCrystalTime_vs_runNum->Draw("AP") ;

    g_crateCrystalTime_vs_runNum->Write() ;
    c1->SaveAs((tgraph_name_short + string(".pdf")).c_str()) ;


    // ----- crystal + crate time constants + offset vs run counter------
    // This will remove gaps and ignore the actual run number

    /* Define a vector to store a renumbering of the run numbers, incrementing
       by +1 so that there are no gaps.  The runs are not in order so the
       run numbers&indices first have to be sorted before the "run counter"
       numbers can used.*/
    int numRunsWithCrateTimes = allCrates_crystalCrate_times[i].size();
    vector<Double_t> counterVec(numRunsWithCrateTimes);


    // Vector to store element
    // with respective present index
    vector<pair<int, double> > runNum_index_pairs;

    // Inserting element in pair vector
    // to keep track of previous indexes
    for (int pairIndex = 0; pairIndex < numRunsWithCrateTimes; pairIndex++) {
      runNum_index_pairs.push_back(make_pair(allCrates_run_nums[i][pairIndex], pairIndex));
    }

    B2INFO("Crate id = " << i + 1);
    B2INFO("Unsorted run numbers");
    for (int runCounter = 0; runCounter < numRunsWithCrateTimes; runCounter++) {
      B2INFO("Run number, run number vector index = " << runNum_index_pairs[runCounter].first << ", " <<
             runNum_index_pairs[runCounter].second);
    }

    // Sorting pair vector
    sort(runNum_index_pairs.begin(), runNum_index_pairs.end());

    // Fill the run counter vector
    for (int runCounter = 0; runCounter < numRunsWithCrateTimes; runCounter++) {
      counterVec[runNum_index_pairs[runCounter].second] = runCounter + 1;
    }

    B2INFO("Run numbers with index and times");
    for (int runCounter = 0; runCounter < numRunsWithCrateTimes; runCounter++) {
      int idx = (int) round(counterVec[runCounter]);
      B2INFO("Vector index, Run number, run number sorting order index, tcrystal+tcrate+shifts = " << runCounter << ", " <<
             allCrates_run_nums[i][runCounter] << ", " << idx << ", " << single_crate_crystalCrate_times[idx] << " ns");
    }


    if (numRunsWithCrateTimes > 0) {
      // NULL for run number errors = 0 for all
      //g_crateCrystalTime_vs_runCounter = new TGraphErrors(numRunsWithCrateTimes, counterVec,
      g_crateCrystalTime_vs_runCounter = new TGraphErrors(numRunsWithCrateTimes, &counterVec[0],
                                                          single_crate_crystalCrate_times, NULL, single_crate_crystalCrate_times_unc) ;

      tgraph_title = string("e") + to_string(minExpNum) + string("r") + to_string(minRunNum) +
                     string("-e") + to_string(maxExpNum) + string("r") + to_string(maxRunNum) ;
      tgraph_name_short = "crystalCrateTimeVSrunCounter_crate" ;
      tgraph_title = tgraph_title + string("_crate") + paddedCrateID ;
      tgraph_name_short = tgraph_name_short + paddedCrateID ;
      tgraph_title = tgraph_title + string(" (") + to_string(m_tcrate_min_cut) + string(" < tcrate < ") +
                     to_string(m_tcrate_max_cut) + string(" ns, ") + to_string(m_tcrate_unc_min_cut) +
                     string(" < tcrate unc. < ") + to_string(m_tcrate_unc_max_cut) + string(" ns cuts)")  ;


      g_crateCrystalTime_vs_runCounter->SetName(tgraph_name_short.c_str()) ;
      g_crateCrystalTime_vs_runCounter->SetTitle(tgraph_title.c_str()) ;
      g_crateCrystalTime_vs_runCounter->GetXaxis()->SetTitle("Run number") ;
      g_crateCrystalTime_vs_runCounter->GetYaxis()->SetTitle("Crate time + Crystal time + centring overall offset [ns]") ;

      g_crateCrystalTime_vs_runCounter->GetYaxis()->SetRangeUser(crysCrateShift_min, crysCrateShift_max) ;
      g_crateCrystalTime_vs_runCounter->GetXaxis()->SetRangeUser(0, numRunsWithCrateTimes + 1) ;

      g_crateCrystalTime_vs_runCounter->Draw("AP") ;
      g_crateCrystalTime_vs_runCounter->SetMarkerSize(0.8) ;
      g_crateCrystalTime_vs_runCounter->Draw("AP") ;

      g_crateCrystalTime_vs_runCounter->Write() ;
      c1->SaveAs((tgraph_name_short + string(".pdf")).c_str()) ;
    } else {
      B2INFO("Crate " << i + 1 << " has no entries that pass all the cuts so no crystalCrateTimeVSrunCounter_crate plot will be made.");
    }
  }




  /* Loop over all the runs and crates and let the user know when a crate time jump
     has occurred.  Jumps can be of various sizes so have different thresholds. */
  double smallThreshold = 1 ;    //ns
  double largeThreshold = 6.5 ;  //ns

  B2INFO("======================= Crate time jumps =========================");
  B2INFO("======================= Small threshold jumps ====================");
  B2INFO("Crate ID = 1..52");
  B2INFO("==================================================================");

  for (int i = 0; i < m_numCrates; i++) {
    int numRunsWithCrateTimes = allCrates_crystalCrate_times[i].size();
    for (int runCounter = 0; runCounter < numRunsWithCrateTimes - 1; runCounter++) {
      int run_i = allCrates_run_nums[i][runCounter] ;
      int run_f = allCrates_run_nums[i][runCounter + 1] ;
      double time_i = allCrates_crystalCrate_times[i][runCounter] ;
      double time_f = allCrates_crystalCrate_times[i][runCounter + 1] ;

      if (fabs(time_f - time_i) > smallThreshold) {
        B2INFO("Crate " << i + 1 << " has crate time jump > " << smallThreshold << " ns: t(run " << run_f << ") = " << time_f <<
               " ns - t(run " << run_i << ") = " << time_i << " ns = " << time_f - time_i);
      }
    }
  }


  B2INFO("~~~~~~~~~~~~~~~~~~~~~~~ Large threshold jumps ~~~~~~~~~~~~~~~~~~~~");
  B2INFO("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

  for (int i = 0; i < m_numCrates; i++) {
    int numRunsWithCrateTimes = allCrates_crystalCrate_times[i].size();
    for (int runCounter = 0; runCounter < numRunsWithCrateTimes - 1; runCounter++) {
      int run_i = allCrates_run_nums[i][runCounter] ;
      int run_f = allCrates_run_nums[i][runCounter + 1] ;
      double time_i = allCrates_crystalCrate_times[i][runCounter] ;
      double time_f = allCrates_crystalCrate_times[i][runCounter + 1] ;

      if (fabs(time_f - time_i) > largeThreshold) {
        B2INFO("WARNING: Crate " << i + 1 << " has crate time jump > " << largeThreshold << " ns: t(run " << run_f << ") = " << time_f <<
               " ns - t(run " << run_i << ") = " << time_i << " ns = " << time_f - time_i);
      }
    }
  }




  // Just in case, we remember the current TDirectory so we can return to it
  TDirectory* executeDir = gDirectory;

  tcratefile->Write();
  tcratefile->Close();
  // Go back to original TDirectory
  executeDir->cd();

  return c_OK;
}
