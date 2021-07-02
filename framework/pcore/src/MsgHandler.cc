/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/pcore/MsgHandler.h>
#include <framework/logging/Logger.h>

#include <TObject.h>
#include <TMessage.h>
#include <RZip.h>

using namespace std;
using namespace Belle2;


namespace {
  /** Warn if a streamed object is larger than this. */
  const static int c_maxObjectSizeBytes = 50000000; //50MB
}

MsgHandler::MsgHandler(int complevel):
  m_buf(100000),
  m_compBuf(0),
  m_msg(new TMessage(kMESS_OBJECT))
{
  m_complevel = complevel;

  //Schema evolution is needed to stream genfit tracks
  //If disabled, streamers will crash when reading data.
  TMessage::EnableSchemaEvolutionForAll();
  m_msg->SetWriteMode();
}

MsgHandler::~MsgHandler()  = default;

void MsgHandler::clear()
{
  m_buf.clear();
  m_compBuf.clear();
}

void MsgHandler::add(const TObject* obj, const string& name)
{
  m_msg->WriteObject(obj);

  int len = m_msg->Length();
  char* buf = m_msg->Buffer();

  if (len > c_maxObjectSizeBytes) {
    B2WARNING("MsgHandler: Object " << name << " is very large (" << len  << " bytes), parallel processing may be slow.");
  }

  // Put name of object in output buffer including a final 0-byte
  UInt_t nameLength = name.size() + 1;
  m_buf.add(&nameLength, sizeof(nameLength));
  m_buf.add(name.c_str(), nameLength);
  // Copy object into buffer
  m_buf.add(&len, sizeof(len));
  m_buf.add(buf, len);
  m_msg->Reset();
}

EvtMessage* MsgHandler::encode_msg(ERecordType rectype)
{
  if (rectype == MSG_TERMINATE) {
    auto* eod = new EvtMessage(nullptr, 0, rectype);
    return eod;
  }

  // which buffer to send? defaults to uncompressed
  auto buf = &m_buf;
  unsigned int flags = 0;
  // but if we have compression enabled then please compress.
  if (m_complevel > 0) {
    // make sure buffer for the compression is big enough.
    m_compBuf.resize(m_buf.size());
    // And call the root compression function
    const int algorithm = m_complevel / 100;
    const int level = m_complevel % 100;
    int irep{0}, nin{(int)m_buf.size()}, nout{nin};
    R__zipMultipleAlgorithm(level, &nin, m_buf.data(), &nout, m_compBuf.data(), &irep,
                            (ROOT::RCompressionSetting::EAlgorithm::EValues) algorithm);
    // it returns the number of bytes of the output in irep. If that is zero or
    // to big compression failed and we transmit uncompressed.
    if (irep > 0 && irep <= nin) {
      //set correct size of compressed message
      m_compBuf.resize(irep);
      // and set pointer to correct buffer for creating message
      buf = &m_compBuf;
      // also add a flag indicating it's compressed
      flags = EvtMessage::c_MsgCompressed;
    }
  }

  auto* evtmsg = new EvtMessage(buf->data(), buf->size(), rectype);
  evtmsg->setMsgFlags(flags);
  clear();

  return evtmsg;
}

void MsgHandler::decode_msg(EvtMessage* msg, vector<TObject*>& objlist,
                            vector<string>& namelist)
{
  const char* msgptr = msg->msg();
  const char* end = msgptr + msg->msg_size();

  if (msg->hasMsgFlags(EvtMessage::c_MsgCompressed)) {
    // apparently message is compressed, let's decompress
    m_compBuf.clear();
    int nzip{0}, nout{0};
    // ROOT wants unsigned char so make a new pointer to the data
    auto* zipptr = (unsigned char*) msgptr;
    // and uncompress everything
    while (zipptr < (unsigned char*)end) {
      // first get a header of the next block so we know how big the output will be
      if (R__unzip_header(&nzip, zipptr, &nout) != 0) {
        B2FATAL("Cannot uncompress message header");
      }
      // no more output? fine
      if (!nout) break;
      if (std::distance(zipptr, (unsigned char*) end) > nzip) {
        B2FATAL("Not enough bytes left to uncompress");
      }
      // otherwise make sure output buffer is large enough
      int old_size = m_compBuf.size();
      m_compBuf.resize(old_size + nout);
      // and uncompress, the amount of bytes will be returned as irep
      int irep{0};
      R__unzip(&nzip, zipptr, &nout, (unsigned char*)(m_compBuf.data() + old_size), &irep);
      // if that is not positive an error happend, bail
      if (irep <= 0) {
        B2FATAL("Cannot uncompress message");
      }
      // otherwise advance pointer by the amount of bytes compressed bytes in the block
      zipptr += nzip;
    }
    // ok, decompressed succesfully, set msg pointer to the correct area
    msgptr = m_compBuf.data();
    end = msgptr + m_compBuf.size();
  }

  while (msgptr < end) {
    // Restore object name
    UInt_t nameLength;
    memcpy(&nameLength, msgptr, sizeof(nameLength));
    msgptr += sizeof(nameLength);
    if (nameLength == 0 || std::distance(msgptr, end) < nameLength)
      B2FATAL("Buffer overrun while decoding object name, check length fields!");

    // read full string but omit final 0-byte. This safeguards against strings containing 0-bytes
    namelist.emplace_back(msgptr, nameLength - 1);
    msgptr += nameLength;

    // Restore object
    UInt_t objlen;
    memcpy(&objlen, msgptr, sizeof(objlen));
    msgptr += sizeof(objlen);
    if (objlen == 0 || std::distance(msgptr, end) < objlen)
      B2FATAL("Buffer overrun while decoding object, check length fields!");

    m_inMsg.SetBuffer(msgptr, objlen);
    objlist.push_back(m_inMsg.readTObject());
    msgptr += objlen;
    //no need to call InMessage::Reset() here (done in SetBuffer())
  }
}
