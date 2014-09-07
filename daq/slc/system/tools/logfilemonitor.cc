#include <daq/slc/system/Inotify.h>
#include <daq/slc/system/LogFile.h>

using namespace Belle2;

int main()
{
  Inotify inotify;
  inotify.open();
  inotify.add("/home/usr/hltdaq/run/", Inotify::FILE_CREATE);
  inotify.add("/home/usr/hltdaq/run/", Inotify::FILE_OPEN);
  inotify.add("/home/usr/hltdaq/run/", Inotify::FILE_CLOSE_WRITE);
  inotify.add("/home/usr/hltdaq/run/", Inotify::FILE_MODIFY);
  while (true) {
    InotifyEventList list(inotify.wait(10));
    for (InotifyEventList::iterator it = list.begin();
         it != list.end(); it++) {
      if (it->getMask() == Inotify::FILE_CREATE) {
        LogFile::debug("file %s created", it->getName().c_str());
      } else if (it->getMask() == Inotify::FILE_OPEN) {
        LogFile::debug("file %s opened", it->getName().c_str());
      } else if (it->getMask() == Inotify::FILE_CLOSE_WRITE) {
        LogFile::debug("file %s closed", it->getName().c_str());
      } else if (it->getMask() == Inotify::FILE_MODIFY) {
        LogFile::debug("file %s modified", it->getName().c_str());
      }
    }
  }
  return 0;
}
