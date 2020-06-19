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
eclTimeShiftsAlgorithm::eclTimeShiftsAlgorithm(): CalibrationAlgorithm("DummyCollector"),
  debugFilenameBase("ECL_time_offsets"),
  timeShiftForPlotStyle{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  crysCrateShift_min(-20),
  crysCrateShift_max(20),
  m_ECLCrystalTimeOffset("ECLCrystalTimeOffset"),
  m_ECLCrateTimeOffset("ECLCrateTimeOffset"),
  m_refCrysIDzeroingCrate("ECLReferenceCrystalPerCrateCalib"),
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
  B2INFO("forcePayloadIOVnotOpenEndedAndSequentialRevision = " << forcePayloadIOVnotOpenEndedAndSequentialRevision);
  B2INFO("timeShiftForPlotStyle = {");
  for (int crateTest = 0; crateTest < 51; crateTest++) {
    B2INFO(timeShiftForPlotStyle[crateTest] << ",");
  }
  B2INFO(timeShiftForPlotStyle[51] << "}");


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
    B2INFO("Exiting with failure");
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

  B2INFO("minExpNum = " << minExpNum) ;
  B2INFO("minRunNum = " << minRunNum) ;
  B2INFO("maxExpNum = " << maxExpNum) ;
  B2INFO("maxRunNum = " << maxRunNum) ;


  if (minExpNum != maxExpNum) {
    B2WARNING("The runs must all come from the same experiment");
    return c_Failure;
  }

  int experiment = minExpNum;

  //------------------------------------------------------------------------
  // Conversion coefficient from ADC ticks to nanoseconds
  const double TICKS_TO_NS = 1.0 / (4.0 * EclConfiguration::m_rf) *
                             1e3;  // 1/(4fRF) = 0.4913 ns/clock tick, where fRF is the accelerator RF frequency, fRF=508.889 MHz. Same for all crystals.  Proper accurate value

  // Need to load information about the event/run/experiment to get the right database information
  // Will be used for:
  // * ECLChannelMapper (to map crystal to crates)
  // * crystal payload updating for iterating crystal and crate fits
  int eventNumberForCrates = 1;


  //------------------------------------------------------------------------
  /* Set up variables for storing timing information and cutting on
     timing quality */

  double tcrate_min_cut = -150; //-20 ;
  double tcrate_max_cut = 150; //10 ;
  double tcrate_unc_min_cut = 0.0001 ;
  double tcrate_unc_max_cut = 0.4 ;

  vector< vector<double> > allCrates_crate_times ;
  vector< vector<double> > allCrates_run_nums ;
  vector< vector<double> > allCrates_time_unc ;
  vector< vector<double> > allCrates_crystalCrate_times ;
  vector< vector<double> > allCrates_crystalCrate_times_unc ;

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

  int previousRevNum = -1 ;

  //------------------------------------------------------------------------
  /** Loop over all the experiments and runs and extract the crate times*/
  for (int run = minRunNum; run <= maxRunNum; run++) {

    B2INFO("---------") ;
    B2INFO("Looking at run " << run) ;

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
        the high run number.  This should hopefully skip the runs that*/

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

        mean_crystalCrate_time_ns[crateID_temp - 1] += (crystalTimeOffsetsCalib[crysID - 1] +
                                                        crateTimeOffsetsCalib[crysID - 1]) * TICKS_TO_NS;

      }
    }


    for (int iCrate = 0; iCrate < m_numCrates; iCrate++) {
      double tcrate = Crate_time_ns[iCrate] ;
      double tcrate_unc = Crate_time_unc_ns[iCrate];
      double tcrystalCrate = crystalCrate_time_ns[iCrate];
      double tcrystalCrate_unc = crystalCrate_time_unc_ns[iCrate];

      if ((tcrate < tcrate_max_cut) &&
          (tcrate > tcrate_min_cut) &&
          (fabs(tcrate_unc) > tcrate_unc_min_cut) &&
          (fabs(tcrate_unc) < tcrate_unc_max_cut)) {
        allCrates_crate_times[iCrate].push_back(tcrate) ;
        allCrates_run_nums[iCrate].push_back(run) ;
        allCrates_time_unc[iCrate].push_back(tcrate_unc) ;
        allCrates_crystalCrate_times[iCrate].push_back(tcrystalCrate) ;
        allCrates_crystalCrate_times_unc[iCrate].push_back(tcrystalCrate_unc) ;
      }
    }


    //------------------------------------------------------------------------
    /** Write out a few values for quality control purposes */
    for (int ic = 0; ic < m_numCrates; ic++) {
      B2INFO("Crate " << ic + 1 << ", t_crate = " << Crate_time_tick[ic] << " ticks = "
             << Crate_time_ns[ic] << " +- " << Crate_time_unc_ns[ic]
             << " ns;    t crys+crate+shift = " << crystalCrate_time_ns[ic] << " +- "
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


  //------------------------------------------------------------------------
  /** Shift all the crystal+crate times by the mean time to naturally
      roughly centre all the crys+crate+shift times*/
  B2INFO("Shift all run crys+crate+off times.  Show the results for a subset of crates/runs:");
  for (int iCrate = 0; iCrate < m_numCrates; iCrate++) {
    double mean_time = mean_crystalCrate_time_ns[iCrate] / allCrates_crate_times[iCrate].size() ;
    B2INFO("Mean crys+crate times for all runs used as offset (crate " << iCrate + 1 << ") = " << mean_time);

    for (long unsigned int jRun = 0; jRun < allCrates_crate_times[iCrate].size(); jRun++) {
      allCrates_crystalCrate_times[iCrate][jRun] += -mean_time + timeShiftForPlotStyle[iCrate] ;
      if (jRun < 4 || iCrate == 1 || iCrate == 40 || iCrate == 51) {
        B2INFO("allCrates_crystalCrate_times(crate " << iCrate + 1 << ", run counter " << jRun + 1 << ") = " <<
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
    tgraph_title = tgraph_title + string(" (") + to_string(tcrate_min_cut) + string(" < tcrate < ") +
                   to_string(tcrate_max_cut) + string(" ns, ") + to_string(tcrate_unc_min_cut) +
                   string(" < tcrate unc. < ") + to_string(tcrate_unc_max_cut) + string(" ns cuts)")  ;

    g_tcrate_vs_runNum->SetName(tgraph_name_short.c_str()) ;
    g_tcrate_vs_runNum->SetTitle(tgraph_title.c_str()) ;
    g_tcrate_vs_runNum->GetXaxis()->SetTitle("Run number") ;
    g_tcrate_vs_runNum->GetYaxis()->SetTitle("Crate time [ns]") ;

    g_tcrate_vs_runNum->GetYaxis()->SetRangeUser(tcrate_min_cut, tcrate_max_cut) ;

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
    tgraph_title = tgraph_title + string(" (") + to_string(tcrate_min_cut) + string(" < tcrate < ") +
                   to_string(tcrate_max_cut) + string(" ns, ") + to_string(tcrate_unc_min_cut) +
                   string(" < tcrate unc. < ") + to_string(tcrate_unc_max_cut) + string(" ns cuts)")  ;


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

  }


  // Just in case, we remember the current TDirectory so we can return to it
  TDirectory* executeDir = gDirectory;

  tcratefile->Write();
  tcratefile->Close();
  // Go back to original TDirectory
  executeDir->cd();

  return c_OK;
}
