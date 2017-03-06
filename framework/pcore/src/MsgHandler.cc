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

using namespace std;
using namespace Belle2;


namespace {
  /** Warn if a streamed object is larger than this. */
  const static int c_maxObjectSizeBytes = 50000000; //50MB
}

MsgHandler::MsgHandler(int complevel):
  m_buf(new CharBuffer(100000)),
  m_msg(new TMessage(kMESS_OBJECT))
{
  if (complevel != 0) {
    B2FATAL("Compression support disabled because of https://sft.its.cern.ch/jira/browse/ROOT-4550 . You can enable it manually by removing this check in MsgHandler, but be aware of huge memory leaks.");
  }
  m_complevel = complevel;

  //Schema evolution is needed to stream genfit tracks
  //If disabled, streamers will crash when reading data.
  TMessage::EnableSchemaEvolutionForAll();

  m_msg->SetWriteMode();
  m_msg->SetCompressionLevel(complevel);
}

MsgHandler::~MsgHandler()
{
}

void MsgHandler::clear()
{
  m_buf->clear();
}

void MsgHandler::add(const TObject* obj, const string& name)
{
  m_msg->WriteObject(obj);
  m_msg->Compress(); //no effect if m_complevel == 0

  int len = m_msg->Length();
  char* buf = m_msg->Buffer();
  if (m_msg->CompBuffer()) { // Compression ON
    len = m_msg->CompLength();
    buf = m_msg->CompBuffer();
  }

  if (len > c_maxObjectSizeBytes) {
    B2WARNING("MsgHandler: Object " << name << " is very large (" << len  << " bytes), parallel processing may be slow.");
  }

  // Put name of object in output buffer
  UInt_t nameLength = name.size() + 1;
  m_buf->add(&nameLength, sizeof(nameLength));
  m_buf->add(name.c_str(), nameLength);
  // Copy object into buffer
  m_buf->add(&len, sizeof(len));
  m_buf->add(buf, len);
  m_msg->Reset();
}

EvtMessage* MsgHandler::encode_msg(RECORD_TYPE rectype)
{
  if (rectype == MSG_TERMINATE) {
    EvtMessage* eod = new EvtMessage(NULL, 0, rectype);
    return eod;
  }

  EvtMessage* evtmsg = new EvtMessage(m_buf->data(), m_buf->size(), rectype);
  clear();

  return evtmsg;
}

void MsgHandler::decode_msg(EvtMessage* msg, vector<TObject*>& objlist,
                            vector<string>& namelist)
{
  const char* msgptr = msg->msg();
  const char* end = msgptr + msg->msg_size();

  while (msgptr < end) {
    // Restore object name
    UInt_t nameLength;
    memcpy(&nameLength, msgptr, sizeof(nameLength));
    msgptr += sizeof(nameLength);
    if (msgptr >= end) B2FATAL("Buffer overrun while decoding message, check length fields!");

    namelist.emplace_back(msgptr);
    msgptr += nameLength;
    if (msgptr >= end) B2FATAL("Buffer overrun while decoding message, check length fields!");

    // Restore object
    UInt_t objlen;
    memcpy(&objlen, msgptr, sizeof(objlen));
    msgptr += sizeof(objlen);
    if (msgptr >= end) B2FATAL("Buffer overrun while decoding message, check length fields!");

    m_inMsg.SetBuffer(msgptr, objlen);
    TObject* obj = static_cast<TObject*>(m_inMsg.ReadObjectAny(m_inMsg.GetClass()));
    objlist.push_back(obj);
    msgptr += objlen;
    //no need to call InMessage::Reset() here (done in SetBuffer())
  }
}
