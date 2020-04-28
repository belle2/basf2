/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/pcore/zmq/connections/ZMQNullConnection.h>

using namespace Belle2;

void ZMQNullConnection::handleIncomingData()
{
  B2FATAL("There should be no data coming here!");
}

std::vector<zmq::socket_t*> ZMQNullConnection::getSockets() const
{
  return {};
}