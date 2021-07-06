/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_IOException_hh
#define _Belle2_IOException_hh

#include "daq/slc/base/Exception.h"

namespace Belle2 {

  class IOException : public Exception {

  public:
    IOException() {}
    IOException(const std::string& comment, ...);
    IOException(int err, const std::string& comment, ...);
#if __GNUC__ >= 7
    virtual ~IOException() {}
#else
    virtual ~IOException() throw() {}
#endif

  };

}

#endif
