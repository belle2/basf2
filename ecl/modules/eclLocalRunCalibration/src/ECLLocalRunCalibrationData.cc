/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Local run Digit Calibration.                                           *
 *                                                                        *
 * This class wraps Boost accumulator to provide mean and standard        *
 * deviation for repeated measurements (either amplitude or time          *
 * measurements) concerning single specific crystal in ECL.               *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su),               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <math.h>
#include <cmath>
#include <algorithm>
#include <ecl/modules/eclLocalRunCalibration/ECLLocalRunCalibrationData.h>

using namespace Belle2;

ECLLocalRunCalibrationData::ECLLocalRunCalibrationData(
  const float& minValue,
  const float& maxValue,
  const int& nOfInitEvs,
  const int& maxNOfDevs):
  m_isMedianNotCalculated(true),
  m_nTotalInputs(0),
  m_isOffset(false),
  m_leftLimit(minValue),
  m_rightLimit(maxValue),
  c_nOfInitEvs(nOfInitEvs),
  c_maxNOfDevs(maxNOfDevs)
{
}

ECLLocalRunCalibrationData::~ECLLocalRunCalibrationData()
{
}

void ECLLocalRunCalibrationData::enableOffsetFlag()
{
  m_isOffset = true;
}

bool ECLLocalRunCalibrationData::getOffsetFlag() const
{
  return m_isOffset;
}

int ECLLocalRunCalibrationData::getNOfAcceptedInputs() const
{
  return bacc::count(m_acc);
}

int ECLLocalRunCalibrationData::getNOfRejectedInputs() const
{
  return m_nTotalInputs - bacc::count(m_acc);
}

float ECLLocalRunCalibrationData::getMean() const
{
  return bacc::mean(m_acc);
}

float ECLLocalRunCalibrationData::getStdDev() const
{
  return sqrt(
           (bacc::variance(m_acc) *
            bacc::count(m_acc)) /
           (bacc::count(m_acc) - 1));
}

void ECLLocalRunCalibrationData::calcMedian()
{
  std::size_t n = m_initValues.size() / 2;
  std::nth_element(m_initValues.begin(),
                   m_initValues.begin() + n,
                   m_initValues.end());
  float median = m_initValues[n];
  auto acc = bacc::accumulator_set <
             float,
             bacc::features <
             bacc::tag::count,
             bacc::tag::variance >> ();
  for (const auto& value : m_initValues) {
    acc(value);
  }
  float delta = c_maxNOfDevs * sqrt(
                  (bacc::variance(acc) *
                   bacc::count(acc)) /
                  (bacc::count(acc) - 1));
  m_leftLimit = std::max(m_leftLimit, median - delta);
  m_rightLimit = std::min(m_rightLimit, median + delta);
}

void ECLLocalRunCalibrationData::update(const float& value)
{
  m_acc(value);
}

bool ECLLocalRunCalibrationData::isValueInRange(const float& value) const
{
  return (value > m_leftLimit) &&
         (value < m_rightLimit);
}

void ECLLocalRunCalibrationData::processInitialEvents()
{
  for (const auto& value : m_initValues) {
    if (isValueInRange(value)) {
      update(value);
    }
  }
}

void ECLLocalRunCalibrationData::add(const float& value)
{
  m_nTotalInputs++;
  if (!isValueInRange(value)) {
    return;
  }
  if (m_initValues.size() == (unsigned) c_nOfInitEvs) {
    m_isMedianNotCalculated = false;
    calcMedian();
    processInitialEvents();
    m_acc = bacc::accumulator_set <
            float,
            bacc::features <
            bacc::tag::count,
            bacc::tag::mean,
            bacc::tag::variance >> ();
    processInitialEvents();
    m_initValues.clear();
  }
  if (m_isMedianNotCalculated) {
    m_initValues.emplace_back(value);
  } else {
    update(value);
  }
}

void ECLLocalRunCalibrationData::stop()
{
  if (m_isMedianNotCalculated) {
    calcMedian();
    processInitialEvents();
    m_acc = bacc::accumulator_set <
            float,
            bacc::features <
            bacc::tag::count,
            bacc::tag::mean,
            bacc::tag::variance >> ();
    processInitialEvents();
    m_initValues.clear();
  }
}
