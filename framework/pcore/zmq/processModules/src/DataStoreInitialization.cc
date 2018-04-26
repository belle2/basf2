#include <framework/pcore/zmq/processModules/DataStoreInitialization.h>

#include <framework/pcore/SeqFile.h>
#include <framework/core/Environment.h>
#include <framework/logging/LogMethod.h>
#include <framework/pcore/DataStoreStreamer.h>

#include <TSystem.h>

using namespace std;
using namespace Belle2;

void DataStoreInitialization::initializeDataStore(const std::string& inputFileName)
{

  std::unique_ptr<SeqFile> file(new SeqFile(inputFileName.c_str(), "r"));

  DataStoreInitialization::initializeDataStore(file);
}

void DataStoreInitialization::initializeDataStore(const unique_ptr<SeqFile>& file)
{
  // TODO
  gSystem->Load("libdataobjects");
  gSystem->Load("libTreePlayer");
  gSystem->Load("libgenfit2");    // Because genfit2 classes need custom streamers.
  gSystem->Load("libvxd");
  gSystem->Load("libsvd");
  gSystem->Load("libpxd");
  gSystem->Load("libcdc");

  std::unique_ptr<DataStoreStreamer> streamer(new DataStoreStreamer());

  // Open input file
  if (file->status() <= 0) {
    B2FATAL("SeqRootInput : Error in opening input file");
  }

  //Read StreamerInfo and the first event
  int info_cnt = 0;
  while (true) {
    std::unique_ptr<char[]> evtbuf(new char[EvtMessage::c_MaxEventSize]);
    int size = file->read(evtbuf.get(), EvtMessage::c_MaxEventSize);
    if (size > 0) {
      std::unique_ptr<EvtMessage> evtmsg(new EvtMessage(evtbuf.get()));
      streamer->restoreDataStore(evtmsg.get());
      if (evtmsg->type() == MSG_STREAMERINFO) {
        // StreamerInfo was read
        B2INFO("Reading StreamerInfo");
        if (info_cnt != 0) B2FATAL("SeqRootInput : Reading StreamerInfos twice");
        info_cnt++;
      } else {
        // first event was read
        break;
      }
    } else {
      B2FATAL("SeqRootInput : Error in reading first event");
    }
  }

  B2INFO("SeqRootInput: initialized.");
}