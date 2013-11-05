#include "nsm/NSMMessage.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>

extern "C" {
#include "nsm/nsmlib2.h"
#include "nsm/belle2nsm.h"
}

using namespace Belle2;

NSMMessage::NSMMessage() throw()
{
  _nsmc = NULL;
  _nsm_msg.req = 0;
  _nsm_msg.seq = 0;
  _nsm_msg.node = 0;
  _nsm_msg.npar = 0;
  _nsm_msg.len = 0;
  _text = "";
}

const char* NSMMessage::getRequestName() const throw()
{
  if (_nsmc != NULL)
    return nsmlib_reqname(_nsmc, _nsm_msg.req);
  else
    return NULL;
}

const char* NSMMessage::getNodeName() const throw()
{
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

unsigned int NSMMessage::getParam(int i) const throw()
{
  return _nsm_msg.pars[i];
}

const unsigned int* NSMMessage::getParams() const throw()
{
  return _nsm_msg.pars;
}

unsigned int* NSMMessage::getParams() throw()
{
  return _nsm_msg.pars;
}

unsigned int NSMMessage::getLength() const throw()
{
  return _nsm_msg.len;
}

const std::string& NSMMessage::getData() const throw()
{
  return _text;
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

void NSMMessage::setData(int len, const char* data)  throw()
{
  _nsm_msg.len = len;
  _text = data;
}

void NSMMessage::setData(const std::string& text)  throw()
{
  _nsm_msg.len = text.size();
  _text = text;
}

size_t NSMMessage::try_read(int sock, char* buf, int datalen)
{
  int recvlen = 0;
  while (recvlen < datalen) {
    int ret;
    if ((ret = ::read(sock, buf + recvlen, datalen)) <= 0) {
      if (ret == -1 && errno == EINTR) continue;
      if (ret < 0)  return -1;
    }
    datalen -= ret;
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
  size_t ret = 0;
  NSMtcphead hp;
  int datalen = sizeof(NSMtcphead);
  if ((ret = try_read(sock, (char*)&hp, datalen)) < 0) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to read header"));
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
      throw (NSMHandlerException(__FILE__, __LINE__, "Failed to read params"));
    }
    count += ret;
    for (int i = 0; i < _nsm_msg.npar; i++) {
      _nsm_msg.pars[i] = ntohl(_nsm_msg.pars[i]);
    }
  }

  datalen = _nsm_msg.len;
  if (datalen > 0) {
    char buf[512];
    memset(buf, 0, sizeof(buf));
    if ((ret = try_read(sock, buf, datalen)) < 0) {
      throw (NSMHandlerException(__FILE__, __LINE__, "Failed to read data"));
    }
    _text = buf;
    count += ret;
  }
  return count;
}
