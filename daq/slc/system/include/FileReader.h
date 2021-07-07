/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_FileReader_hh
#define _Belle2_FileReader_hh

#include "daq/slc/base/Reader.h"

namespace Belle2 {

  template <class FD>
  class FileReader : public Reader {

  public:
    FileReader() {}
    FileReader(const FD& fd) : _fd(fd) {}
    virtual ~FileReader() {}

  public:
    virtual size_t read(void* v, size_t count) { return _fd.read(v, count); }
    virtual bool available() { return _fd.select(); }

  private:
    FD _fd;

  };

};

#endif
