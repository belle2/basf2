#include "daq/slc/nsm/NSMMessage.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>

extern "C" {
#include "nsm2/nsmlib2.h"
#include "nsm2/belle2nsm.h"
}

#include <daq/slc/system/BufferedReader.h>
#include <daq/slc/system/BufferedWriter.h>
#include <daq/slc/system/StreamSizeCounter.h>

#include <daq/slc/base/Reader.h>
#include <daq/slc/base/Writer.h>

using namespace Belle2;

const unsigned int NSMMessage::DATA_SIZE = NSM_TCPDATSIZ;

void NSMMessage::init() throw()
{
  m_nsmc = NULL;
  memset(&m_nsm_msg, 0, sizeof(NSMmsg));
  m_hasobj = false;
}

NSMMessage::NSMMessage() throw()
{
  init();
}

NSMMessage::NSMMessage(const NSMNode& node) throw()
{
  init();
  m_nodename = node.getName();
}

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd) throw()
{
  init();
  m_nodename = node.getName();
  m_reqname = cmd.getLabel();
}

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd,
                       int npar, int* pars) throw()
{
  init();
  m_nodename = node.getName();
  m_reqname = cmd.getLabel();
  m_nsm_msg.npar = npar;
  memcpy(m_nsm_msg.pars, pars, sizeof(int) * npar);
}

/*
NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd,
                       int par, const Serializable& obj) throw()
{
  init();
  m_nodename = node.getName();
  m_reqname = cmd.getLabel();
  m_nsm_msg.npar = 1;
  m_nsm_msg.pars[0] = par;
  setData(obj);
}
*/

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd,
                       int par, const std::string& obj) throw()
{
  init();
  m_nodename = node.getName();
  m_reqname = cmd.getLabel();
  m_nsm_msg.npar = 1;
  m_nsm_msg.pars[0] = par;
  setData(obj);
}

/*
NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd,
                       const Serializable& obj) throw()
{
  init();
  m_nodename = node.getName();
  m_reqname = cmd.getLabel();
  setData(obj);
}
*/

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd,
                       const std::string& data) throw()
{
  init();
  m_nodename = node.getName();
  m_reqname = cmd.getLabel();
  setData(data);
}

NSMMessage::NSMMessage(const NSMCommand& cmd) throw()
{
  init();
  m_reqname = cmd.getLabel();
}

NSMMessage::NSMMessage(const NSMMessage& msg) throw()
{
  init();
  *this = msg;
}

const NSMMessage& NSMMessage::operator=(const NSMMessage& msg) throw()
{
  m_nsmc = msg.m_nsmc;
  memcpy(&m_nsm_msg, &(msg.m_nsm_msg), sizeof(m_nsm_msg));
  m_data = msg.m_data;
  m_nsm_msg.len = msg.m_nsm_msg.len;
  m_nodename = msg.m_nodename;
  m_reqname = msg.m_reqname;
  m_hasobj = msg.m_hasobj;
  return *this;
}

const char* NSMMessage::getRequestName() const throw()
{
  if (m_reqname.size() > 0) return m_reqname.c_str();
  if (m_nsmc != NULL) {
    const char* reqname = nsmlib_reqname(m_nsmc, m_nsm_msg.req);
    if (reqname != NULL) {
      m_reqname = reqname;
    }
    return m_reqname.c_str();
  } else {
    m_reqname = "";
    return m_reqname.c_str();
  }
}

void NSMMessage::setRequestName() throw()
{
  if (m_nsmc != NULL) {
    const char* reqname = nsmlib_reqname(m_nsmc, m_nsm_msg.req);
    if (reqname != NULL) {
      m_reqname = reqname;
    }
  }
}

void NSMMessage::setRequestName(const std::string& reqname) throw()
{
  m_reqname = reqname;
}

void NSMMessage::setRequestName(const NSMCommand& cmd) throw()
{
  m_reqname = cmd.getLabel();
}

void NSMMessage::setNodeName(const std::string& nodename) throw()
{
  m_nodename = nodename;
}

void NSMMessage::setNodeName(const NSMNode& node) throw()
{
  m_nodename = node.getName();
}

const char* NSMMessage::getNodeName() const throw()
{
  if (m_nodename.size() > 0) return m_nodename.c_str();
  if (m_nsmc != NULL)
    return nsmlib_nodename(m_nsmc, m_nsm_msg.node);
  else
    return NULL;
}

unsigned short NSMMessage::getRequestId() const throw()
{
  return m_nsm_msg.req;
}

unsigned short NSMMessage::getSequenceId() const throw()
{
  return m_nsm_msg.seq;
}

unsigned short NSMMessage::getNodeId() const throw()
{
  return m_nsm_msg.node;
}

unsigned short NSMMessage::getNParams() const throw()
{
  return m_nsm_msg.npar;
}

int NSMMessage::getParam(int i) const throw()
{
  return m_nsm_msg.pars[i];
}

#if NSM_PACKAGE_VERSION >= 1914
const int* NSMMessage::getParams() const throw()
{
  return m_nsm_msg.pars;
}

int* NSMMessage::getParams() throw()
{
  return m_nsm_msg.pars;
}
#else
const unsigned int* NSMMessage::getParams() const throw()
{
  return m_nsm_msg.pars;
}

unsigned int* NSMMessage::getParams() throw()
{
  return m_nsm_msg.pars;
}
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif

unsigned int NSMMessage::getLength() const throw()
{
  return m_nsm_msg.len;
}

const char* NSMMessage::getData() const throw()
{
  if (m_nsm_msg.len > 0) return (const char*)m_data.ptr();
  else return NULL;
}

void NSMMessage::setRequestId(unsigned short id) throw()
{
  m_nsm_msg.req = id;
}

void NSMMessage::setSequenceId(unsigned short id) throw()
{
  m_nsm_msg.seq = id;
}

void NSMMessage::setNodeId(unsigned short id) throw()
{
  m_nsm_msg.node = id;
}

void NSMMessage::setNParams(unsigned short npar) throw()
{
  m_nsm_msg.npar = npar;
}

void NSMMessage::setParam(int i, unsigned int v) throw()
{
  m_nsm_msg.pars[i] = v;
}

void NSMMessage::setData(int len, const char* data)  throw()
{
  m_nsm_msg.len = len;
  if (len > 0 && data != NULL) {
    m_data = Buffer(len);
    memcpy(m_data.ptr(), data, len);
  }
  m_hasobj = false;
}

void NSMMessage::setData(const std::string& text)  throw()
{
  setData(text.size() + 1, text.c_str());
}

int NSMMessage::try_read(int sock, char* buf, int datalen)
{
  int recvlen = 0;
  while (recvlen < datalen) {
    int ret;
    if ((ret = ::read(sock, buf + recvlen, datalen)) <= 0) {
      if (ret == -1 && errno == EINTR) continue;
      if (ret < 0)  return -1;
    }
    recvlen += ret;
  }
  return recvlen;
}

size_t NSMMessage::read(NSMcontext* nsmc) throw(NSMHandlerException)
{
  if (nsmc == NULL) return 0;
  m_nsmc = nsmc;
  int sock = nsmc->sock;
  size_t count = 0;
  int ret = 0;
  NSMtcphead hp;
  int datalen = sizeof(NSMtcphead);
  if ((ret = try_read(sock, (char*)&hp, datalen)) < 0) {
    throw (NSMHandlerException("Failed to read header"));
  }
  count += ret;
  m_nsm_msg.req  = ntohs(hp.req);
  m_nsm_msg.seq  = ntohs(hp.seq);
  m_nsm_msg.node = ntohs(hp.src);
  m_nsm_msg.npar = hp.npar;
  m_nsm_msg.len  = ntohs(hp.len);
  m_nsm_msg.pars[0] = m_nsm_msg.pars[1] = 0;

  datalen = sizeof(int32) * m_nsm_msg.npar;
  if (datalen > 0) {
    if ((ret = try_read(sock, (char*)(m_nsm_msg.pars), datalen)) < 0) {
      throw (NSMHandlerException("Failed to read params"));
    }
    count += ret;
    for (int i = 0; i < m_nsm_msg.npar; i++) {
      m_nsm_msg.pars[i] = ntohl(m_nsm_msg.pars[i]);
    }
  }

  datalen = m_nsm_msg.len;
  if (datalen > 0) {
    m_data = Buffer(datalen);
    if ((ret = try_read(sock, (char*)m_data.ptr(), datalen)) < 0) {
      throw (NSMHandlerException("Failed to read data"));
    }
    count += ret;
  }
  return count;
}

void NSMMessage::readObject(Reader& reader) throw(IOException)
{
  setRequestName(reader.readString());
  setNodeName(reader.readString());
  setNParams(reader.readInt());
  for (int i = 0; i < getNParams(); i++) {
    setParam(i, reader.readInt());
  }
  size_t len = reader.readInt();
  m_nsm_msg.len = len;
  if (len > 0) {
    m_data = Buffer(len);
    reader.read(m_data.ptr(), len);
  }
}

void NSMMessage::writeObject(Writer& writer) const throw(IOException)
{
  writer.writeString(getRequestName());
  writer.writeString(getNodeName());
  writer.writeInt(getNParams());
  for (int i = 0; i < getNParams(); i++) {
    writer.writeInt(getParam(i));
  }
  if (m_hasobj) {
    writer.writeInt(-1);
  } else {
    writer.writeInt(getLength());
  }
  if (getLength() > 0) {
    writer.write(m_data.ptr(), getLength());
  }
}
