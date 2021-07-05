/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_SharedMemory_h
#define _Belle2_SharedMemory_h

#include <string>

namespace Belle2 {

  class SharedMemory {

  public:
    static bool unlink(const std::string& path);

    // constructors and destructor
  public:
    SharedMemory();
    SharedMemory(const std::string& path, size_t size);
    SharedMemory(const SharedMemory&);
    ~SharedMemory();

    // member functions;
  public:
    bool open(const std::string& path, size_t size);
    bool open();
    void close();
    bool unlink();
    bool seekTo(size_t);
    bool seekBy(size_t);
    void* map(size_t, size_t);
    void* map();
    bool isOpened();
    const std::string& getPath() const { return m_path; }
    size_t size() const { return m_size; }
    int getfd() const { return m_fd; }
    bool truncate(size_t size);

    // operators
  public:
    const SharedMemory& operator=(const SharedMemory&);

    // data members;
  private:
    int m_fd;
    std::string m_path;
    size_t m_size;
    void* m_addr;

  };

};

#endif
