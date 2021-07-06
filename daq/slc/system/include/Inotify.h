/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_Inotify_hh
#define _Belle2_Inotify_hh

#include "daq/slc/system/FileDescriptor.h"

#include <string>
#include <vector>

namespace Belle2 {

  class InotifyEvent {

    friend class Inotify;

  public:
    InotifyEvent()
      : m_wd(-1), m_mask(-1), m_name() {}
    ~InotifyEvent() {}

  private:
    InotifyEvent(int wd, unsigned long mask, const std::string& name)
      : m_wd(wd), m_mask(mask), m_name(name) {}

  public:
    int get_wd() const { return m_wd;}
    unsigned long getMask() const { return m_mask; }
    const std::string& getName() const { return m_name; }

  private:
    int m_wd;
    unsigned long m_mask;
    std::string m_name;

  };

  typedef std::vector<InotifyEvent> InotifyEventList;

  class Inotify : public FileDescriptor {

  public:
    static const unsigned long FILE_CREATE;
    static const unsigned long FILE_OPEN;
    static const unsigned long FILE_CLOSE_WRITE;
    static const unsigned long FILE_CLOSE_NOWRITE;
    static const unsigned long FILE_DELETE;
    static const unsigned long FILE_MODIFY;
    static const unsigned long FILE_ACCESS;
    static const unsigned long FILE_ATTRIB;

  public:
    Inotify() {}
    virtual ~Inotify() {}

  public:
    void open();
    int add(const std::string& path, unsigned long mask);
    void remove(int wd);
    InotifyEventList wait(int sec);

  };

}

#endif
