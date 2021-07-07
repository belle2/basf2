/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/nsm/NSMDataStore.h"

#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>

#include <cstdio>
#include <cstring>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <daq/slc/system/LockGuard.h>

using namespace Belle2;

NSMDataStore NSMDataStore::g_store;

bool NSMDataStore::open(unsigned short max)
{
  size_t size = (max > 0) ? m_mutex.size() + m_cond.size() +
                sizeof(Header) + sizeof(Entry) * max : 0;
  if (!m_mem.open("NSMDataStore", size)) {
    return false;
  }
  m_mem.truncate(0);
  char* buf = (char*)m_mem.map();
  m_buf = buf;
  m_mutex = MMutex(buf);
  buf += m_mutex.size();
  m_cond = MCond(buf);
  buf += m_cond.size();
  m_header = (Header*)buf;
  buf += sizeof(Header);
  m_entries = (Entry*)buf;
  if (max > 0) {
    m_header->maxentries = max;
  }
  return true;
}

void NSMDataStore::init()
{
  if (m_header == NULL) return;
  int max = m_header->maxentries;
  memset(m_buf, 0, m_mem.size());
  m_header->maxentries = max;
  m_mutex.init();
  m_cond.init();
}

NSMDataStore::Entry* NSMDataStore::add(unsigned int addr,
                                       unsigned int size,
                                       unsigned int revision,
                                       const std::string& name,
                                       const std::string& format,
                                       unsigned int rid)
{
  MLockGuard lockGuard(m_mutex);
  unsigned int n = m_header->nentries;
  for (unsigned int i = 0; i < n; i++) {
    if (m_entries[i].id > 0 && name == m_entries[i].name) {
      return &m_entries[i];
    }
  }
  memset(&m_entries[n], 0, sizeof(Entry));
  m_entries[n].id = n + 1;
  m_entries[n].rid = rid;
  m_entries[n].addr = addr;
  m_entries[n].size = size;
  m_entries[n].revision = revision;
  m_entries[n].utime = (unsigned int)Time().getSecond();
  strcpy(m_entries[n].name, name.c_str());
  strcpy(m_entries[n].format, format.c_str());
  m_header->nentries++;
  return &m_entries[n];
}

NSMDataStore::Entry* NSMDataStore::get(const std::string& name)
{
  MLockGuard lockGuard(m_mutex);
  const unsigned int n = m_header->nentries;
  for (unsigned int i = 0; i < n; i++) {
    if (m_entries[i].id > 0 && name == m_entries[i].name) {
      return &m_entries[i];
    }
  }
  return NULL;
}

NSMDataStore::Entry* NSMDataStore::get(unsigned int id)
{
  if (id == 0) return NULL;
  MLockGuard lockGuard(m_mutex);
  const unsigned int n = m_header->nentries;
  for (unsigned int i = 0; i < n; i++) {
    if (m_entries[i].id > 0 &&
        m_entries[i].id == (unsigned int)id) {
      return &m_entries[i];
    }
  }
  return NULL;
}

NSMDataStore::Entry* NSMDataStore::get(unsigned int addr,
                                       unsigned int id)
{
  if (id == 0) return NULL;
  MLockGuard lockGuard(m_mutex);
  const unsigned int n = m_header->nentries;
  for (unsigned int i = 0; i < n; i++) {
    if (m_entries[i].rid > 0 &&
        m_entries[i].addr == (unsigned int)addr &&
        m_entries[i].rid == (unsigned int)id) {
      return &m_entries[i];
    }
  }
  return NULL;
}

void NSMDataStore::unlink()
{
  const unsigned int n = m_header->nentries;
  for (unsigned int i = 0; i < n; i++) {
    char* name = m_entries[i].name;
    if (strlen(name) > 0) {
      std::string path = "";
      sockaddr_in sa;
      sa.sin_addr.s_addr = m_entries[i].addr;
      if (m_entries[i].addr > 0) {
        path = StringUtil::form("%s:%s",
                                inet_ntoa(sa.sin_addr), name);
      } else {
        path = StringUtil::form("127.0.0.1:%s", name);
      }
      shm_unlink(path.c_str());
    }
  }
  m_mem.unlink();
}
