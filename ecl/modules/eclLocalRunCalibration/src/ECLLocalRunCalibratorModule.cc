/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su),               *
 * Vitaly Vorobyev (vvorob@inp.nsk.su) (BINP)                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <math.h>
// FRAMEWORK
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
// ECL
#include <ecl/modules/eclLocalRunCalibration/ECLLocalRunCalibratorModule.h>
#include <ecl/dataobjects/ECLTrig.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
// STL
#include <cmath>
#include <limits>
// BOOST
#include <boost/format.hpp>
// ROOT
#include <TTree.h>
#include "TFile.h"
using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLLocalRunCalibrator)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
const std::string ECLLocalRunCalibratorModule::c_eclDigitArrayName = "ECLDigits";
const std::string ECLLocalRunCalibratorModule::c_timeKey = "time";
const std::string ECLLocalRunCalibratorModule::c_amplKey = "ampl";
const ECLLocalRunCalibratorModule::StringMap
ECLLocalRunCalibratorModule::c_refTags = {
  {c_timeKey, "ECLRefTime"},
  {c_amplKey, "ECLRefAmpl"}
};
const ECLLocalRunCalibratorModule::StringMap
ECLLocalRunCalibratorModule::c_calibTags = {
  {c_timeKey, "ECLCalibTime"},
  {c_amplKey, "ECLCalibAmpl"}
};
// Constructor
ECLLocalRunCalibratorModule::ECLLocalRunCalibratorModule()
{
  // Module description
  setDescription("Time and amplitude calibration of the ECL crystals.");
  // Reference mode
  addParam("refMode", m_refMode, "Enables reference mode.", false);
  // Verbose mode
  addParam("verbose", m_verbose, "Enables verbose mode.", false);
  // Path to debug file
  addParam("debugFile", m_debugFile, "Path to debug file. If debugFile "
           "is the empty string, there is no debug file will be created.",
           std::string(""));
  // Initial lower limit for time
  addParam("minTime", m_minTime, "Initial lower limit for time.", -500.f);
  // Initial upper limit for time
  addParam("maxTime", m_maxTime, "Initial upper limit for time.", 500.f);
  // Initial lower limit for amplitude
  addParam("minAmpl", m_minAmpl, "Initial lower limit for amplitude.", 1.e+04f);
  // Initial upper limit for amplitude
  addParam("maxAmpl", m_maxAmpl, "Inital upper limit for amplitude.", 3.e+04f);
  addParam("maxTimeOffset", m_maxTimeOffset, "Maximum time offset.",
           0.3f);
  addParam("maxAmplOffset", m_maxAmplOffset, "Maximum amplitude offset.",
           0.001f);
  // Number of events used to calculate median value and values
  addParam("initNOfEvents", m_initNOfEvents,
           "Number of events used to calculate median value, "
           "upper and lower limits.",
           100);
  // Number of standard deviations
  // used to calculate lower and
  // upper value limits
  addParam("nOfStdDevs", m_nOfStdDevs,
           "Number of stddevs used to form inital limits.",
           5);
}

ECLLocalRunCalibratorModule::~ECLLocalRunCalibratorModule()
{
}

void ECLLocalRunCalibratorModule::beginRun()
{
  m_data.clear();
  m_data.emplace(
    c_timeKey,
    DataVect(
      c_nCrystals,
      ECLLocalRunCalibrationData(
        m_minTime,
        m_maxTime,
        m_initNOfEvents,
        m_nOfStdDevs)));
  m_data.emplace(
    c_amplKey,
    DataVect(
      c_nCrystals,
      ECLLocalRunCalibrationData(
        m_minAmpl,
        m_maxAmpl,
        m_initNOfEvents,
        m_nOfStdDevs)));
}
// Fill debug tree for
// calibration run.
void ECLLocalRunCalibratorModule::fillDebugCalibTree(
  const std::string& key,
  const DBObjPtr<ECLCrystalCalib>& refArray)
{
  int cryNumber;
  int isOffset;
  int nOfMisfits;
  int nOfCounts;
  float calibMean;
  float calibStdDev;
  float refMean;
  float refStdDev;
  float calibNormMean;
  float calibNormStdDev;
  std::string treeName = (boost::format("calibTree_%s") % key).str();
  m_treeMap[key] = new TTree(treeName.c_str(), "");
  // Crystal number
  m_treeMap.at(key)->Branch("crystal",
                            &cryNumber,
                            "crystal/I");
  // 1 if the calibration value
  // offset is big and 0 in a nornmal
  // case
  m_treeMap.at(key)->Branch("isoffset",
                            &isOffset,
                            "isoffset/I");
  // Number of misfits
  m_treeMap.at(key)->Branch("nofmisfits",
                            &nOfMisfits,
                            "nofmisfits/I");
  // Number of counts
  m_treeMap.at(key)->Branch("nofcounts",
                            &nOfCounts,
                            "nofcounts/I");
  // Referene mean value
  m_treeMap.at(key)->Branch("refmean",
                            &refMean,
                            "refmean/F");
  // Reference standard deviation
  m_treeMap.at(key)->Branch("refstddev",
                            &refStdDev,
                            "refstddev/F");
  // Mean value for
  // calibration run
  m_treeMap.at(key)->Branch("calibmean",
                            &calibMean,
                            "calibmean/F");
  // Standard deviation for
  // calibration run
  m_treeMap.at(key)->Branch("calibstddev",
                            &calibStdDev,
                            "calibstddev/F");
  // Normalized mean value
  // for calibration run
  m_treeMap.at(key)->Branch("calibnormmean",
                            &calibNormMean,
                            "calibnormmean/F");
  // Normalized standard deviation
  // for calibration run
  m_treeMap.at(key)->Branch("calibnormstddev",
                            &calibNormStdDev,
                            "calibnormstddev/F");
  for (int cry = 0; cry < c_nCrystals; ++cry) {
    cryNumber = cry + 1;
    nOfMisfits = m_data.at(key)[cry].getNOfRejectedInputs();
    nOfCounts = m_data.at(key)[cry].getNOfAcceptedInputs();
    calibMean = m_data.at(key)[cry].getMean();
    calibStdDev = m_data.at(key)[cry].getStdDev();
    refMean = refArray->getCalibVector()[cry];
    refStdDev = refArray->getCalibUncVector()[cry];
    calibNormMean = normalizeMean(key, calibMean, refMean);
    calibNormStdDev = normalizeStdDev(calibStdDev, refStdDev);
    isOffset = m_data.at(key)[cry].getOffsetFlag() ? 1 : 0;
    m_treeMap.at(key)->Fill();
  }
}
// Fill debug tree for reference run.
void ECLLocalRunCalibratorModule::fillDebugRefTree(
  const std::string& key)
{
  int cryNumber;
  int nOfMisfits;
  int nOfCounts;
  float refMean;
  float refStdDev;
  std::string treeName = (boost::format("refTree_%s") % key).str();
  m_treeMap[key] = new TTree(treeName.c_str(), "");
  // Crystal number
  m_treeMap.at(key)->Branch("crystal",
                            &cryNumber,
                            "crystal/I");
  // Number of misfists
  m_treeMap.at(key)->Branch("nofmisfits",
                            &nOfMisfits,
                            "nofmisfits/I");
  // Number of counts
  m_treeMap.at(key)->Branch("nofcounts",
                            &nOfCounts,
                            "nofcounts/I");
  // Mean value for
  // reference run
  m_treeMap.at(key)->Branch("refmean",
                            &refMean,
                            "refmean/F");
  // Standard deviation
  // for reference run
  m_treeMap.at(key)->Branch("refstddev",
                            &refStdDev,
                            "refstddev/F");
  for (int cry = 0; cry < c_nCrystals; ++cry) {
    cryNumber = cry + 1;
    nOfMisfits = m_data.at(key)[cry].getNOfRejectedInputs();
    nOfCounts = m_data.at(key)[cry].getNOfAcceptedInputs();
    refMean = m_data.at(key)[cry].getMean();
    refStdDev = m_data.at(key)[cry].getStdDev();
    m_treeMap.at(key)->Fill();
  }
}
// Normalize standard deviation
// in relation to reference
// standard deviation.
float ECLLocalRunCalibratorModule::normalizeStdDev(
  const float& calibStdDev,
  const float& refStdDev) const
{
  return calibStdDev / refStdDev - 1;
}
// Normalize mean value in relation
// to reference mean value.
float ECLLocalRunCalibratorModule::normalizeMean(
  const std::string& key,
  const float& calibMean,
  const float& refMean) const
{
  if (key == c_timeKey) {
    return calibMean - refMean;
  }
  return calibMean / refMean - 1;
}
// Write mean values and standard
// deviations to database.
void ECLLocalRunCalibratorModule::writeObjToDB(
  const std::string& key,
  const IntervalOfValidity& iov)
{
  // Raise error if key doesn't exist
  // in m_data map
  if (!m_data.count(key)) {
    B2FATAL("ECLLocalRunCalibratorModule::fillTable(): wrong key " << key);
  }
  // vector of mean values
  std::vector<float> means;
  // vector of standard deviations
  std::vector<float> stddevs;
  for (const auto& el : m_data.at(key)) {
    means.emplace_back(el.getMean());
    stddevs.emplace_back(el.getStdDev());
  }
  if (m_refMode) {
    // If the debug file path
    // is not an empty string,
    // fill debug trees and wirte
    // them into debug file.
    if (m_debugFile.size() > 0) {
      fillDebugRefTree(key);
    }
  } else {
    // Calibration mode.
    // Reading reference mean values
    // and standard deviations.
    DBObjPtr<ECLCrystalCalib> refArray(c_refTags.at(key));
    if (!refArray) {
      B2FATAL("ECLLocalRunCalibratorModule: bad "
              << key << " reference object");
    }
    float normMeanVal;
    for (int cry = 0; cry < c_nCrystals; ++cry) {
      // Get mean values and
      // standard deviations
      // for each crystal.
      float refMean = refArray->getCalibVector()[cry];
      // Calculate normalized
      // mean value.
      normMeanVal = normalizeMean(
                      key,
                      m_data.at(key)[cry].getMean(),
                      refMean);
      // Enable offset flag
      if (key == c_timeKey &&
          normMeanVal > m_maxTimeOffset) {
        m_data.at(key)[cry].enableOffsetFlag();
      }
      if (key == c_amplKey &&
          normMeanVal > m_maxAmplOffset) {
        m_data.at(key)[cry].enableOffsetFlag();
      }
      // If the debug file path
      // is not an empty string,
      // fill debug trees and wirte
      // them into debug file.
      if (m_debugFile.size() > 0) {
        fillDebugCalibTree(key, refArray);
      }
    }
  }
  // Write mean values and
  // standard deviations
  // to database.
  ECLCrystalCalib calarr;
  calarr.setCalibVector(means, stddevs);
  const auto tag = m_refMode ? c_refTags.at(key) : c_calibTags.at(key);
  DBImportObjPtr<ECLCrystalCalib> dbobj(tag);
  dbobj.construct(calarr);
  dbobj.import(iov);
}

void ECLLocalRunCalibratorModule::event()
{
  if (!m_eclDigits.isValid()) {
    B2FATAL("eclDigits not valid");
  }
  // Loop over the input array.
  for (const auto& digit : m_eclDigits) {
    // Get the cell id from the
    // ECLDigit as identifier.
    auto cellid = digit.getCellId() - 1;
    // Check that the cell id is valid.
    if (cellid < 0 || cellid >= signed(c_nCrystals)) {
      B2FATAL("ECLLocalRunCalibratorModule::event(): cellid = "
              << cellid << " out of range!");
    }
    // Accumulate time mean
    // values and standard deviations.
    auto time = digit.getTimeFit();
    m_data.at(c_timeKey)[cellid].add(
      time + getTimeShift(digit));
    // Accamulate amplitude mean
    // values and standard deviations.
    m_data.at(c_amplKey)[cellid].add(
      digit.getAmp());
  }
}
// Read collector's time to
// tune the time starting point.
int16_t ECLLocalRunCalibratorModule::getTimeShift(
  const ECLDigit& digit) const
{
  auto trig = static_cast<uint32_t>(digit.getRelationsTo<ECLTrig>()[0]->getTimeTrig());
  switch (decodeTrigTime(trig) % 3) {
    case 1 : return 8;
    case 2 : return -8;
  }
  return 0;
}
// Decode time obtaned
// from collector.
inline uint32_t ECLLocalRunCalibratorModule::decodeTrigTime(uint32_t time) const
{
  // Bits magic by
  // Vladimir Zhulanov.
  return time - 2 * (time / 8);
}

void ECLLocalRunCalibratorModule::endRun()
{
  for (const auto& key : m_data) {
    for (auto& el : m_data.at(key.first)) {
      el.stop();
    }
  }
  // Get experiment
  // and run numbers.
  auto exp = m_evtPtr->getExperiment();
  auto run = m_evtPtr->getRun();
  int run_min;
  int run_max;
  // Set minimum and maximum
  // run numbers.
  if (m_refMode) {
    run_min = 1;
    run_max = -1;
  } else {
    run_min = run;
    run_max = run;
  }
  // Create interval
  // of validity.
  IntervalOfValidity iov(exp, run_min, exp, run_max);
  // Write mean values and standard
  // deviations for time to
  // database.
  writeObjToDB(c_timeKey, iov);
  // Write mean values and standard
  // deviations for amplitude to
  // database.
  writeObjToDB(c_amplKey, iov);
  // Write to results
  // to debug file.
  if (m_debugFile.size() > 0) {
    writeToFile();
  }
  // Print summary
  if (m_verbose) {
    printSummary(c_timeKey);
    printSummary(c_amplKey);
  }
}

void ECLLocalRunCalibratorModule::writeToFile() const
{
  auto fl = TFile::Open(m_debugFile.c_str(), "recreate");
  fl->cd();
  for (const auto& el : m_treeMap) {
    el.second->Write();
  }
  fl->Close();
  delete fl;
}

void ECLLocalRunCalibratorModule::printSummary(const std::string& key) const
{
  float totalMisfits = 0;
  float totalCounts = 0;
  float totalOffsets = 0;
  for (const auto& el : m_data.at(key)) {
    totalMisfits += el.getNOfRejectedInputs();
    totalCounts += el.getNOfAcceptedInputs();
    totalOffsets += el.getOffsetFlag() ? 1 : 0;
  }
  std::string summaryTitle;
  if (key == c_timeKey) {
    summaryTitle = "TIME SUMMARY:";
  } else {
    summaryTitle = "AMPLITUDE SUMMARY:";
  }
  B2INFO("\n--------------------------------------\n" <<
         summaryTitle << "\n" <<
         "Total number of misfits: " << totalMisfits << "\n" <<
         "Total number of counts: " << totalCounts << "\n" <<
         "Total number of offsets: " << totalOffsets << "\n" <<
         "--------------------------------------\n");
}
