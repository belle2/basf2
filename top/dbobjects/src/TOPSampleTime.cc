/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPSampleTime.h>
#include <iostream>

using namespace std;
using namespace Belle2;

void TOPSampleTime::setTimeAxis(double syncTimeBase)
{
  double DTime = 2 * syncTimeBase;
  double timeBin = DTime / c_TimeAxisSize;
  for (unsigned i = 0; i < c_TimeAxisSize; i++) m_timeAxis[i] = timeBin * i;
  m_timeAxis[c_TimeAxisSize] = DTime;
}


void TOPSampleTime::setTimeAxis(const std::vector<double>& sampleTimes,
                                double syncTimeBase)
{
  if (sampleTimes.size() < c_TimeAxisSize) {
    B2FATAL("TOPSampleTime::setTimeAxis: vector too short");
    return;
  }

  for (unsigned i = 0; i < c_TimeAxisSize; i++) m_timeAxis[i] = sampleTimes[i];
  double DTime = 2 * syncTimeBase;
  m_timeAxis[c_TimeAxisSize] = DTime;
}


float TOPSampleTime::getTime(unsigned window, float sample) const
{

  int sampleNum = int(sample);
  float frac = sample - sampleNum;

  sampleNum += window * c_WindowSize; // counted from window 0
  int n = sampleNum / c_TimeAxisSize;
  int k = sampleNum % c_TimeAxisSize;
  float time = n * getTimeRange() + m_timeAxis[k]; // from sample 0 window 0
  time += (m_timeAxis[k + 1] - m_timeAxis[k]) * frac; // add fraction

  return time;
}


