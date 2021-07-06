/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_TimeoutException_hh
#define _Belle2_TimeoutException_hh

#include "daq/slc/base/IOException.h"

namespace Belle2 {

  class TimeoutException : public IOException {

  public:
    TimeoutException() {}
    TimeoutException(const std::string& comment, ...);
    TimeoutException(int err, const std::string& comment, ...);
#if __GNUC__ >= 7
    ~TimeoutException() {}
#else
    ~TimeoutException() throw() {}
#endif

  };

}

#endif
