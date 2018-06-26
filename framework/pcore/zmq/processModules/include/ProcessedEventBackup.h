/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Anselm Baur                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/logging/LogMethod.h>
#include <memory>
#include <chrono>
#include <vector>


namespace Belle2 {
  /// Storage item for the event backup storing the event message, the time stamp and the event meta data
  class ProcessedEventBackup {
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
    auto message = ZMQMessageFactory::createMessage(c_MessageTypes::c_eventMessage, m_eventMessage);
    socket.publish(std::move(message));
    B2DEBUG(100, "sent backup evt: " << m_eventMetaData.getEvent());
  }
}
