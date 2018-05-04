#include <framework/pcore/zmq/processModules/ZMQRxSeqRootOutputModule.h>

#include <framework/pcore/MsgHandler.h>
#include <TClass.h>
#include <TList.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ZMQRxSeqRootOutput)


void ZMQRxSeqRootOutputModule::writeStreamerInfo()
{
  //
  // Write StreamerInfo to a file
  // Copy from RootOutputModule::initialize() and TSocket::SendStreamerInfos()
  //

  std::unique_ptr<MsgHandler> messageHandler = std::unique_ptr<MsgHandler>(new MsgHandler(m_param_compressionLevel));

  std::unique_ptr<TList> minilist;
  for (int durability = 0; durability < DataStore::c_NDurabilityTypes; durability++) {
    DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(DataStore::EDurability(durability));

    for (DataStore::StoreEntryIter iter = map.begin(); iter != map.end(); ++iter) {
      const TClass* entryClass = iter->second.objClass;
      TVirtualStreamerInfo* vinfo = entryClass->GetStreamerInfo();
      B2INFO("Recording StreamerInfo : durability " << durability << " : Class Name " << entryClass->GetName());
      if (not minilist) {
        minilist = std::unique_ptr<TList>(new TList());
      }
      minilist->Add((TObject*) vinfo);
    }
  }

  if (minilist) {
    messageHandler->add(minilist.get(), "StreamerInfo");
    EvtMessage* msg = messageHandler->encode_msg(MSG_STREAMERINFO);
    (msg->header())->nObjects = 1;       // No. of objects
    (msg->header())->nArrays = 0;    // No. of arrays
    int size = m_seqRootFile->write(msg->buffer());
    B2INFO("Wrote StreamerInfo to a file : " << size << "bytes");
  }
}


void ZMQRxSeqRootOutputModule::writeEvent(const std::unique_ptr<ZMQNoIdMessage>& message)
{
  // TODO: Needs to have information on data store
  // layout in initialize! And: initialize is in parallel
  // output process only.

  if (m_firstEvent) {
    writeStreamerInfo();
  }

  message->toSeqFile(m_seqRootFile);

  if (not m_eventMetaData.isValid()) {
    m_eventMetaData.create();
  }
}