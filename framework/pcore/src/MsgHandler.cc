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


/** Maximal size of streamed objects. */
const static int c_maxObjectSizeBytes = 50000000; //50MB

using namespace std;
using namespace Belle2;

MsgHandler::MsgHandler(int complevel)
{
  //  printf("MsgHandler : constructor called.....\n");
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
  //  printf("MsgHandler : destructor called.....\n");
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

  // Store streamed objects in array
  if (len < c_maxObjectSizeBytes) {
    m_buf.push_back(msg);
    m_name.push_back(name);
    return true;
  } else {
    B2FATAL("MsgHandler : " << name <<
            " : size too large (" << len  << " bytes), dropped.");
    return false;
  }
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
  for (vector<TMessage*>::iterator it = m_buf.begin(); it != m_buf.end(); ++it) {
    // Get object
    TMessage* msg = *it;
    // Get buffer
    char* buf = msg->Buffer();
    // Get buffer length
    //    int len = msg->BufferSize();
    UInt_t len = msg->Length();
    if (msg->CompBuffer()) {
      // Compression ON
      len = msg->CompLength();
      buf = msg->CompBuffer();
    }
    // Put name of object in output buffer
    string& name = m_name[nameptr];
    UInt_t nameLength = strlen(name.c_str()) + 1;
    memcpy(msgptr, &nameLength, sizeof(nameLength));
    memcpy(msgptr + sizeof(nameLength), name.c_str(), nameLength);
    msgptr += (sizeof(nameLength) + nameLength);
    totlen += (sizeof(nameLength) + nameLength);
    // Copy object into buffer
    //    printf ( "encode: obj name = %s : size = %d, msgptr = %8.8x\n",
    //           name.c_str(), len, msgptr );
    //    fflush ( stdout );
    memcpy(msgptr, &len, sizeof(len));
    memcpy(msgptr + sizeof(len), buf, len);
    msgptr += (sizeof(len) + len);
    totlen += (sizeof(len) + len);
    nameptr++;
    delete msg; // test
  }

  // Create EvtMessage
  EvtMessage* evtmsg = new EvtMessage(msgbuf, totlen, rectype);

  //  printf ( "encode : msgbuf = %8.8x, %8.8x, %8.8x, %8.8x\n",
  //     *((int*)msgbuf), *((int*)(msgbuf+1)), *((int*)(msgbuf+2)), *((int*)(msgbuf+3)) );

  delete[] msgbuf;
  m_buf.erase(m_buf.begin(), m_buf.end());
  m_name.erase(m_name.begin(), m_name.end());

  return evtmsg;

  //  return new EvtMessage ( NULL, 0, MSG_TERMINATE );
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
    //    printf ( "MsgHandler::decode obj=%s\n", name.c_str() );
    memcpy(&objlen, msgptr, sizeof(objlen));
    //    printf ( "decode : objlen = %d\n", objlen );
    // Old impl.
    TMessage* tmsg = new InMessage(msgptr + sizeof(objlen), objlen);
    //    char* tmpmsg = new char[objlen];
    //    TMessage* tmsg = new InMessage ( tmpmsg, objlen );
    //    TObject* obj = (TObject*)tmsg->ReadObjectAny(NULL);
    TObject* obj = (TObject*)tmsg->ReadObjectAny(tmsg->GetClass());
    objlist.push_back(obj);

    /*
    if ( obj != NULL ) {
      printf ( "MsgHandler::decode_msg : obj=%s, class=%s\n",
         name.c_str(), obj->ClassName() );
    }
    else {
      printf ( "MsgHandler::decode_msg : obj=%s, Null Object\n",
         name.c_str() );
    }
    */

    msgptr += objlen + sizeof(objlen);
    totlen += objlen + sizeof(objlen);

    //TMessage doesn't honour the kIsOwner bit for the compression buffer and
    //tries to delete the passed message.
    //TODO: workaround: leak message; remove once fixed in ROOT
    if (!tmsg->CompBuffer())
      delete tmsg; // tmpmsg should be deleted here also.

    //    printf ( "decode : %s added to objlist; size=%d (pid=%d)\n",
    //           name.c_str(), objlen, (int)getpid()  );
    fflush(stdout);

  }
  //  printf ( "decode : done\n" );
  return 0;
}
