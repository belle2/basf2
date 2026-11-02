/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_FileWriter_hh
#define _Belle2_FileWriter_hh

#include "daq/slc/base/Writer.h"

namespace Belle2 {

  template <class FD>
  class FileWriter : public Writer {

  public:
    FileWriter() {}
    explicit FileWriter(const FD& fd) : _fd(fd) {}
    virtual ~FileWriter() override {}

  public:
    virtual size_t write(const void* v, size_t count) override
    {
      return _fd.write(v, count);
    }
    virtual bool available() override
    {
      return _fd.select();
    }

  private:
    FD _fd;

  };

};

#endif
