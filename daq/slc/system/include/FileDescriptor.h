/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_FileDescriptor_hh
#define _Belle2_FileDescriptor_hh

namespace Belle2 {

  class FileDescriptor {

  public:
    FileDescriptor();
    FileDescriptor(const FileDescriptor&);
    FileDescriptor(int fd);
    virtual ~FileDescriptor();

  public:
    int get_fd() const;
    bool select(int sec = -1, int usec = -1);
    bool select2(int sec = -1, int usec = -1);
    bool close();

  protected:
    int m_fd;

  };

}

#endif
