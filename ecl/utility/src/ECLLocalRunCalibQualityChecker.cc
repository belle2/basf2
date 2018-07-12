/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * ECLLocalRunCalibQualityChecker                                         *
 *                                                                        *
 * This class has been designed in order to check quality of local run    *
 * calibration.                                                           *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <math.h>
// ECL
#include <ecl/utility/ECLLocalRunCalibQualityChecker.h>
using namespace Belle2;
ECLLocalRunCalibQualityChecker::ECLLocalRunCalibQualityChecker(bool isTime):
  m_isTime(isTime), m_minNOfEvs(0), m_badCountRatio(0.),
  m_largeOffsetValue(0.), m_minMeanValue(0.),
  m_maxMeanValue(0.), m_nBadCountCh(0),
  m_nLargeOffsetCh(0), m_nChOutsideOfLimits(0),
  m_tooSmallNOfEvs(false), m_isNegAmpl(false)
{
}
ECLLocalRunCalibQualityChecker::~ECLLocalRunCalibQualityChecker()
{
}
void ECLLocalRunCalibQualityChecker::setMinNOfEvs(const int& minNOfEvs)
{
  m_minNOfEvs = minNOfEvs;
}
void ECLLocalRunCalibQualityChecker::setBadCountRatio(
  const float& badCountRatio)
{
  m_badCountRatio = badCountRatio;
}
void ECLLocalRunCalibQualityChecker::setBadOffsetValue(
  const float& largeOffsetValue)
{
  m_largeOffsetValue = largeOffsetValue;
}
void ECLLocalRunCalibQualityChecker::setMeanValueLimits(
  const float& minMeanValue,
  const float& maxMeanValue)
{
  m_minMeanValue = minMeanValue;
  m_maxMeanValue = maxMeanValue;
}

bool ECLLocalRunCalibQualityChecker::checkNOfEvs(
  const ECLCrystalLocalRunCalib* const payload)
{
  int nOfEvents = payload->getNumberOfEvents();
  if (nOfEvents < m_minNOfEvs) {
    m_tooSmallNOfEvs = true;
    return true;
  }
  return false;
}
bool ECLLocalRunCalibQualityChecker::checkCount(
  const ECLCrystalLocalRunCalib* const payload)
{
  bool result = false;
  int nOfEvents = payload->getNumberOfEvents();
  for (const auto& count : payload->getNumbersOfAcceptedEvents()) {
    if (static_cast<float>(count) / nOfEvents < m_badCountRatio) {
      m_nBadCountCh++;
      result = true;
    }
  }
  return result;
}
bool ECLLocalRunCalibQualityChecker::checkOffset(
  const ECLCrystalLocalRunCalib* const payload,
  const ECLCrystalLocalRunCalib* const ref_payload)
{
  bool result = false;
  std::size_t ncellid = payload->getCalibVector().size();
  float offset;
  for (std::size_t i = 0; i < ncellid; ++i) {
    if (m_isTime) {
      offset = fabs(payload->getCalibVector()[i] -
                    ref_payload->getCalibVector()[i]);
    } else {
      offset = fabs(payload->getCalibVector()[i] /
                    ref_payload->getCalibVector()[i] - 1);
    }
    if (offset > m_largeOffsetValue) {
      m_nLargeOffsetCh++;
      result = true;
    }
  }
  return result;
}
bool ECLLocalRunCalibQualityChecker::checkValueLimits(
  const ECLCrystalLocalRunCalib* const payload)
{
  bool result = false;
  for (const auto& el : payload->getCalibVector()) {
    if (el < m_minMeanValue || el > m_maxMeanValue) {
      m_nChOutsideOfLimits++;
      result = true;
    }
  }
  return result;
}
bool ECLLocalRunCalibQualityChecker::checkNegAmpls(
  const ECLCrystalLocalRunCalib* const payload)
{
  m_isNegAmpl = payload->isNegAmpl();
  return payload->isNegAmpl();
}
bool ECLLocalRunCalibQualityChecker::checkQuality(
  const ECLCrystalLocalRunCalib* const payload,
  const ECLCrystalLocalRunCalib* const ref_payload)
{
  bool flag00 = checkNOfEvs(payload);
  bool flag01 = checkCount(payload);
  bool flag02 = checkOffset(payload, ref_payload);
  bool flag03 = checkValueLimits(payload);
  if (m_isTime) {
    return flag00 || flag01 || flag02 || flag03;
  }
  bool flag04 = checkNegAmpls(payload);
  return flag00 || flag01 || flag02 || flag03 || flag04;
}
void ECLLocalRunCalibQualityChecker::reset()
{
  m_nBadCountCh = 0;
  m_nLargeOffsetCh = 0;
  m_nChOutsideOfLimits = 0;
  m_tooSmallNOfEvs = false;
  m_isNegAmpl = false;
}
int ECLLocalRunCalibQualityChecker::getNBadCountCh() const
{
  return m_nBadCountCh;
}
int ECLLocalRunCalibQualityChecker::getNLargeOffsetCh() const
{
  return m_nLargeOffsetCh;
}
int ECLLocalRunCalibQualityChecker::getNChOutsideLimits() const
{
  return m_nChOutsideOfLimits;
}
bool ECLLocalRunCalibQualityChecker::isTooSmallNOfEvents() const
{
  return m_tooSmallNOfEvs;
}
bool ECLLocalRunCalibQualityChecker::isNegAmpl() const
{
  return m_isNegAmpl;
}
