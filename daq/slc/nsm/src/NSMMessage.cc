#include "daq/slc/nsm/NSMMessage.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

extern "C" {
#include "nsm2/nsmlib2.h"
#include "nsm2/belle2nsm.h"
}

#include <daq/slc/system/BufferedReader.h>
#include <daq/slc/system/BufferedWriter.h>

#include <daq/slc/base/Reader.h>
#include <daq/slc/base/Writer.h>

using namespace Belle2;

const unsigned int NSMMessage::DATA_SIZE = NSM_TCPDATSIZ;

void NSMMessage::init() throw()
{
  _nsmc = NULL;
  memset(&_nsm_msg, 0, sizeof(NSMmsg));
  memset(_data, 0, sizeof(_data));
}

NSMMessage::NSMMessage() throw()
{
  init();
}

NSMMessage::NSMMessage(const NSMNode& node) throw()
{
  init();
  _nodename = node.getName();
}

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd) throw()
{
  init();
  _nodename = node.getName();
  _reqname = cmd.getLabel();
}

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd,
                       int npar, int* pars) throw()
{
  init();
  _nodename = node.getName();
  _reqname = cmd.getLabel();
  _nsm_msg.npar = npar;
  memcpy(_nsm_msg.pars, pars, sizeof(int) * npar);
}

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd,
                       int par, const Serializable& obj) throw()
{
  init();
  _nodename = node.getName();
  _reqname = cmd.getLabel();
  _nsm_msg.npar = 1;
  _nsm_msg.pars[0] = par;
  setData(obj);
}

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd,
                       int par, const std::string& obj) throw()
{
  init();
  _nodename = node.getName();
  _reqname = cmd.getLabel();
  _nsm_msg.npar = 1;
  _nsm_msg.pars[0] = par;
  setData(obj);
}

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd,
                       const Serializable& obj) throw()
{
  init();
  _nodename = node.getName();
  _reqname = cmd.getLabel();
  setData(obj);
}

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd,
                       const std::string& data) throw()
{
  init();
  _nodename = node.getName();
  _reqname = cmd.getLabel();
  setData(data);
}

NSMMessage::NSMMessage(const NSMCommand& cmd) throw()
{
  init();
  _reqname = cmd.getLabel();
}

NSMMessage::NSMMessage(const NSMMessage& msg) throw()
{
  *this = msg;
}

const NSMMessage& NSMMessage::operator=(const NSMMessage& msg) throw()
{
  _nsmc = msg._nsmc;
  memcpy(&_nsm_msg, &(msg._nsm_msg), sizeof(_nsm_msg));
  memcpy(_data, msg._data, sizeof(_data));
  _nodename = msg._nodename;
  _reqname = msg._reqname;
  return *this;
}

const char* NSMMessage::getRequestName() const throw()
{
  if (_reqname.size() > 0) return _reqname.c_str();
  if (_nsmc != NULL) {
    const char* reqname = nsmlib_reqname(_nsmc, _nsm_msg.req);
    if (reqname != NULL) {
      _reqname = reqname;
    }
    return _reqname.c_str();
  } else {
    return NULL;
  }
}

void NSMMessage::setRequestName() throw()
{
  if (_nsmc != NULL) {
    const char* reqname = nsmlib_reqname(_nsmc, _nsm_msg.req);
    if (reqname != NULL) {
      _reqname = reqname;
    }
  }
}

void NSMMessage::setRequestName(const std::string& reqname) throw()
{
  _reqname = reqname;
}

void NSMMessage::setRequestName(const NSMCommand& cmd) throw()
{
  _reqname = cmd.getLabel();
}

void NSMMessage::setNodeName(const std::string& nodename) throw()
{
  _nodename = nodename;
}

void NSMMessage::setNodeName(const NSMNode& node) throw()
{
  _nodename = node.getName();
}

const char* NSMMessage::getNodeName() const throw()
{
  if (_nodename.size() > 0) return _nodename.c_str();
  if (_nsmc != NULL)
    return nsmlib_nodename(_nsmc, _nsm_msg.node);
  else
    return NULL;
}

unsigned short NSMMessage::getRequestId() const throw()
{
  return _nsm_msg.req;
}

unsigned short NSMMessage::getSequenceId() const throw()
{
  return _nsm_msg.seq;
}

unsigned short NSMMessage::getNodeId() const throw()
{
  return _nsm_msg.node;
}

unsigned short NSMMessage::getNParams() const throw()
{
  return _nsm_msg.npar;
}

int NSMMessage::getParam(int i) const throw()
{
  return _nsm_msg.pars[i];
}

#if NSM_PACKAGE_VERSION >= 1914
const int* NSMMessage::getParams() const throw()
{
  return _nsm_msg.pars;
}

int* NSMMessage::getParams() throw()
{
  return _nsm_msg.pars;
}
#else
const unsigned int* NSMMessage::getParams() const throw()
{
  return _nsm_msg.pars;
}

unsigned int* NSMMessage::getParams() throw()
{
  return _nsm_msg.pars;
}
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif

unsigned int NSMMessage::getLength() const throw()
{
  return _nsm_msg.len;
}

void NSMMessage::setLength(unsigned int len) throw()
{
  _nsm_msg.len = len;
}

const char* NSMMessage::getData() const throw()
{
  if (_nsm_msg.len > 0) return (const char*)_data;
  else return NULL;
}

void NSMMessage::setRequestId(unsigned short id) throw()
{
  _nsm_msg.req = id;
}

void NSMMessage::setSequenceId(unsigned short id) throw()
{
  _nsm_msg.seq = id;
}

void NSMMessage::setNodeId(unsigned short id) throw()
{
  _nsm_msg.node = id;
}

void NSMMessage::setNParams(unsigned short npar) throw()
{
  _nsm_msg.npar = npar;
}

void NSMMessage::setParam(int i, unsigned int v) throw()
{
  _nsm_msg.pars[i] = v;
}

void NSMMessage::getData(Serializable& obj) const throw(IOException)
{
  BufferedReader reader(sizeof(_data), (unsigned char*)_data);
  reader.readObject(obj);
}

void NSMMessage::setData(const Serializable& obj) throw(IOException)
{
  BufferedWriter writer(sizeof(_data), (unsigned char*)_data);
  writer.writeObject(obj);
  _nsm_msg.len = writer.count();
}

void NSMMessage::setData(int len, const char* data)  throw()
{
  memset(_data, 0, sizeof(_data));
  if (len > 0 && data != NULL) {
    _nsm_msg.len = len;
    memcpy(_data, data, len);
  }
}

void NSMMessage::setData(const std::string& text)  throw()
{
  setData(text.size(), text.c_str());
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
  _nsmc = nsmc;
  int sock = nsmc->sock;
  size_t count = 0;
  int ret = 0;
  NSMtcphead hp;
  int datalen = sizeof(NSMtcphead);
  if ((ret = try_read(sock, (char*)&hp, datalen)) < 0) {
    throw (NSMHandlerException("Failed to read header"));
  }
  count += ret;
  _nsm_msg.req  = ntohs(hp.req);
  _nsm_msg.seq  = ntohs(hp.seq);
  _nsm_msg.node = ntohs(hp.src);
  _nsm_msg.npar = hp.npar;
  _nsm_msg.len  = ntohs(hp.len);
  _nsm_msg.pars[0] = _nsm_msg.pars[1] = 0;

  datalen = sizeof(int32) * _nsm_msg.npar;
  if (datalen > 0) {
    if ((ret = try_read(sock, (char*)(_nsm_msg.pars), datalen)) < 0) {
      throw (NSMHandlerException("Failed to read params"));
    }
    count += ret;
    for (int i = 0; i < _nsm_msg.npar; i++) {
      _nsm_msg.pars[i] = ntohl(_nsm_msg.pars[i]);
    }
  }

  datalen = _nsm_msg.len;
  if (datalen > 0) {
    memset(_data, 0, sizeof(_data));
    if ((ret = try_read(sock, (char*)_data, datalen)) < 0) {
      throw (NSMHandlerException("Failed to read data"));
    }
    count += ret;
  }
  return count;
}

void NSMMessage::readObject(Reader& reader) throw(IOException)
{
  _reqname = reader.readString();
  setNParams(reader.readInt());
  for (int i = 0; i < getNParams(); i++) {
    setParam(i, reader.readInt());
  }
  setLength(reader.readInt());
  reader.read(_data, getLength());
}

void NSMMessage::writeObject(Writer& writer) const throw(IOException)
{
  writer.writeString(getRequestName());
  writer.writeInt(getNParams());
  for (int i = 0; i < getNParams(); i++) {
    writer.writeInt(getParam(i));
  }
  writer.writeInt(getLength());
  writer.write(_data, getLength());
}
