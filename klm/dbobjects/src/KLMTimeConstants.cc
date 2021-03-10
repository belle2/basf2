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

float KLMTimeConstants::getDelay(int cType) const
{
  switch (cType) {
    case c_EKLM:
      return m_DelayEKLMScintillators;
    case c_BKLM:
      return m_DelayBKLMScintillators;
    case c_RPCPhi:
      return m_DelayRPCPhi;
    case c_RPCZ:
      return m_DelayRPCZ;
  }
  B2FATAL("Incorrect channel type: " << cType);
}

void KLMTimeConstants::setDelay(float delay, int cType)
{
  switch (cType) {
    case c_EKLM:
      m_DelayEKLMScintillators = delay;
      return;
    case c_BKLM:
      m_DelayBKLMScintillators = delay;
      return;
    case c_RPCPhi:
      m_DelayRPCPhi = delay;
      return;
    case c_RPCZ:
      m_DelayRPCZ = delay;
      return;
  }
  B2FATAL("Incorrect channel type: " << cType);
}

float KLMTimeConstants::getAmpTimeConstant(int cType) const
{
  switch (cType) {
    case c_EKLM:
      return m_ampTimeConstant_end;
    case c_BKLM:
      return m_ampTimeConstant;
    case c_RPCPhi:
    case c_RPCZ:
      return m_ampTimeConstantRPC;
  }
  B2FATAL("Incorrect channel type: " << cType);
}

void KLMTimeConstants::setAmpTimeConstant(float amplitudeTimeConstant, int cType)
{
  switch (cType) {
    case c_EKLM:
      m_ampTimeConstant_end = amplitudeTimeConstant;
      return;
    case c_BKLM:
      m_ampTimeConstant = amplitudeTimeConstant;
      return;
    case c_RPCPhi:
    case c_RPCZ:
      m_ampTimeConstantRPC = amplitudeTimeConstant;
      return;
  }
  B2FATAL("Incorrect channel type: " << cType);
}

