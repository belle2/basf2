/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jincheng Mei                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own headers. */
#include <klm/dbobjects/KLMTimeConstants.h>

/* Belle2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMTimeConstants::KLMTimeConstants() :
  m_effLightSpeed(0.0),
  m_effLightSpeedRPC(0.0),
  m_ampTimeConstant(0.0),
  m_ampTimeConstantRPC(0.0)
{
}

KLMTimeConstants::~KLMTimeConstants()
{
}

double KLMTimeConstants::getEffLightSpeed(bool isRPC) const
{
  if (isRPC) {
    return m_effLightSpeedRPC;
  } else {
    return m_effLightSpeed;
  }
}

void KLMTimeConstants::setEffLightSpeed(double lightSpeed, bool isRPC)
{
  if (isRPC) {
    m_effLightSpeedRPC = lightSpeed;
  } else {
    m_effLightSpeed = lightSpeed;
  }
}

double KLMTimeConstants::getAmpTimeConstant(bool isRPC) const
{
  if (isRPC) {
    return m_ampTimeConstantRPC;
  } else {
    return m_ampTimeConstant;
  }
}

void KLMTimeConstants::setAmpTimeConstant(double amplitudeTimeConstant, bool isRPC)
{
  if (isRPC) {
    m_ampTimeConstantRPC = amplitudeTimeConstant;
  } else {
    m_ampTimeConstant = amplitudeTimeConstant;
  }
}

