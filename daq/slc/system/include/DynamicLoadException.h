/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_DynamicLoadException_hh
#define _Belle2_DynamicLoadException_hh

#include "daq/slc/base/Exception.h"

namespace Belle2 {

  class DynamicLoadException : public Exception {

  public:
    DynamicLoadException(const std::string& format, ...);

  };

}

#endif
