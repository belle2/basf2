/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                 *
 * Copyright(C) 2016  Belle II Collaboration                          *
 *                                                                    *
 * Author: The Belle II Collaboration                                 *
 * Contributors: Kirill Chilikin                                      *
 *                                                                    *
 * This software is provided "as is" without any warranty.            *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMReconstructionParameters.h>

using namespace Belle2;

EKLMReconstructionParameters::EKLMReconstructionParameters()
{
}

EKLMReconstructionParameters::~EKLMReconstructionParameters()
{
}

float EKLMReconstructionParameters::getTimeResolution() const
{
  return m_TimeResolution;
}

void EKLMReconstructionParameters::setTimeResolution(float resolution)
{
  m_TimeResolution = resolution;
}

