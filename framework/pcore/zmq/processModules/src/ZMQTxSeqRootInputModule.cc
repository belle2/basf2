#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/processModules/ZMQTxSeqRootInputModule.h>

using namespace Belle2;


REG_MODULE(ZMQTxSeqRootInput)

std::unique_ptr<ZMQIdMessage> ZMQTxSeqRootInputModule::readEventToMessage(std::string& NextWorkerID)
{
  std::unique_ptr<char> evtbuf(new char[EvtMessage::c_MaxEventSize]);
  const int readBytes = m_seqRootFile->read(evtbuf.get(), EvtMessage::c_MaxEventSize);
  if (readBytes <= 0) {
    return ZMQMessageFactory::createMessage(NextWorkerID, c_MessageTypes::c_emptyMessage);
  } else {
    auto eventMessage = std::make_unique<EvtMessage>(evtbuf.get());
    if (eventMessage->type() == MSG_STREAMERINFO) {
      return readEventToMessage(NextWorkerID);
    } else {
      if (not m_eventMetaData.isValid()) {
        m_eventMetaData.create();
      }
      return ZMQMessageFactory::createMessage(NextWorkerID, eventMessage);
    }
  }
}