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


namespace {
  /** Warn if a streamed object is larger than this. */
  const static int c_maxObjectSizeBytes = 50000000; //50MB
}

using namespace std;
using namespace Belle2;

MsgHandler::MsgHandler(int complevel)
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
  // Initialize TMessage
  TMessage* msg = new TMessage(kMESS_OBJECT);
  msg->Reset();
  msg->SetWriteMode();
  msg->SetCompressionLevel(m_complevel);

  // Write object in TMessage
  msg->WriteObject(obj);
  msg->Compress(); //no effect if m_complevel == 0
  //  msg->ForceWriteInfo(obj->, true );

  // Obtain size of streamed object
  //  int len = msg->BufferSize();
  int len = msg->Length();
  if (msg->CompBuffer()) {
    // Compression ON
    len = msg->CompLength();
  }
  //  printf ( "MsgHandler : size of %s = %d (pid=%d)\n", name.c_str(), len, (int)getpid() );

  if (len > c_maxObjectSizeBytes) {
    B2WARNING("MsgHandler: Object " << name << " is very large (" << len  << " bytes), parallel processing may be slow.");
  }
  // Store streamed objects in array
  m_buf.push_back(msg);
  m_name.push_back(name);
  return true;
}

EvtMessage* MsgHandler::encode_msg(RECORD_TYPE rectype)
{
  if (rectype == MSG_TERMINATE) {
    EvtMessage* eod = new EvtMessage(NULL, 0, rectype);
    return eod;
  }
  //  printf ( "MsgHandler : encoding message ..... Nobjs = %d\n", m_buf.size() );

  // Initialize output buffer
  int totlen = 0;
  char* msgbuf = new char[EvtMessage::c_MaxEventSize];
  char* msgptr = msgbuf;
  int nameptr = 0;

  // Loop over streamed objects
  for (const TMessage* msg : m_buf) {
    char* buf = msg->Buffer();
    //    int len = msg->BufferSize();
    UInt_t len = msg->Length();
    if (msg->CompBuffer()) {
      // Compression ON
      len = msg->CompLength();
      buf = msg->CompBuffer();
    }
    // Put name of object in output buffer
    const string& name = m_name[nameptr];
    UInt_t nameLength = strlen(name.c_str()) + 1;
    memcpy(msgptr, &nameLength, sizeof(nameLength));
    memcpy(msgptr + sizeof(nameLength), name.c_str(), nameLength);
    msgptr += (sizeof(nameLength) + nameLength);
    totlen += (sizeof(nameLength) + nameLength);
    // Copy object into buffer
    memcpy(msgptr, &len, sizeof(len));
    memcpy(msgptr + sizeof(len), buf, len);
    msgptr += (sizeof(len) + len);
    totlen += (sizeof(len) + len);
    nameptr++;
    delete msg;
  }

  EvtMessage* evtmsg = new EvtMessage(msgbuf, totlen, rectype);

  //  printf ( "encode : msgbuf = %8.8x, %8.8x, %8.8x, %8.8x\n",
  //     *((int*)msgbuf), *((int*)(msgbuf+1)), *((int*)(msgbuf+2)), *((int*)(msgbuf+3)) );

  delete[] msgbuf;
  m_buf.clear();
  m_name.clear();

  return evtmsg;
}

int MsgHandler::decode_msg(EvtMessage* msg, vector<TObject*>& objlist,
                           vector<string>& namelist)
{
  int totlen = 0;
  char* msgptr = msg->msg();
  //  printf ( "decode : msgbuf = %8.8x, %8.8x, %8.8x, %8.8x\n",
  //     *((int*)msgptr), *((int*)(msgptr+1)), *((int*)(msgptr+2)), *((int*)(msgptr+3)) );

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

    TMessage* tmsg = new InMessage(msgptr + sizeof(objlen), objlen);
    TObject* obj = static_cast<TObject*>(tmsg->ReadObjectAny(tmsg->GetClass()));
    objlist.push_back(obj);

    msgptr += objlen + sizeof(objlen);
    totlen += objlen + sizeof(objlen);

    //TMessage doesn't honour the kIsOwner bit for the compression buffer and
    //tries to delete the passed message.
    //TODO: workaround: leak message; remove once fixed in ROOT
    if (!tmsg->CompBuffer())
      delete tmsg;

    //    printf ( "decode : %s added to objlist; size=%d (pid=%d)\n",
    //           name.c_str(), objlen, (int)getpid()  );
    //    fflush(stdout);

  }
  return 0;
}
