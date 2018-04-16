#include "dqm/analysis/HistMemory.h"

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>

using namespace Belle2;

void HistMemory::open(const char* path, unsigned int size, const char* mode)
{
  m_path = path;
  m_size = size;
  int flag = O_RDWR | O_CREAT | O_EXCL;
  bool recreate = true;
  if (mode != NULL && StringUtil::tolower(mode) == "recreate") {
    flag = O_RDWR | O_CREAT;
  }
  int fd = ::open(m_path.c_str(), flag, 0666);
  if (fd < 0) {
    if (errno == EEXIST) {
      recreate = false;
      fd = ::open(m_path.c_str(), O_RDWR, 0666);
    } else {
      throw (IOException("Failed to open file %s : %s", path, strerror(errno)));
    }
  }
  m_buf = new char[size];
  ::memset(m_buf, 0, size);
  if (recreate) {
    int ret = ::write(fd, m_buf, size);
    if (ret < 0) {
      delete [] m_buf;
      throw (IOException("Failed to clear file %s : %s", path, strerror(errno)));
    }
  }
  m_fd = fd;
  char* buf = (char*)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
  m_mutex = MMutex(buf);
  buf += m_mutex.size();
  m_header = (Header*)buf;
  buf += sizeof(Header);
  m_body = (char*)buf;
  if (recreate) init();
}

void HistMemory::init()
{
  if (m_body == NULL) {
    throw (IOException("%s is not opened", m_path.c_str()));
  }
  memset(m_buf, 0, m_size);
  m_mutex.init();
}

void HistMemory::serialize()
{
  for (size_t i = 0; i < m_hist.size(); i++) {
    m_handler.add(m_hist[i], m_hist[i]->GetName());
  }
  EvtMessage* msg = m_handler.encode_msg(MSG_EVENT);
  (msg->header())->reserved[0] = 0;
  (msg->header())->reserved[1] = (int)m_hist.size();
  (msg->header())->reserved[2] = 0;
  m_mutex.lock();
  memcpy(m_body, (char*)msg->buffer(), msg->size());
  m_header->nbytes = msg->size();
  m_header->updateid++;
  m_mutex.unlock();
  delete msg;
}

std::vector<TH1*>& HistMemory::deserialize(Header* header)
{
  m_hist = std::vector<TH1*>();
  m_mutex.lock();
  if (m_header->updateid <= m_updateid) {
    m_mutex.unlock();
    return m_hist;
  }
  if (header != NULL) {
    memcpy(header, m_header, sizeof(Header));
  }
  m_updateid = m_header->updateid;
  memcpy(m_buf, m_body, m_header->nbytes);
  m_mutex.unlock();

  for (size_t i = 0; i < m_hist.size(); i++) {
    delete m_hist[i];
  }

  EvtMessage* msg = new EvtMessage(m_buf);
  std::vector<TObject*> objlist;
  std::vector<std::string> strlist;
  m_handler.decode_msg(msg, objlist, strlist);
  int nobjs = (msg->header())->reserved[1];
  for (int i = 0; i < nobjs; i++) {
    add((TH1*)objlist[i]->Clone());
  }
  delete msg;
  return m_hist;
}

