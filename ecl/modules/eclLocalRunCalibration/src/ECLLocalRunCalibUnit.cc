/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * ECLLocalRunCalibUnit                                                   *
 *                                                                        *
 * This class controls feature (mean value, standard deviation and        *
 * number of accepted events) accumulators for each cell id.              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// FRAMEWORK
#include <framework/database/DBObjPtr.h>
// ECL
#include <ecl/modules/eclLocalRunCalibration/ECLLocalRunCalibUnit.h>
#include <ecl/dbobjects/ECLCrystalLocalRunCalib.h>
#include <ecl/dbobjects/ECLLocalRunCalibRef.h>
using namespace Belle2;
// Constructor.
ECLLocalRunCalibUnit::ECLLocalRunCalibUnit(
  const int& ncellids,
  const float& min_value,
  const float& max_value,
  const int* const ndevs):
  m_isNegAmpl(false), m_unitData(ncellids,
                                 ECLLocalRunCalibAcc(
                                   min_value,
                                   max_value,
                                   ndevs))
{
}
// Destructor.
ECLLocalRunCalibUnit::~ECLLocalRunCalibUnit()
{
}
// This function will be called, if the
// negative amplitude values are observed
// in the current run.
void ECLLocalRunCalibUnit::enableNegAmpl()
{
  m_isNegAmpl = true;
}
// Checking presence of negative
// amplitudes.
bool ECLLocalRunCalibUnit::isNegAmpl() const
{
  return m_isNegAmpl;
}
// Add value to
// accamulate mean value,
// standard deviation and
// number of accepted events.
void ECLLocalRunCalibUnit::add(const int& index, const float& value)
{
  m_unitData[index].add(value);
}
// Calculate accumulated values.
void ECLLocalRunCalibUnit::calc()
{
  for (auto& cellAcc : m_unitData) {
    cellAcc.calc();
  }
}
// Getter of accumulated values.
template <typename T>
void ECLLocalRunCalibUnit::callAccGetter(
  std::vector<T>* vec,
  T(ECLLocalRunCalibAcc::*getter)() const)
{
  for (const auto& obj : m_unitData) {
    vec->push_back((obj.*getter)());
  }
}
// Make current run as reference.
void ECLLocalRunCalibUnit::markAsRefference(const bool& isLocal,
                                            const std::string& dbName,
                                            const int& run,
                                            const IntervalOfValidity& iov)
{
  int exp = iov.getExperimentLow();
  ECLDBTool refPayload(isLocal, dbName.c_str(),
                       "ECLCalibRef");
  refPayload.connect();
  ECLLocalRunCalibRef refobj(exp, run);
  refPayload.write(&refobj, iov);
}
// Change previous validity interval.
void ECLLocalRunCalibUnit::changePreviousIoV(const ECLDBTool& payload,
                                             const IntervalOfValidity& iov)
{
  int exp = iov.getExperimentLow();
  int run_high = iov.getRunLow() - 1;
  EventMetaData ev(1, run_high, exp);
  IntervalOfValidity* prevIoV;
  payload.read(&prevIoV, ev);
  int run_low = prevIoV->getRunLow();
  IntervalOfValidity ciov(exp, run_low,
                          exp, run_high);
  payload.changeIoV(ev, ciov);
  delete prevIoV;
}
// Save results.
void ECLLocalRunCalibUnit::writeToDB(
  bool isLocal,
  const std::string& dbName,
  const std::string& payloadName,
  const IntervalOfValidity& iov,
  const int& run,
  const bool& changePrev,
  const bool& addref)
{
  int exp = iov.getExperimentLow();
  ECLCrystalLocalRunCalib data(isNegAmpl());
  if (m_unitData.size() > 0) {
    int nevents = m_unitData[0].getNOfEvents();
    std::vector<int> counts;
    counts.reserve(m_unitData.size());
    callAccGetter<int>(&counts,
                       &ECLLocalRunCalibAcc::getCount);
    std::vector<float> means;
    means.reserve(m_unitData.size());
    callAccGetter<float>(&means,
                         &ECLLocalRunCalibAcc::getMean);
    std::vector<float> stddevs;
    stddevs.reserve(m_unitData.size());
    callAccGetter<float>(&stddevs,
                         &ECLLocalRunCalibAcc::getStdDev);
    data.setNumberOfEvents(nevents);
    data.setNumbersOfAcceptedEvents(counts);
    data.setCalibVector(means, stddevs);
    data.setExpRun(exp, run);
    ECLDBTool payload(isLocal, dbName.c_str(),
                      payloadName.c_str());
    payload.connect();
    if (changePrev) {
      changePreviousIoV(payload, iov);
    }
    payload.write(&data, iov);
    if (addref) {
      markAsRefference(isLocal, dbName,
                       run, iov);
    }
  }
}
