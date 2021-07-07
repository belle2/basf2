/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/pcore/MsgHandler.h>

#include <daq/slc/system/SharedMemory.h>
#include <daq/slc/system/MMutex.h>

#include <TH1.h>

#include <string>
#include <vector>

namespace Belle2 {

  /**
   * Class to manipulate histograms in shared memory.
   */
  class HistMemory {

  public:
    /**
     * Header information to deseriale the shared memory.
     */
    struct Header {
      /** Number of bytes. */
      unsigned int nbytes;
      /** Id of the update. */
      unsigned int updateid;
      /** Timestamp of last update. */
      unsigned int timestamp;
    };

  public:
    /** The size of the buffer for shared memory. */
    static const unsigned int BUFFER_SIZE = 100000000; //100MB

  public:
    HistMemory() : m_path(), m_size(0),
      m_handler(0), m_fd(0), m_body(NULL), m_buf(NULL), m_updateid(0), m_header(NULL) {}
    ~HistMemory() {}

  public:
    /**
     * Open shared memory.
     * @param path The name of the shared memory.
     * @param size The size of the shared memory.
     * @param mode The open mode: read or write.
     */
    void open(const char* path, unsigned int size,
              const char* mode = "");
    /**
     * Initialize the shared memory.
     */
    void init();
    /**
     * Serialize the shared memory.
     */
    void serialize();
    /**
     * Deserialize the shared memory.
     * @param header The Header of the shared memory.
     */
    std::vector<TH1*>& deserialize(Header* header = NULL);
    /**
     * Get the list of the histograms.
     */
    std::vector<TH1*>& get() { return m_hist; }
    /**
     * Add histogram to the list of histograms.
     * @param h The histogram to be added.
     */
    TH1* add(TH1* h)
    {
      if (h == NULL) return NULL;
      m_hist.push_back(h);
      return h;
    }

  private:
    /** The name of the shared memory. */
    std::string m_path;
    /** The size of the shared memory. */
    unsigned int m_size;
    /** The message handler. */
    MsgHandler m_handler;
    /** The file descriptor. */
    int m_fd;
    /** The pointer to the body of the message. */
    char* m_body;
    /** The buffer to hold the message. */
    char* m_buf;
    /** The id of the udpate. */
    unsigned int m_updateid;
    /** The shared memory to hold the histograms. */
    SharedMemory m_memory;
    /** The mutex lock for the shared memory. */
    MMutex m_mutex;
    /** The header for the message. */
    Header* m_header;
    /** The list of the histograms. */
    std::vector<TH1*> m_hist;

  };

}
