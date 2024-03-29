/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/nsm/NSMMessage.h"

// 20191004
// duplicating the NSM2 library function in a user function
// NSMMessage::read(NSMcontext* nsmc)
// is really a bad idea, need to be revised.
// Until then, nsmsys2.h is needed.
#include <nsm2/nsmsys2.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMHandlerException.h>

#include <daq/slc/base/Reader.h>
#include <daq/slc/base/Writer.h>

#include <nsm2/nsmlib2.h>
#include <nsm2/nsmsys2.h>

#include <arpa/inet.h>

using namespace Belle2;

const unsigned int NSMMessage::DATA_SIZE = NSM_TCPDATSIZ;

void NSMMessage::init()
{
  m_nsmc = NULL;
  memset(&m_nsm_msg, 0, sizeof(NSMmsg));
  m_hasobj = false;
}

void NSMMessage::init(const NSMNode& node, const NSMVar& var)
{
  init();
  m_nodename = node.getName();
  m_reqname = NSMCommand::VSET.getLabel();
  m_nsm_msg.npar = 6;
  m_nsm_msg.pars[0] = (int)var.getType();
  m_nsm_msg.pars[1] = (int)var.getLength();
  m_nsm_msg.pars[2] = var.getNode().size();
  m_nsm_msg.pars[3] = var.getName().size();
  m_nsm_msg.pars[4] = var.getId();
  m_nsm_msg.pars[5] = var.getDate();
  if (var.getType() != NSMVar::NONE) {
    int size = var.getNode().size() + 1 + var.getName().size() + 1 + var.size();
    m_data = Buffer(size);
    char* pdata = (char*)m_data.ptr();
    memset(pdata, 0, size);
    memcpy(pdata, var.getNode().c_str(), var.getNode().size());
    pdata += var.getNode().size();
    *pdata = '\0';
    pdata++;
    memcpy(pdata, var.getName().c_str(), var.getName().size());
    pdata += var.getName().size();
    *pdata = '\0';
    pdata++;
    memcpy(pdata, var.get(), var.size());
    m_hasobj = false;
    m_nsm_msg.len = size;
  }
}

void NSMMessage::init(const NSMNode& node, const DAQLogMessage& log)
{
  init();
  m_nodename = node.getName();
  m_reqname = NSMCommand::LOG.getLabel();
  setNParams(3);
  setParam(0, (int)log.getPriority());
  setParam(1, log.getDateInt());
  setParam(2, log.getId());
  setData(log.getMessage());
}

void NSMMessage::init(const NSMNode& node, const DAQLogMessage& log,
                      const NSMCommand& cmd)
{
  init();
  m_nodename = node.getName();
  m_reqname = cmd.getLabel();
  setNParams(3);
  setParam(0, (int)log.getPriority());
  setParam(1, log.getDateInt());
  setParam(2, log.getId());
  setData(log.getMessage());
}

void NSMMessage::init(const NSMNode& node, const NSMData& data)
{
  init();
  m_nodename = node.getName();
  m_reqname = NSMCommand::DATASET.getLabel();
  setNParams(2);
  setParam(0, data.getRevision());
  setParam(1, data.getSize());
  std::string s = data.getName() + "\n" + data.getFormat();
  int len = s.size() + 1 + data.getSize();
  m_nsm_msg.len = len;
  m_data = Buffer(len);
  memcpy(m_data.ptr(), s.c_str(), s.size());
  memcpy(m_data.ptr() + s.size() + 1, data.get(), data.getSize());
  m_hasobj = false;
}

NSMMessage::NSMMessage()
{
  init();
}

NSMMessage::NSMMessage(const NSMNode& node)
{
  init();
  m_nodename = node.getName();
}

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd)
{
  init();
  m_nodename = node.getName();
  m_reqname = cmd.getLabel();
}

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd,
                       int npar, int* pars)
{
  init();
  m_nodename = node.getName();
  m_reqname = cmd.getLabel();
  m_nsm_msg.npar = npar;
  memcpy(m_nsm_msg.pars, pars, sizeof(int) * npar);
}

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd,
                       int npar, int* pars,
                       const std::string& data)
{
  init();
  m_nodename = node.getName();
  m_reqname = cmd.getLabel();
  m_nsm_msg.npar = npar;
  memcpy(m_nsm_msg.pars, pars, sizeof(int) * npar);
  setData(data);
}

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd,
                       int par, const std::string& obj)
{
  init();
  m_nodename = node.getName();
  m_reqname = cmd.getLabel();
  m_nsm_msg.npar = 1;
  m_nsm_msg.pars[0] = par;
  setData(obj);
}

NSMMessage::NSMMessage(const NSMCommand& cmd,
                       int par)
{
  init();
  m_reqname = cmd.getLabel();
  m_nsm_msg.npar = 1;
  m_nsm_msg.pars[0] = par;
}

NSMMessage::NSMMessage(const NSMCommand& cmd,
                       const std::string& data)
{
  init();
  m_reqname = cmd.getLabel();
  setData(data);
}

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd, int par)
{
  init();
  m_nodename = node.getName();
  m_reqname = cmd.getLabel();
  m_nsm_msg.npar = 1;
  m_nsm_msg.pars[0] = par;
}

NSMMessage::NSMMessage(const NSMNode& node,
                       const NSMCommand& cmd,
                       const std::string& data)
{
  init();
  m_nodename = node.getName();
  m_reqname = cmd.getLabel();
  setData(data);
}

NSMMessage::NSMMessage(const NSMCommand& cmd)
{
  init();
  m_reqname = cmd.getLabel();
}

NSMMessage::NSMMessage(const NSMMessage& msg)
{
  init();
  *this = msg;
}

NSMMessage::NSMMessage(const NSMNode& node, const NSMVar& var)
{
  init(node, var);
}

NSMMessage::NSMMessage(const NSMNode& node, const NSMData& data)
{
  init(node, data);
}

NSMMessage::NSMMessage(const NSMNode& node, const DAQLogMessage& log)
{
  init(node, log);
}

NSMMessage::NSMMessage(const NSMNode& node, const DAQLogMessage& log,
                       const NSMCommand& cmd)
{
  init(node, log, cmd);
}

NSMMessage::NSMMessage(const NSMVar& var)
{
  init(NSMNode(), var);
}

NSMMessage::NSMMessage(const NSMData& data)
{
  init(NSMNode(), data);
}

NSMMessage::NSMMessage(const DAQLogMessage& log)
{
  init(NSMNode(), log);
}

NSMMessage::NSMMessage(const NSMCommand& cmd,
                       int npar, int* pars)
{
  init();
  m_reqname = cmd.getLabel();
  m_nsm_msg.npar = npar;
  memcpy(m_nsm_msg.pars, pars, sizeof(int) * npar);
}

const NSMMessage& NSMMessage::operator=(const NSMMessage& msg)
{
  m_nsmc = msg.m_nsmc;
  memcpy(&m_nsm_msg, &(msg.m_nsm_msg), sizeof(m_nsm_msg));
  m_nsm_msg.npar = msg.m_nsm_msg.npar;
  m_nsm_msg.len = msg.m_nsm_msg.len;
  m_nodename = msg.m_nodename;
  m_reqname = msg.m_reqname;
  m_hasobj = msg.m_hasobj;
  //m_data = msg.m_data;
  m_data = Buffer(m_nsm_msg.len);
  memcpy(m_data.ptr(), msg.m_data.ptr(), m_nsm_msg.len);
  return *this;
}

const char* NSMMessage::getRequestName() const
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

void NSMMessage::setRequestName()
{
  if (m_nsmc != NULL) {
    const char* reqname = nsmlib_reqname(m_nsmc, m_nsm_msg.req);
    if (reqname != NULL) {
      m_reqname = reqname;
    }
  }
}

void NSMMessage::setRequestName(const std::string& reqname)
{
  m_reqname = reqname;
}

void NSMMessage::setRequestName(const NSMCommand& cmd)
{
  m_reqname = cmd.getLabel();
}

void NSMMessage::setNodeName(const std::string& nodename)
{
  m_nodename = nodename;
}

void NSMMessage::setNodeName(const NSMNode& node)
{
  m_nodename = node.getName();
}

const char* NSMMessage::getNodeName() const
{
  if (m_nodename.size() > 0) return m_nodename.c_str();
  if (m_nsmc != NULL)
    return nsmlib_nodename(m_nsmc, m_nsm_msg.node);
  else
    return m_nodename.c_str();
}

unsigned short NSMMessage::getRequestId() const
{
  return m_nsm_msg.req;
}

unsigned short NSMMessage::getSequenceId() const
{
  return m_nsm_msg.seq;
}

unsigned short NSMMessage::getNodeId() const
{
  return m_nsm_msg.node;
}

unsigned short NSMMessage::getNParams() const
{
  return m_nsm_msg.npar;
}

int NSMMessage::getParam(int i) const
{
  return m_nsm_msg.pars[i];
}

#if NSM_PACKAGE_VERSION >= 1914
const int* NSMMessage::getParams() const
{
  return m_nsm_msg.pars;
}

int* NSMMessage::getParams()
{
  return m_nsm_msg.pars;
}
#else
const unsigned int* NSMMessage::getParams() const
{
  return m_nsm_msg.pars;
}

unsigned int* NSMMessage::getParams()
{
  return m_nsm_msg.pars;
}
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif

unsigned int NSMMessage::getLength() const
{
  return m_nsm_msg.len;
}

const char* NSMMessage::getData() const
{
  if (m_nsm_msg.len > 0) return (const char*)m_data.ptr();
  else return NULL;
}

void NSMMessage::setRequestId(unsigned short id)
{
  m_nsm_msg.req = id;
}

void NSMMessage::setSequenceId(unsigned short id)
{
  m_nsm_msg.seq = id;
}

void NSMMessage::setNodeId(unsigned short id)
{
  m_nsm_msg.node = id;
}

void NSMMessage::setNParams(unsigned short npar)
{
  m_nsm_msg.npar = npar;
}

void NSMMessage::setParam(int i, unsigned int v)
{
  m_nsm_msg.pars[i] = v;
}

void NSMMessage::setData(int len, const char* data)
{
  m_nsm_msg.len = len;
  if (len > 0 && data != NULL) {
    m_data = Buffer(len);
    memcpy(m_data.ptr(), data, len);
  }
  m_hasobj = false;
}

void NSMMessage::setData(const std::string& text)
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

size_t NSMMessage::read(NSMcontext* nsmc)
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

void NSMMessage::readObject(Reader& reader)
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

void NSMMessage::writeObject(Writer& writer) const
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
