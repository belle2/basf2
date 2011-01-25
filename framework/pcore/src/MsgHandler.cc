//+
// File : MsgHandler.cc
// Description : Encode/Decode EvtMessage
//
// Author : SooHyung Lee, Ryosuke Itoh
// Date : 31 - Jul - 2008
// Modified : 4 - Jun - 2010
//-

#include <framework/pcore/MsgHandler.h>

#include <stdlib.h>
//#include <iostream>
//#include <cassert>

// #include "pcore/EvtMessage.h"


#define MAX_BUFFER_SIZE 4000000

using namespace std;
using namespace Belle2;

MsgHandler::MsgHandler(int complevel)
{
  printf("MsgHandler : constructor called.....\n");
  m_cbuf = (char*) malloc(MAX_BUFFER_SIZE);
  m_complevel = complevel;
}

MsgHandler::~MsgHandler(void)
{
  printf("MsgHandler : destructor called.....\n");
  //  free ( m_cbuf );
}

void MsgHandler::clear(void)
{
  m_buf.clear();
}

void MsgHandler::add(TObject* obj, string name)
{
  TMessage* msg = new TMessage(kMESS_OBJECT);
  msg->SetWriteMode();
  msg->Reset();
  msg->WriteObject(obj);
  msg->SetCompressionLevel(m_complevel);
  msg->Compress();
  m_buf.push_back(msg);
  m_name.push_back(name);
}

EvtMessage* MsgHandler::encode_msg(RECORD_TYPE rectype)
{
  if (rectype == MSG_TERMINATE) {
    EvtMessage* eod = new EvtMessage(NULL, 0, rectype);
    return eod;
  }

  int totlen = 0;
  char* msgbuf = new char[MAXEVTMSG];
  char* msgptr = msgbuf;
  int nameptr = 0;
  for (vector<TMessage*>::iterator it = m_buf.begin(); it != m_buf.end(); ++it) {
    // Put name of object
    string& name = m_name[nameptr];
    int lname = strlen(name.c_str()) + 1;
    memcpy(msgptr, &lname, sizeof(int));
    memcpy(msgptr + sizeof(int), name.c_str(), lname);
    msgptr += (sizeof(int) + lname);
    totlen += (sizeof(int) + lname);
    // Put object
    TMessage* msg = *it;
    char* buf = msg->Buffer();
    int len = msg->BufferSize();
    if (msg->CompBuffer()) {
      // Compression ON
      len = msg->CompLength();
      buf = msg->CompBuffer();
    }
    //    printf ( "new obj size = %d, msgptr = %8.8x\n", len, msgptr );
    memcpy(msgptr, &len, sizeof(int));
    memcpy(msgptr + sizeof(int), buf, len);
    msgptr += (sizeof(int) + len);
    totlen += (sizeof(int) + len);
  }
  EvtMessage* evtmsg = new EvtMessage(msgbuf, totlen, rectype);

  //  printf ( "encode : msgbuf = %8.8x, %8.8x, %8.8x, %8.8x\n",
  //     *((int*)msgbuf), *((int*)(msgbuf+1)), *((int*)(msgbuf+2)), *((int*)(msgbuf+3)) );

  delete[] msgbuf;

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
    int lname;
    memcpy(&lname, msgptr, sizeof(int));
    string name((char*)(msgptr + sizeof(int)));
    namelist.push_back(name);
    msgptr += (sizeof(int) + lname);
    totlen += (sizeof(int) + lname);
    // Restore object
    int objlen;
    memcpy(&objlen, msgptr, sizeof(int));
    //    printf ( "decode_msg : objlen = %d\n", objlen );
    // Old impl.
    TMessage * tmsg = new InMessage(msgptr + sizeof(int), objlen);
    //    char* tmpmsg = new char[objlen];
    //    TMessage* tmsg = new InMessage ( tmpmsg, objlen );
    TObject* obj = (TObject*)tmsg->ReadObjectAny(NULL);
    objlist.push_back(obj);
    msgptr += objlen + sizeof(int);
    totlen += objlen + sizeof(int);
    //    delete tmsg; // tmpmsg should be deleted here also.
  }
  return 0;
}
