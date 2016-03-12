/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMTimeCalibrationData.h>

using namespace Belle2;

EKLMTimeCalibrationData::EKLMTimeCalibrationData()
{
  m_TimeShift = 0;
}

EKLMTimeCalibrationData::EKLMTimeCalibrationData(double timeShift)
{
  m_TimeShift = timeShift;
}

EKLMTimeCalibrationData::~EKLMTimeCalibrationData()
{
}

void EKLMTimeCalibrationData::setTimeShift(double timeShift)
{
  m_TimeShift = timeShift;
}

double EKLMTimeCalibrationData::getTimeShift()
{
  return m_TimeShift;
}

