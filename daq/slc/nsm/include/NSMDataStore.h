/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_NSMDataHeader_h
#define _Belle2_NSMDataHeader_h

#include <daq/slc/system/SharedMemory.h>
#include "daq/slc/system/MMutex.h"
#include "daq/slc/system/MCond.h"

namespace Belle2 {

  class NSMDataStore {

  public:
    static NSMDataStore& getStore()
    {
      return g_store;
    }

  private:
    static NSMDataStore g_store;

  public:
    struct Header {
      unsigned short maxentries;
      unsigned short nentries;
    };

  public:
    struct Entry {
      unsigned short id;
      unsigned short rid;
      unsigned int addr;
      unsigned int size;
      unsigned int utime;
      unsigned int revision;
      char name[32];
      char format[32];
    };

  private:
    NSMDataStore()
    {
      m_buf = NULL;
      m_header = NULL;
      m_entries = NULL;
    }

  public:
    bool isOpend() { return m_buf != NULL; }
    bool open(unsigned short max = 0);
    void init();
    Entry* add(unsigned int addr,
               unsigned int size,
               unsigned int revision,
               const std::string& name,
               const std::string& format,
               unsigned int rid);
    void unlink();
    Header* getHeader() { return m_header; }
    Entry* get(const std::string& name);
    Entry* get(unsigned int id);
    Entry* get(unsigned int addr, unsigned int rid);
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }
    bool wait(int second)
    {
      return m_cond.wait(m_mutex, second, 0);
    }
    void signal() { m_cond.broadcast(); }

  private:
    char* m_buf;
    Header* m_header;
    Entry* m_entries;
    SharedMemory m_mem;
    MMutex m_mutex;
    MCond m_cond;
  };

}

#endif
