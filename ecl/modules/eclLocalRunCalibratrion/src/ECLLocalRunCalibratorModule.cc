/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Local run Digit Calibration.                                           *
 *                                                                        *
 * This module computes the averages and Stddev values for the fitted     *
 * time and amplitude                                                     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vitaly Vorobyev (vvorob@inp.nsk.su) (BINP),              *
 * Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// ECL
#include <ecl/modules/eclLocalRunCalibratrion/ECLLocalRunCalibratorModule.h>
#include <ecl/dataobjects/ECLTrig.h>

// FRAMEWORK
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBImportObjPtr.h>

// STL
#include <cmath>
#include <utility>

using namespace Belle2;
using namespace ECL;

using std::vector;
using std::string;
using std::sqrt;

// A value condirered to be zero if less then epsilon
const double epsilon = 0.0001;

bool is_good(const double& val)
{
  return !std::isnan(val) && (fabs(val) > epsilon);
}

bool is_good_posi(const double& val)
{
  return !std::isnan(val) && (val > epsilon);
}

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLLocalRunCalibrator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// Static constants
const string ECLLocalRunCalibratorModule::c_timeStr = "time";
const string ECLLocalRunCalibratorModule::c_amplStr = "amplitude";
const ECLLocalRunCalibratorModule::StrMap
ECLLocalRunCalibratorModule::c_tagMapRef = {
  {c_timeStr, "ECLRefTime"},
  {c_amplStr, "ECLRefAmpl"}
};
const ECLLocalRunCalibratorModule::StrMap
ECLLocalRunCalibratorModule::c_tagMap = {
  {c_timeStr, "ECLCalibTime"},
  {c_amplStr, "ECLCalibAmpl"}
};
const string ECLLocalRunCalibratorModule::c_eclDigitArrayName = "ECLDigits";
const string ECLLocalRunCalibratorModule::c_eclTrigsArrayName = "ECLTrigs";

// Default parameter values
const float def_max_time_offset = 0.001;
const float def_max_ampl_offset = 0.001;
const uint16_t def_max_rej_cnt = 5;

// Static variables
uint32_t ECLLocalRunCalibratorModule::Unit::m_maxDev;
uint32_t ECLLocalRunCalibratorModule::Unit::m_minCounts;

// Constructor
ECLLocalRunCalibratorModule::ECLLocalRunCalibratorModule()
{
  // Set module properties
  setDescription("Time and amplitude calibration of the ECL crystals");
  addParam("maxDev", Unit::m_maxDev,
           "Number of standard deviations for outliers rejection", 5u);
  addParam("minCounts", Unit::m_minCounts,
           "Minimal number of counts to activate outliers rejection", 100u);
  addParam("refMode", m_refMode, "Operate in reference mode", false);
  addParam("verbose", m_verb, "Print results in stdout", false);
//  addParam("logDir", m_logdir, "Log dir", "./");
  addParam("maxTimeOffset", m_max_time_offset, "Max time offset from the reference value", def_max_time_offset);
  addParam("maxAmplOffset", m_max_ampl_offset, "Max amplitude offset from the reference value", def_max_ampl_offset);
  addParam("maxRejectCount", m_max_misfit, "Max number of rejected counts", def_max_rej_cnt);
}

// initialize
void ECLLocalRunCalibratorModule::initialize()
{
  B2INFO("ECLLocalRunCalibratorModule::initialize()");
  m_max_offset.emplace(c_timeStr, m_max_time_offset);
  m_max_offset.emplace(c_amplStr, m_max_ampl_offset);
  fillReferenceMap(c_timeStr);
  fillReferenceMap(c_amplStr);
  B2INFO("ECLLocalRunCalibratorModule::initialize() done");
}

// begin run
void ECLLocalRunCalibratorModule::beginRun()
{
  B2INFO("ECLLocalRunCalibratorModule::beginRun()");
  m_aveMap.clear();
  m_aveMap.emplace(c_timeStr, vector<Unit>(c_nCrystals));
  m_aveMap.emplace(c_amplStr, vector<Unit>(c_nCrystals));
}

float ECLLocalRunCalibratorModule::Unit::getMean() const {return mean;}

float ECLLocalRunCalibratorModule::Unit::getStddev() const
{
  if (counter < 2) return -1.;  // something is wrong
  if (!is_good_posi(variance)) return 0.;  // can be negative
  float result = sqrt(variance / (counter - 1));
  if (std::isnan(result)) {
    B2INFO("nan getStddev(): " << variance << ", " << counter);
    return 0.;
  }
  return result;
}

uint32_t ECLLocalRunCalibratorModule::Unit::getCounter() const {return counter;}
uint32_t ECLLocalRunCalibratorModule::Unit::getMisfits() const {return misfits;}

bool ECLLocalRunCalibratorModule::Unit::isMisfit(float value)
{
  if (!m_maxDev) return false;  // accept everything if m_maxDev is zero
  const float stddev = getStddev();
  if (!is_good_posi(stddev)) return true;
  return std::fabs(value - mean) > stddev * m_maxDev;
}

ECLLocalRunCalibratorModule::Unit::Unit() :
  mean(0.), variance(0.), counter(0), misfits(0) {}

void ECLLocalRunCalibratorModule::Unit::add(float value)
{
  if (std::isnan(value)) {
    B2INFO("Unit::add: nan input!");
    return;
  }
  if ((counter > m_minCounts) && isMisfit(value)) {
    misfits++;
  } else {
    const auto old_mean = mean;
    mean += (value - mean) / static_cast<float>(++counter);
    if (std::isnan(mean)) {
      B2INFO("Unit::add: nan mean!");
      mean = old_mean;
      return;
    }
    // The way of computing variance goes back to a 1962 paper
    // by B. P. Welford and is presented in Donald Knuth’s Art
    // of Computer Programming, Vol 2, page 232, 3rd edition
    variance += (value - old_mean) * (value - mean);
    if (std::isnan(variance)) {
      B2INFO("Unit::add: nan variance!");
      return;
    }
  }
}

void ECLLocalRunCalibratorModule::checkOffset(const string& key) const
{
  if (!m_aveMap.count(key))
    B2FATAL("ECLLocalRunCalibraotorModule::checkResults(): wrong key " << key);

  const auto& unitVec = m_aveMap.find(key)->second;
  const auto& refsVec = m_refMap.find(key)->second;
  const auto& refsUncVec = m_refMapUnc.find(key)->second;
  const auto max_offset = m_max_offset.at(key);

  if (unitVec.size() != refsVec.size())
    B2FATAL("unitVec mismatches refsVec");

  // first loop fills in the units and catches small counters
  uint16_t cry = 0;
  for (const auto& unit : unitVec) {
    if (unit.getCounter() < c_minCounts) {
      B2INFO("Small " << key << " counter in crystal "
             << cry + 1 << ": " << unit.getCounter() << ", misfits: "
             << unit.getMisfits());
    } else if (unit.getMisfits() > m_max_misfit) {
      B2INFO("Large number of " << key << " misfit counts in crystal"
             << cry + 1 << ": " << unit.getMisfits());
    } else {
      const auto refVal = refsVec[cry];
      const auto refUncVal = refsUncVec[cry];
      if (!is_good(refVal) || !is_good_posi(refUncVal)) {
        B2INFO("Bad reference value " << refVal << " +- " << refUncVal
               << " for chan " << cry + 1);
      } else {
        auto offset = key == c_amplStr ? fabs(unit.getMean() / refVal - 1.) :
                      fabs(unit.getMean() - refVal);
        if (offset > max_offset) {
          B2INFO("Large " << key << " offset " << offset
                 << " in crystal " << cry + 1 << ". Val: " << unit.getMean()
                 << ", ref val: " << refVal);
        }
      }
    }
    cry++;
  }
}

void ECLLocalRunCalibratorModule::checkResults(const string& key) const
{
  Unit meanUnit;
  Unit rmsUnit;
  Unit misfitUnit;

  if (!m_aveMap.count(key))
    B2FATAL("ECLLocalRunCalibraotorModule::checkResults(): wrong key " << key);

  const auto& unitVec = m_aveMap.find(key)->second;
  const auto& refsVec = m_refMap.find(key)->second;
  const auto& refsUncVec = m_refMapUnc.find(key)->second;

  if (unitVec.size() != refsVec.size())
    B2FATAL("unitVec mismatches refsVec");

  // first loop fills in the units and catches small counters
  uint16_t cry = 0;
  for (const auto& unit : unitVec) {
    if (unit.getCounter() < c_minCounts) {
      B2INFO("Small " << key << " counter in crystal "
             << cry + 1 << ": " << unit.getCounter());
    } else {
      const auto refVal = refsVec[cry];
      const auto refUncVal = refsUncVec[cry];
      if (!is_good(refVal) || !is_good_posi(refUncVal)) {
        B2INFO("Bad reference value " << refVal << " +- " << refUncVal
               << " for chan " << cry + 1);
      } else {
        meanUnit.add(unit.getMean() / refVal);
        rmsUnit.add(unit.getStddev() / refUncVal);
        misfitUnit.add(unit.getMisfits());
      }
    }
    cry++;
  }

  B2INFO("### Summary for " << key << " mean ###");
  B2INFO("  Number of crystals: " << meanUnit.getCounter());
  B2INFO("                Mean: " << meanUnit.getMean());
  B2INFO("              Stddev: " << meanUnit.getStddev());
  B2INFO("    Misfits (approx): " << meanUnit.getMisfits());

  B2INFO("### Summary for " << key << " stddev ###");
  B2INFO("  Number of crystals: " << rmsUnit.getCounter());
  B2INFO("                Mean: " << rmsUnit.getMean());
  B2INFO("              Stddev: " << rmsUnit.getStddev());
  B2INFO("    Misfits (approx): " << rmsUnit.getMisfits());

  B2INFO("### Summary for " << key << " misfit ###");
  B2INFO("  Number of crystals: " << misfitUnit.getCounter());
  B2INFO("                Mean: " << misfitUnit.getMean());
  B2INFO("              Stddev: " << misfitUnit.getStddev());
  B2INFO("    Misfits (approx): " << misfitUnit.getMisfits());

  // second loop catches misfits
  cry = 1;
  for (const auto& unit : unitVec) {
    cry++;
    if (unit.getCounter() >= c_minCounts) {
      if (meanUnit.isMisfit(unit.getMean()))
        B2INFO("Bad mean " << key << " in crystal "
               << cry << ": " << unit.getMean()
               << ", cnt: " << unit.getCounter());
      if (rmsUnit.isMisfit(unit.getStddev()))
        B2INFO("Bad Stddev " << key << " in crystal "
               << cry << ": " << unit.getStddev()
               << ", cnt: " << unit.getCounter());
      if (unit.getMisfits() > c_maxMisfits)
        B2INFO("Many misfits for " << key << " in crystal "
               << cry << ": " << unit.getMisfits());
    }
  }
}

// event
void ECLLocalRunCalibratorModule::event()
{
  // Input Array
  StoreArray<ECLDigit> eclDigits(c_eclDigitArrayName);
  if (!eclDigits.isValid())
    B2FATAL("eclDigits not valid");

  // Loop over the input array
  for (const auto& digit : eclDigits) {
    // get the cell id from the ECLDigit as identifier
    auto cellid = digit.getCellId() - 1;

    // check that the cell id is valid
    if (cellid < 0 || cellid >= signed(c_nCrystals))
      B2FATAL("ECLLocalRunCalibratorModule::event(): cellid = "
              << cellid << " out of range!");

    // update time averages
    auto time = digit.getTimeFit();
    if (time != c_wrongTime)
      m_aveMap[c_timeStr][cellid].add(time + getTimeShift(digit));

    // update amplitude averages
    m_aveMap[c_amplStr][cellid].add(digit.getAmp());
  }
}

// Read collector's time to tune the time starting point
int16_t ECLLocalRunCalibratorModule::getTimeShift(const ECLDigit& digit) const
{
  auto trig = static_cast<uint32_t>(digit.getRelationsTo<ECLTrig>()[0]->getTimeTrig());
  switch (decodeTrigTime(trig) % 3) {
    case 1 : return 8;
    case 2 : return -8;
  }
  return 0;
}

// Decode time obtaned from collector */
inline uint32_t ECLLocalRunCalibratorModule::decodeTrigTime(uint32_t time) const
{
  return time - 2 * (time / 8);  // bits magic by Vladimir Zhulanov
}

// Read reference DB entry and fill the reference map
void ECLLocalRunCalibratorModule::fillReferenceMap(const string& key)
{
  if (m_refMode) {
    // normalize all values to unity in the case of reference mode
    m_refMap.emplace(key, vector<float>(c_nCrystals, 1.));
    m_refMapUnc.emplace(key, vector<float>(c_nCrystals, 1.));
    B2INFO("ECLLocalRunCalibratorModule: ref mode. All references are unit");
  } else {
    B2INFO("ECLLocalRunCalibratorModule: reading reference arrays");
    DBObjPtr<ECLCrystalCalib> m_refArray(c_tagMapRef.at(key));
    if (!m_refArray)
      B2FATAL("ECLLocalRunCalibratorModule: bad " << key << " reference object");
    m_refMap.emplace(key, m_refArray->getCalibVector());
    m_refMapUnc.emplace(key, m_refArray->getCalibUncVector());

    if (false && m_verb) {
      for (int cryno = 0; cryno < c_nCrystals; cryno++)
        B2INFO(" Ref " + key + " Cr " << cryno << ": "
               << m_refMap.at(key)[cryno] << " +- "
               << m_refMapUnc.at(key)[cryno]);
    }
  }
}

void ECLLocalRunCalibratorModule::saveObj(
  const string& key, const IntervalOfValidity& iov) const
{
  if (!m_aveMap.count(key))
    B2FATAL("ECLLocalRunCalibratorModule::fillTable(): wrong key " << key);
  if (m_verb)
    B2INFO("ECLLocalRunCalibratorModule::saveObj: " << key);

  vector<float> mean;
  vector<float> stddev;

  const auto& refsVec = m_refMap.find(key)->second;
  const auto& refsUncVec = m_refMapUnc.find(key)->second;

  uint16_t cnt = 0;
  for (const auto& unit : m_aveMap.find(key)->second) {
    auto refmean = is_good(refsVec[cnt]) ? refsVec[cnt] : 1.;
    auto refunc = is_good_posi(refsUncVec[cnt]) ? refsUncVec[cnt] : 1.;

    const auto mean_diff = key == c_amplStr ? unit.getMean() / refmean :
                           unit.getMean() - refmean;
    const auto stdd_diff = unit.getStddev() / refunc;

    mean.emplace_back(mean_diff);
    stddev.emplace_back(stdd_diff);
    cnt++;
    if (m_verb) {
      B2INFO(cnt << ": mean: " << mean_diff
             ///!!! << " (" << unit.getMean() << " / " << refmean << ")"
             <<  " stddev: " << stdd_diff
             ///!!! << " (" << unit.getStddev() << " / " << refunc << ")"
             <<    " cntr: " << unit.getCounter()
             << " misfits: " << unit.getMisfits()
            );
    }
  }

  ECLCrystalCalib calarr;
  calarr.setCalibVector(mean, stddev);
  const auto tag = m_refMode ? c_tagMapRef.at(key) : c_tagMap.at(key);
  DBImportObjPtr<ECLCrystalCalib> dbobj(tag);
  dbobj.construct(calarr);
  dbobj.import(iov);
}

// end run
void ECLLocalRunCalibratorModule::endRun()
{
  if (!m_refMode) {
    B2INFO("Cheking offsets");
    checkOffset(c_timeStr);
    checkOffset(c_amplStr);
    B2INFO("Cheking distributions");
    checkResults(c_timeStr);
    checkResults(c_amplStr);
  }

  StoreObjPtr<EventMetaData> evtPtr;
  auto exp = evtPtr->getExperiment();
  auto run = evtPtr->getRun();
  if (m_verb) {
    B2INFO("ECLLocalRunCalibratorModule::endRun: exp " << exp
           << ", run " << run);
  }
  IntervalOfValidity iov(exp, run, exp + 100, run + 10000);

  saveObj(c_timeStr, iov);
  saveObj(c_amplStr, iov);
}

