/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * ECLLocalRunCalibratorModule                                            *
 *                                                                        *
 * This module computes the mean values and standard deviations           *
 * for the fitted time and amplitude.                                     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP),        *
 * Vitaly Vorobyev (vvorob@inp.nsk.su) (BINP)                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// ECL
#include <ecl/modules/eclLocalRunCalibration/ECLLocalRunCalibratorModule.h>
#include <ecl/dataobjects/ECLTrig.h>

// ROOT
#include "TFile.h"

using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLLocalRunCalibrator)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
// Number of cell ids.
const int ECLLocalRunCalibratorModule::c_ncellids = 8736;
// Time payload name.
const std::string
ECLLocalRunCalibratorModule::c_timePayloadName = "ECLCalibTime";
// Amplitude payload name.
const std::string
ECLLocalRunCalibratorModule::c_amplPayloadName = "ECLCalibAmplitude";
// Constructor.
ECLLocalRunCalibratorModule::ECLLocalRunCalibratorModule():
  m_time(nullptr), m_ampl(nullptr)
{
  // Module description.
  setDescription("ECL Local Run Calibration.");
  // Initial lower limit for time.
  addParam("minTime", m_minTime, "Initial lower limit for time.", -500.f);
  // Initial upper limit for time.
  addParam("maxTime", m_maxTime, "Initial upper limit for time.", 500.f);
  // Initial lower limit for amplitude.
  addParam("minAmpl", m_minAmpl, "Initial lower limit for amplitude.", 1.e+04f);
  // Initial upper limit for amplitude.
  addParam("maxAmpl", m_maxAmpl, "Initial upper limit for amplitude.", 3.e+04f);
  // Number of standard deviations
  // used to update lower and
  // upper value limits.
  addParam("nOfStdDevs", m_devs,
           "Number of standard deviations used to form initial limits.", 5);
  // Boolean flag used to choose local or central DB.
  addParam("isLocal", m_isLocal, "Boolean flag used to"
           "select local or central DB.", true);
  // Default path to a local database.
  std::string defaultDBName = "localdb/database.txt";
  // Central DB name or path to a local DB.
  addParam("dbName", m_dbName, "Central DB name or path "
           "to a local DB.", defaultDBName);
  // Used to change validity intervals in the previous run.
  addParam("changePrev", m_changePrev,
           "If changePrev is set, the upper run in the interval of "
           "validity for the previous written payload will be changed.",
           false);
  // Low run for the current validity interval.
  addParam("lowRun", m_lowRun,
           "Low run for validity interval. "
           "If the low run value is negative, "
           "current run number "
           "is used as a low run.", -1);
  // High run for the current validity interval.
  addParam("highRun", m_highRun,
           "High run for validity interval. "
           "If the high run value is equal to -1, "
           "the high run of validity interval is not limited.",
           -1);
  // Mark current run as reference.
  addParam("addRef", m_addref,
           "Mark run as reference if it is"
           "needed to do that at start",
           false);
  // Enables histogram filling mode.
  addParam("fillHisto", m_isFillHisto,
           "Fill time and amplitude histograms"
           "for a certain cell id.", false);
  // Cell id.
  addParam("cellid", m_cellid,
           "Cell id number, used to "
           "fill amplitude or time histogram.");
  // Default file name used to save histograms,
  // obtained in the histogram filling mode.
  const std::string defaultHistoFileName =
    "time_ampl_histo.root";
  // File name used to save histograms,
  // obtained in the histogram filling mode.
  addParam("histoFileName", m_histoFileName,
           "File name used to save amplitude and time"
           "histograms for a certain cell id.",
           defaultHistoFileName);
  // Enables full tree writing mode.
  addParam("fulltree", m_fulltree,
           "Fill full tree.", false);
}
// Destructor.
ECLLocalRunCalibratorModule::~ECLLocalRunCalibratorModule()
{
  if (m_time) {
    delete m_time;
  }
  if (m_ampl) {
    delete m_ampl;
  }
}
// Begin run
void ECLLocalRunCalibratorModule::beginRun()
{
  if (m_isFillHisto) {
    // Histogram filling mode.
    // Creating histograms, which will
    // be contain time and amplitude distributions
    // for a certain cell id.
    m_histo_time = new TH1F("histo_time", "", 10000, -1, -1);
    m_histo_ampl = new TH1F("histo_ampl", "", 10000, -1, -1);
  } else {
    // Local run calibration mode.
    // Creating objects, which will be contain
    // time and amplitude accumulators for all
    // cell ids.
    m_time = new ECLLocalRunCalibUnit(c_ncellids,
                                      m_minTime, m_maxTime,
                                      &m_devs);
    m_ampl = new ECLLocalRunCalibUnit(c_ncellids,
                                      m_minAmpl, m_maxAmpl,
                                      &m_devs);
    if (m_fulltree) {
      m_tree = new TTree("localrun", "");
      m_tree->Branch("cellid", &m_tree_cellid,
                     "m_tree_cellid/I");
      m_tree->Branch("time", &m_tree_time,
                     "time/F");
      m_tree->Branch("ampl", &m_tree_ampl,
                     "ampl/F");
      m_tree_event = 0;
      m_tree->Branch("event", &m_tree_event,
                     "event/I");
    }
  }
}
// Event.
void ECLLocalRunCalibratorModule::event()
{
  // Getting amplitude and time input values.
  if (!m_ECLDigits.isValid()) {
    B2FATAL("ECLDigits not valid");
  }
  // Loop over the input array.
  for (const auto& digit : m_ECLDigits) {
    // Getting cell id and setting the
    // corresponding index.
    auto cellid = digit.getCellId();
    auto index = cellid - 1;
    // Check that the cell id is valid.
    if (index < 0 || index >= signed(c_ncellids)) {
      B2FATAL("ECLLocalRunCalibratorModule::event(): cell id = "
              << cellid << " out of range!");
    }
    // Getting raw time and decoding it.
    auto time = digit.getTimeFit();
    auto shiftedTime = time + getTimeShift(digit);
    // Getting amplitude.
    auto ampl = digit.getAmp();
    if (m_isFillHisto) {
      // Histogram filling mode.
      // Filling time and amplitude
      // histograms for a certain
      // cell id.
      if (cellid == m_cellid) {
        m_histo_time->Fill(shiftedTime);
        m_histo_ampl->Fill(ampl);
      }
    } else {
      // Local run calibration mode.
      // Accumulating time mean
      // values and standard deviations.
      m_time->add(index, shiftedTime);
      // Accumulating amplitude mean
      // values and standard deviations.
      m_ampl->add(index, ampl);
      if (m_fulltree) {
        m_tree_cellid = cellid;
        m_tree_time = shiftedTime;
        m_tree_ampl = ampl;
        m_tree->Fill();
      }
      // Enabling negative value alarm,
      // if there are negative amplitudes.
      if (ampl < 0) {
        m_ampl->enableNegAmpl();
      }
    }
  }

  if (m_fulltree) {
    m_tree_event++;
  }
}
// Write histograms to file
// in the case of the histogram
// filling mode.
void ECLLocalRunCalibratorModule::writeHistoToFile()
{
  TFile fl(m_histoFileName.c_str(), "recreate");
  fl.cd();
  m_histo_time->Write();
  m_histo_ampl->Write();
  fl.Close();
  delete m_histo_time;
  delete m_histo_ampl;
}
// Write calibration results
// into a database.
void ECLLocalRunCalibratorModule::writeCalibResultsToDB()
{
  // Getting experiment and run
  // numbers.
  int exp = m_EventMetaData->getExperiment();
  int run = m_EventMetaData->getRun();
  int lowrun;
  // Setting low run
  // of validity interval.
  if (m_lowRun < 0) {
    lowrun = run;
  } else {
    lowrun = m_lowRun;
  }
  // Creating validity interval.
  IntervalOfValidity iov(exp, lowrun, exp, m_highRun);
  // Saving time payload.
  m_time->writeToDB(m_isLocal, m_dbName,
                    c_timePayloadName, iov,
                    run, m_changePrev, m_addref);
  // Saving amplitude payload.
  m_ampl->writeToDB(m_isLocal, m_dbName,
                    c_amplPayloadName, iov,
                    run, m_changePrev, m_addref);
}
// End run.
void ECLLocalRunCalibratorModule::endRun()
{
  if (m_isFillHisto) {
    // Histogram filling mode.
    // Writing histograms to file.
    writeHistoToFile();
  } else {
    // Local run calibration
    // mode.
    // Getting accumulated
    // mean values and
    // standard deviations.
    m_time->calc();
    m_ampl->calc();
    // Saving calibration results
    // into a database.
    writeCalibResultsToDB();
    if (m_fulltree) {
      auto fl = TFile::Open("/ghi/fs01/belle2/bdata/group/detector/ECL/local-run-db/data-full/ecl_local_run_fulltree.root",
                            "recreate");
      fl->cd();
      m_tree->Write();
      fl->Close();
      delete m_tree;
    }
  }
}
// Read collector's time to
// tune the time starting point.
int16_t ECLLocalRunCalibratorModule::getTimeShift(
  const ECLDigit& digit) const
{
  auto trig =
    static_cast<uint32_t>(
      digit.getRelationsTo<ECLTrig>()[0]->getTimeTrig());
  switch (decodeTrigTime(trig) % 3) {
    case 1 : return 8;
    case 2 : return -8;
  }
  return 0;
}
// Decode time obtained
// from collector.
inline uint32_t
ECLLocalRunCalibratorModule::decodeTrigTime(uint32_t time) const
{
  // Bits magic by
  // Vladimir Zhulanov.
  return time - 2 * (time / 8);
}

