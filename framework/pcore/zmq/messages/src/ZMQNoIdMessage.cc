/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>

#include <framework/logging/Logger.h>

using namespace Belle2;

/// Get the data as string
std::string ZMQNoIdMessage::getData() const
{
  B2ASSERT("The message is an event message",
           not isMessage(EMessageTypes::c_eventMessage));
  return getMessagePartAsString<c_data>();
}

/// The if the message is of a given type
bool ZMQNoIdMessage::isMessage(const EMessageTypes isType) const
{
  const auto& type = getMessagePartAsString<c_type>();
  return type.size() == 1 and type[0] == static_cast<char>(isType);
}
