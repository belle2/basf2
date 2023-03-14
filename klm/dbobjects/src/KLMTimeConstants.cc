/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/KLMTimeConstants.h>

/* Basf2 headers. */
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
