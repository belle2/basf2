/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/pcore/zmq/connections/ZMQSimpleConnection.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>

using namespace Belle2;

ZMQSimpleConnection::ZMQSimpleConnection(const std::string& monitoringAddress,
                                         const std::shared_ptr<ZMQParent>& parent) : ZMQConnectionOverSocket(parent)
{
  m_socket = m_parent->createSocket<ZMQ_ROUTER>(monitoringAddress, true);

  log("monitoring_counter", 0l);
}

std::unique_ptr<ZMQIdMessage> ZMQSimpleConnection::handleIncomingData()
{
  auto message = ZMQMessageFactory::fromSocket<ZMQIdMessage>(m_socket);
  return message;
}

void ZMQSimpleConnection::handleEvent(std::unique_ptr<ZMQIdMessage> message)
{
  increment("monitoring_counter");
  ZMQParent::send(m_socket, std::move(message));
}