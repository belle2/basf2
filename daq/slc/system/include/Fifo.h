/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_Fifo_hh
#define _Belle2_Fifo_hh

#include "daq/slc/system/FileDescriptor.h"

#include <cstddef>
#include <string>

namespace Belle2 {

  class Fifo : public FileDescriptor {

  public:
    static Fifo mkfifo(const std::string& path);

  public:
    Fifo() {}
    virtual ~Fifo() {}

  public:
    void open(const std::string& path,
              const std::string& mode = "r");
    void unlink(const std::string& path);
    virtual size_t write(const void* v, size_t count);
    virtual size_t read(void* v, size_t count);

  };

}

#endif
