/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jincheng Mei                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/KLMTimeConstants.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMTimeConstants::KLMTimeConstants() :
  m_effLightSpeed_end(0.0),
  m_effLightSpeed(0.0),
  m_effLightSpeedRPC(0.0),
  m_ampTimeConstant_end(0.0),
  m_ampTimeConstant(0.0),
  m_ampTimeConstantRPC(0.0)
{
}

KLMTimeConstants::~KLMTimeConstants()
{
}

double KLMTimeConstants::getEffLightSpeed(int cType) const
{
  switch (cType) {
    case c_EKLM:
      return m_effLightSpeed_end;
    case c_BKLM:
      return m_effLightSpeed;
    case c_RPC:
      return m_effLightSpeedRPC;
  }
  B2FATAL("Incorrect channel type: " << cType);
}

void KLMTimeConstants::setEffLightSpeed(double lightSpeed, int cType)
{
  switch (cType) {
    case c_EKLM:
      m_effLightSpeed_end = lightSpeed;
      return;
    case c_BKLM:
      m_effLightSpeed = lightSpeed;
      return;
    case c_RPC:
      m_effLightSpeedRPC = lightSpeed;
      return;
  }
  B2FATAL("Incorrect channel type: " << cType);
}

double KLMTimeConstants::getAmpTimeConstant(int cType) const
{
  switch (cType) {
    case c_EKLM:
      return m_ampTimeConstant_end;
    case c_BKLM:
      return m_ampTimeConstant;
    case c_RPC:
      return m_ampTimeConstantRPC;
  }
  B2FATAL("Incorrect channel type: " << cType);
}

void KLMTimeConstants::setAmpTimeConstant(double amplitudeTimeConstant, int cType)
{
  switch (cType) {
    case c_EKLM:
      m_ampTimeConstant_end = amplitudeTimeConstant;
      return;
    case c_BKLM:
      m_ampTimeConstant = amplitudeTimeConstant;
      return;
    case c_RPC:
      m_ampTimeConstantRPC = amplitudeTimeConstant;
      return;
  }
  B2FATAL("Incorrect channel type: " << cType);
}

