#include "daq/storage/BinData.h"

#include "daq/slc/base/StringUtil.h"

#include <cstring>

using namespace Belle2;

const unsigned int BinData::TRAILER_MAGIC = 0x7FFF0007;

BinData::BinData(void* buf)
{
  if (buf != NULL) {
    setBuffer(buf);
    _header->nword_in_header = sizeof(BinHeader) / 4;
    _header->nword = (sizeof(BinHeader) + sizeof(BinTrailer)) / 4;
    _header->nevent_nboard = 0;
    _header->exp_run = 0;
    _header->event_number = 0;
    _header->node_id = 0;
  }
}

BinData::~BinData() throw()
{

}

void BinData::setBuffer(void* buf)
{
  if (buf != NULL) {
    _buf = (int*)buf;
    _header = (BinHeader*)buf;
    _body = (unsigned int*)(((char*)buf) + sizeof(BinHeader));
  } else {
    _buf = NULL;
  }
}

unsigned int BinData::recvEvent(TCPSocket& socket)
throw(IOException)
{
  unsigned int count = socket.read(_header, sizeof(BinHeader));
  if (_header->nword_in_header * 4 != sizeof(BinHeader)) {
    throw (IOException("unexpected header size %d words",
                       _header->nword_in_header));
  }
  const int nbytes_remains = getByteSize() - sizeof(BinHeader);
  count += socket.read(_body, nbytes_remains);
  _trailer = (BinTrailer*)(((char*)_body) + nbytes_remains - sizeof(BinTrailer));
  if (_trailer->magic != TRAILER_MAGIC) {
    throw (IOException("unexpected header trailer magic %08x",
                       _trailer->magic));
  }
  return count;
}

unsigned int BinData::sendEvent(TCPSocket& socket) const throw(IOException)
{
  return socket.write(_buf, getByteSize());
}
