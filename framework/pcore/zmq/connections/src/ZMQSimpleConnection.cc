/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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