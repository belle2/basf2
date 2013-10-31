#ifndef _Belle2_Inotify_hh
#define _Belle2_Inotify_hh

#include "FileDescriptor.h"

#include <vector>

namespace Belle2 {

  class InotifyEvent {

    friend class Inotify;

  public:
    InotifyEvent()
      : _wd(-1), _mask(-1), _name() {}
    ~InotifyEvent() throw() {}

  private:
    InotifyEvent(int wd, unsigned long mask, const std::string& name)
      : _wd(wd), _mask(mask), _name(name) {}

  public:
    int get_wd() const throw() { return _wd;}
    unsigned long getMask() const throw() { return _mask; }
    const std::string& getName() const throw() { return _name; }

  private:
    int _wd;
    unsigned long _mask;
    std::string _name;

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
