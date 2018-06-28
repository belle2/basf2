/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * ECLLocalRunCalibAcc                                                    *
 *                                                                        *
 * Feature (mean value, standard deviation and number of accepted         *
 * events) accumulator.                                                   *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su),               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <math.h>
// ECL
#include <ecl/modules/eclLocalRunCalibration/ECLLocalRunCalibAcc.h>
// STL
#include <algorithm>
// BOOST
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
using namespace Belle2;
// Constructor.
ECLLocalRunCalibAcc::ECLLocalRunCalibAcc(
  const float& min_value,
  const float& max_value,
  const int* const ndevs):
  m_leftLimit(min_value),
  m_rightLimit(max_value),
  c_ndevs(ndevs),
  m_nevents(0),
  m_count(0),
  m_mean(0.),
  m_stddev(0.)
{
  m_data.reserve(1000);
}
// Destructor.
ECLLocalRunCalibAcc::~ECLLocalRunCalibAcc()
{
}
// Check value.
bool ECLLocalRunCalibAcc::isValueInRange(const float& value) const
{
  return (value > m_leftLimit) &&
         (value < m_rightLimit);
}
// Add value.
void ECLLocalRunCalibAcc::add(const float& value)
{
  m_data.push_back(value);
}
// Calculate standard deviation
// using variance and number of
// accepted events.
float ECLLocalRunCalibAcc::calcStdDev(const float& variance,
                                      const int& count) const
{
  return sqrt((variance * count) / (count - 1));
}
// Update value lowe and upper limits.
void ECLLocalRunCalibAcc::updateLimits(const float& mean,
                                       const float& stddev)
{
  m_leftLimit = std::max(m_leftLimit,
                         mean - (*c_ndevs) * stddev);
  m_rightLimit = std::min(m_rightLimit,
                          mean + (*c_ndevs) * stddev);
}
// Calculate accumulated features.
void ECLLocalRunCalibAcc::calc()
{
  namespace bacc = boost::accumulators;
  bacc::accumulator_set <
  float,
  bacc::features <
  bacc::tag::count,
  bacc::tag::median,
  bacc::tag::variance >> acc;
  for (const auto& value : m_data) {
    if (isValueInRange(value)) {
      acc(value);
    }
  }
  float tcount = bacc::count(acc);
  float tmedian = bacc::median(acc);
  float tvariance = bacc::variance(acc);
  float tstddev = calcStdDev(tvariance,
                             tcount);
  updateLimits(tmedian, tstddev);
  bacc::accumulator_set <
  float,
  bacc::features <
  bacc::tag::count,
  bacc::tag::mean,
  bacc::tag::variance >> acc_final;
  for (const auto& value : m_data) {
    if (isValueInRange(value)) {
      acc_final(value);
    }
  }
  m_nevents = static_cast<int>(m_data.size());
  m_data.clear();
  m_count = bacc::count(acc_final);
  m_mean = bacc::mean(acc_final);
  tvariance = bacc::variance(acc_final);
  m_stddev = calcStdDev(tvariance, m_count);
}
// Get total number of events.
int ECLLocalRunCalibAcc::getNOfEvents() const
{
  return m_nevents;
}
// Get number of accepted events.
int ECLLocalRunCalibAcc::getCount() const
{
  return m_count;
}
// Get mean value.
float ECLLocalRunCalibAcc::getMean() const
{
  return m_mean;
}
// Get standard deviation.
float ECLLocalRunCalibAcc::getStdDev() const
{
  return m_stddev;
}
