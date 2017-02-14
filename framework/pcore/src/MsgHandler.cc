//+
// File : MsgHandler.cc
// Description : Encode/Decode EvtMessage
//
// Author : SooHyung Lee, Ryosuke Itoh
// Date : 31 - Jul - 2008
// Modified : 4 - Jun - 2010
//-

#include <framework/pcore/MsgHandler.h>
#include <framework/logging/Logger.h>

#include <TObject.h>
#include <TMessage.h>

#include <stdlib.h>
#include <memory>


namespace {
  /** Warn if a streamed object is larger than this. */
  const static int c_maxObjectSizeBytes = 50000000; //50MB
}

using namespace std;
using namespace Belle2;

MsgHandler::MsgHandler(int complevel):
  m_buf(100000)
{
  if (complevel != 0) {
    B2FATAL("Compression support disabled because of https://sft.its.cern.ch/jira/browse/ROOT-4550 . You can enable it manually by removing this check in MsgHandler, but be aware of huge memory leaks.");
  }
  m_complevel = complevel;

  //Schema evolution is needed to stream genfit tracks
  //If disabled, streamers will crash when reading data.
  TMessage::EnableSchemaEvolutionForAll();
}

MsgHandler::~MsgHandler()
{
}

void MsgHandler::clear()
{
  m_buf.clear();
}

bool MsgHandler::add(const TObject* obj, const string& name)
{
  static std::unique_ptr<TMessage> msg(new TMessage(kMESS_OBJECT));
  msg->Reset();
  msg->SetWriteMode();
  msg->SetCompressionLevel(m_complevel);

  // Write object in TMessage
  msg->WriteObject(obj);
  msg->Compress(); //no effect if m_complevel == 0
  //  msg->ForceWriteInfo(obj->, true );

  int len = msg->Length();
  char* buf = msg->Buffer();
  if (msg->CompBuffer()) { // Compression ON
    len = msg->CompLength();
    buf = msg->CompBuffer();
  }
  //  printf ( "MsgHandler : size of %s = %d (pid=%d)\n", name.c_str(), len, (int)getpid() );

  if (len > c_maxObjectSizeBytes) {
    B2WARNING("MsgHandler: Object " << name << " is very large (" << len  << " bytes), parallel processing may be slow.");
  }

  // Put name of object in output buffer
  UInt_t nameLength = name.size() + 1;
  m_buf.add(&nameLength, sizeof(nameLength));
  m_buf.add(name.c_str(), nameLength);
  // Copy object into buffer
  m_buf.add(&len, sizeof(len));
  m_buf.add(buf, len);
  return true;
}

EvtMessage* MsgHandler::encode_msg(RECORD_TYPE rectype)
{
  if (rectype == MSG_TERMINATE) {
    EvtMessage* eod = new EvtMessage(NULL, 0, rectype);
    return eod;
  }

  EvtMessage* evtmsg = new EvtMessage(m_buf.data(), m_buf.size(), rectype);
  clear();

  return evtmsg;
}

int MsgHandler::decode_msg(EvtMessage* msg, vector<TObject*>& objlist,
                           vector<string>& namelist)
{
  int totlen = 0;
  char* msgptr = msg->msg();

  InMessage tmsg;
  while (totlen < msg->msg_size()) {
    // Restore object name
    UInt_t nameLength;
    memcpy(&nameLength, msgptr, sizeof(nameLength));
    string name((char*)(msgptr + sizeof(nameLength)));
    namelist.push_back(name);
    msgptr += (sizeof(nameLength) + nameLength);
    totlen += (sizeof(nameLength) + nameLength);

    // Restore object
    UInt_t objlen;
    memcpy(&objlen, msgptr, sizeof(objlen));

    tmsg.SetBuffer(msgptr + sizeof(objlen), objlen);
    TObject* obj = static_cast<TObject*>(tmsg.ReadObjectAny(tmsg.GetClass()));
    objlist.push_back(obj);

    msgptr += objlen + sizeof(objlen);
    totlen += objlen + sizeof(objlen);

    tmsg.Reset();
  }
  return 0;
}
