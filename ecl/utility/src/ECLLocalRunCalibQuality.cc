/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * ECLLocalRunCalibQuality                                                *
 *                                                                        *
 * This class contains information about quality of local run calibration.*
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// ECL
#include <ecl/utility/ECLLocalRunCalibQuality.h>
using namespace Belle2;
ECLLocalRunCalibQuality::ECLLocalRunCalibQuality(const int& exp,
                                                 const int& run,
                                                 const int& runRef):
  m_exp(exp), m_run(run), m_runRef(runRef), m_nOfEvents(0),
  m_nOfBadTimeCountCh(0), m_nOfBadAmplCountCh(0),
  m_nOfLargeTimeOffsetCh(0), m_nOfLargeAmplOffsetCh(0),
  m_tooSmallNOfEvs(false), m_isNegAmpl(false)
{
}
ECLLocalRunCalibQuality::ECLLocalRunCalibQuality(
  const ECLLocalRunCalibQuality& quality)
{
  m_exp = quality.m_exp;
  m_run = quality.m_run;
  m_runRef = quality.m_runRef;
  m_nOfEvents = quality.m_nOfEvents;
  m_nOfBadTimeCountCh = quality.m_nOfBadTimeCountCh;
  m_nOfBadAmplCountCh = quality.m_nOfBadAmplCountCh;
  m_nOfLargeTimeOffsetCh = quality.m_nOfLargeTimeOffsetCh;
  m_nOfLargeAmplOffsetCh = quality.m_nOfLargeAmplOffsetCh;
  m_nOfAbsMeanTimeChOutRange = quality.m_nOfAbsMeanTimeChOutRange;
  m_nOfAbsMeanAmplChOutRange = quality.m_nOfAbsMeanAmplChOutRange;
  m_tooSmallNOfEvs = quality.m_tooSmallNOfEvs;
  m_isNegAmpl = quality.m_isNegAmpl;
}
ECLLocalRunCalibQuality::~ECLLocalRunCalibQuality()
{
}
int ECLLocalRunCalibQuality::getExpNumber() const
{
  return m_exp;
}
int ECLLocalRunCalibQuality::getRunNumber() const
{
  return m_run;
}
int ECLLocalRunCalibQuality::getReferenceRunNumber() const
{
  return m_runRef;
}
int ECLLocalRunCalibQuality::getNOfEvents() const
{
  return m_nOfEvents;
}
int ECLLocalRunCalibQuality::getNOfBadTimeCountCh() const
{
  return m_nOfBadTimeCountCh;
}
int ECLLocalRunCalibQuality::getNOfBadAmplCountCh() const
{
  return m_nOfBadAmplCountCh;
}
int ECLLocalRunCalibQuality::getNOfLargeTimeOffsetCh() const
{
  return m_nOfLargeTimeOffsetCh;
}
int ECLLocalRunCalibQuality::getNOfLargeAmplOffsetCh() const
{
  return m_nOfLargeAmplOffsetCh;
}
int ECLLocalRunCalibQuality::getNOfAbsMeanTimeChOutRange() const
{
  return m_nOfAbsMeanTimeChOutRange;
}
int ECLLocalRunCalibQuality::getNOfAbsMeanAmplChOutRange() const
{
  return m_nOfAbsMeanAmplChOutRange;
}
bool ECLLocalRunCalibQuality::isTooSmallNOfEvs() const
{
  return m_tooSmallNOfEvs;
}
bool ECLLocalRunCalibQuality::isNegAmpl() const
{
  return m_isNegAmpl;
}
void ECLLocalRunCalibQuality::setNumberOfEvents(const int& nOfEvents)
{
  m_nOfEvents = nOfEvents;
}
void ECLLocalRunCalibQuality::setNOfBadTimeCountCh(const int& nOfBadTimeCountCh)
{
  m_nOfBadTimeCountCh = nOfBadTimeCountCh;
}
void ECLLocalRunCalibQuality::setNOfBadAmplCountCh(const int& nOfBadAmplCountCh)
{
  m_nOfBadAmplCountCh = nOfBadAmplCountCh;
}
void ECLLocalRunCalibQuality::setNOfLargeTimeOffsetCh(const int& nOfLargeTimeOffsetCh)
{
  m_nOfLargeTimeOffsetCh = nOfLargeTimeOffsetCh;
}
void ECLLocalRunCalibQuality::setNOfLargeAmplOffsetCh(const int& nOfLargeAmplOffsetCh)
{
  m_nOfLargeAmplOffsetCh = nOfLargeAmplOffsetCh;
}
void ECLLocalRunCalibQuality::enableTooSmallNOfEvs()
{
  m_tooSmallNOfEvs = true;
}
void ECLLocalRunCalibQuality::enableNegAmpl()
{
  m_isNegAmpl = true;
}
void ECLLocalRunCalibQuality::setNOfAbsMeanTimeChOutRange(const int& nOfAbsMeanTimeChOutRange)
{
  m_nOfAbsMeanTimeChOutRange = nOfAbsMeanTimeChOutRange;
}
void ECLLocalRunCalibQuality::setNOfAbsMeanAmplChOutRange(const int& nOfAbsMeanAmplChOutRange)
{
  m_nOfAbsMeanAmplChOutRange = nOfAbsMeanAmplChOutRange;
}
