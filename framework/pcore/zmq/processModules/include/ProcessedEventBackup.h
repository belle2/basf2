/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <memory>
#include <chrono>

namespace Belle2 {
  /// Storage item for the event backup storing the event message, the time stamp and the event meta data
  class ProcessedEventBackup {
    /// Short for the class of the time stamp (it is a system clock time stamp)
    using TimeStamp = std::chrono::time_point<std::chrono::system_clock>;
  public:
    /// Constructor setting the informations. Takes ownership of the evtMsg.
    ProcessedEventBackup(std::unique_ptr<EvtMessage> evtMsg, const EventMetaData& evtMetaData,
                         unsigned int workerId);

    /// Compare operator needed in the backup list
    bool operator==(const EventMetaData& rhs);

    /// Publish this event backup directly to the given client.
    template <class AZMQClient>
    void sendToSocket(const AZMQClient& socket);

    /// Getter for the stored event meta data
    const EventMetaData& getEventMetaData() const;
    /// Getter for the stored time stamp
    const TimeStamp& getTimestamp() const;
    /// Getter for the stored worker id
    const unsigned int& getWorkerId() const;

  private:
    /// Stored event message
    std::unique_ptr<EvtMessage> m_eventMessage;
    /// Getter for the stored event meta data
    EventMetaData m_eventMetaData;
    /// Getter for the stored worker id
    unsigned int m_workerId;
    /// Getter for the stored time stamp
    TimeStamp m_timestamp = std::chrono::system_clock::now();
  };

  template <class AZMQClient>
  void ProcessedEventBackup::sendToSocket(const AZMQClient& socket)
  {
    auto message = ZMQMessageFactory::createMessage(EMessageTypes::c_eventMessage, m_eventMessage);
    socket.publish(std::move(message));
    B2DEBUG(100, "sent backup evt: " << m_eventMetaData.getEvent());
  }
}
