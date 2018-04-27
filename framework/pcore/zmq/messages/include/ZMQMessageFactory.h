#pragma once

#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>
#include <framework/pcore/zmq/messages/ZMQIdMessage.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <memory>
#include <string>

namespace Belle2 {
  class ZMQMessageFactory {
  public:
    static std::unique_ptr<ZMQIdMessage> createMessage(const std::string& msgIdentity,
                                                       const c_MessageTypes msgType,
                                                       const std::string& msgData = "")
    {
      return std::unique_ptr<ZMQIdMessage>(new ZMQIdMessage(msgIdentity, msgType, msgData));
    }


    static std::unique_ptr<ZMQIdMessage> createMessage(const std::string& msgIdentity,
                                                       const c_MessageTypes msgType,
                                                       const std::unique_ptr<DataStoreStreamer>& streamer)
    {
      std::unique_ptr<EvtMessage> eventMessage(streamer->streamDataStore(true, true));
      return std::unique_ptr<ZMQIdMessage>(new ZMQIdMessage(msgIdentity, msgType, eventMessage));
    }

    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType,
                                                         const std::string& msgData = "")
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage(msgType, msgData));
    }


    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType,
                                                         const std::unique_ptr<DataStoreStreamer>& streamer)
    {
      std::unique_ptr<EvtMessage> eventMessage(streamer->streamDataStore(true, true));
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage(msgType, eventMessage));
    }

  };
}