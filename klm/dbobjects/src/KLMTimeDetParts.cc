/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jincheng Mei                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <klm/dbobjects/KLMTimeDetParts.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMTimeDetParts::KLMTimeDetParts() :
  m_effLightSpeed(0.0),
  m_effLightSpeedRPC(0.0),
  m_ampTimeConstant(0.0),
  m_ampTimeConstantRPC(0.0)
{
}

KLMTimeDetParts::~KLMTimeDetParts()
{
}

double KLMTimeDetParts::getEffLightSpeed(bool isRPC) const
{
  if (isRPC) {
    return m_effLightSpeedRPC;
  } else {
    return m_effLightSpeed;
  }
}

void KLMTimeDetParts::setEffLightSpeed(double lightSpeed, bool isRPC)
{
  if (isRPC) {
    m_effLightSpeedRPC = lightSpeed;
  } else {
    m_effLightSpeed = lightSpeed;
  }
}

double KLMTimeDetParts::getAmpTimeConstant(bool isRPC) const
{
  if (isRPC) {
    return m_ampTimeConstantRPC;
  } else {
    return m_ampTimeConstant;
  }
}

void KLMTimeDetParts::setAmpTimeConstant(double amplitudeTimeConstant, bool isRPC)
{
  if (isRPC) {
    m_ampTimeConstantRPC = amplitudeTimeConstant;
  } else {
    m_ampTimeConstant = amplitudeTimeConstant;
  }
}

