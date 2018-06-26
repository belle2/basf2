#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>

using namespace Belle2;

void ZMQNoIdMessage::toDataStore(std::unique_ptr<ZMQNoIdMessage> message, const std::unique_ptr<DataStoreStreamer>& streamer)
{
  // TODO: include the random generator here
  B2ASSERT("The message can not be an end/ready message for streaming!",
           message->isMessage(c_MessageTypes::c_eventMessage));

  EvtMessage eventMessage(message->getMessagePartAsCharArray<ZMQNoIdMessage::c_data>());
  streamer->restoreDataStore(&eventMessage);
}

/// Get the data as string
std::string ZMQNoIdMessage::getData() const
{
  B2ASSERT("The message is an event message",
           not isMessage(c_MessageTypes::c_eventMessage));
  return getMessagePartAsString<c_data>();
}

/// The if the message is of a given type
bool ZMQNoIdMessage::isMessage(const c_MessageTypes isType) const
{
  const auto& type = getMessagePartAsString<c_type>();
  return type.size() == 1 and type[0] == static_cast<char>(isType);
}