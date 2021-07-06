/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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