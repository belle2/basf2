#include <framework/pcore/zeromq/ZeroMQTxSeqRootInputModule.h>

using namespace Belle2;


REG_MODULE(ZeroMQTxSeqRootInput)

std::unique_ptr<ZeroMQRoutedMessage> ZeroMQTxSeqRootInputModule::readEvent()
{
  std::unique_ptr<char> evtbuf(new char[EvtMessage::c_MaxEventSize]);
  const int readBytes = m_seqRootFile->read(evtbuf.get(), EvtMessage::c_MaxEventSize);
  if (readBytes <= 0) {
    return ZeroMQRoutedMessage::createEmptyMessage();
  } else {
    EvtMessage eventMessage(evtbuf.get());
    if (eventMessage.type() == MSG_STREAMERINFO) {
      return readEvent();
    } else {
      if (not m_eventMetaData.isValid()) {
        m_eventMetaData.create();
      }
      return ZeroMQRoutedMessage::fromEventMessage(eventMessage, 0);
    }
  }
}