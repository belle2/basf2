/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/Inotify.h"

#include <daq/slc/base/IOException.h>

#include <sys/inotify.h>
#include <unistd.h>

using namespace Belle2;

const unsigned long Inotify::FILE_CREATE(IN_CREATE);
const unsigned long Inotify::FILE_OPEN(IN_OPEN);
const unsigned long Inotify::FILE_CLOSE_WRITE(IN_CLOSE_WRITE);
const unsigned long Inotify::FILE_CLOSE_NOWRITE(IN_CLOSE_NOWRITE);
const unsigned long Inotify::FILE_DELETE(IN_DELETE);
const unsigned long Inotify::FILE_MODIFY(IN_MODIFY);
const unsigned long Inotify::FILE_ACCESS(IN_ACCESS);
const unsigned long Inotify::FILE_ATTRIB(IN_ATTRIB);

void Inotify::open()
{
  if ((m_fd = ::inotify_init()) < 0) {
    throw (IOException("Failed to initialize inotify."));
  }
}

int Inotify::add(const std::string& path, unsigned long mask)
{
  int wd = inotify_add_watch(m_fd, path.c_str(), mask);
  if (wd < 0) {
    throw (IOException("Failed to add a path"));
  }
  return wd;
}

void Inotify::remove(int wd)
{
  if (::inotify_rm_watch(m_fd, wd) < 0) {
    throw (IOException("Failed to remove a path"));
  }
}

InotifyEventList Inotify::wait(int sec)
{
  InotifyEventList ievent_v;
  //try {
  if (select(sec, 0)) {
    char buf[16384];
    int r = read(m_fd, buf, 16384);
    if (r <= 0) return ievent_v;
    int index = 0;
    while (index < r) {
      inotify_event* ev = (inotify_event*)&buf[index];
      int event_size =  sizeof(inotify_event) + ev->len;
      index += event_size;
      ievent_v.push_back(InotifyEvent(ev->wd, ev->mask, ev->name));
    }
  }
  //} catch (const IOException& e) {
  //std::cerr << "Failed to select" << std::endl;
  //}
  return ievent_v;
}
