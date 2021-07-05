/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_NSMNotConnectedException_h
#define _Belle2_NSMNotConnectedException_h

#include "daq/slc/nsm/NSMHandlerException.h"

namespace Belle2 {

  class NSMNotConnectedException : public NSMHandlerException {

  public:
    NSMNotConnectedException(const std::string& format)
      : NSMHandlerException(format) {}

  };

}

#endif
