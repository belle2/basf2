/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/expreco/ERecoRunControlCallback.h"

#include "daq/expreco/ERecoMasterCallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>

using namespace Belle2;

ERecoRunControlCallback::ERecoRunControlCallback(ERecoMasterCallback* callback)
  : m_callback(callback)
{
  callback->setCallback(this);
  allocData(getNode().getName() + "_STATUS", "erunitinfo", 3);
}

void ERecoRunControlCallback::initialize(const DBObject&)
{
  allocData(getNode().getName() + "_STATUS", "erunitinfo", 3);
}

bool ERecoRunControlCallback::perform(NSMCommunicator& com)
{
  NSMCommand cmd(com.getMessage().getRequestName());
  m_callback->perform(com);
  if (cmd == NSMCommand::VGET || cmd == NSMCommand::VSET) {
    return RCCallback::perform(com);
  }
  return true;
}

