#ifndef _Belle2_Inotify_hh
#define _Belle2_Inotify_hh

#include "daq/slc/system/FileDescriptor.h"

#include <vector>

namespace Belle2 {

  class InotifyEvent {

    friend class Inotify;

  public:
    InotifyEvent()
      : m_wd(-1), m_mask(-1), m_name() {}
    ~InotifyEvent() throw() {}

  private:
    InotifyEvent(int wd, unsigned long mask, const std::string& name)
      : m_wd(wd), m_mask(mask), m_name(name) {}

  public:
    int get_wd() const throw() { return m_wd;}
    unsigned long getMask() const throw() { return m_mask; }
    const std::string& getName() const throw() { return m_name; }

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
    Inotify() throw() {}
    virtual ~Inotify() throw() {}

  public:
    void open() throw(IOException);
    int add(const std::string& path, unsigned long mask) throw(IOException);
    void remove(int wd) throw(IOException);
    InotifyEventList wait(int sec) throw(IOException);

  };

}

#endif
