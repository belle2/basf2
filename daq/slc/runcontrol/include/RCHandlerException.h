/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_RCHandlerException_h
#define _Belle2_RCHandlerException_h

#include <daq/slc/nsm/NSMHandlerException.h>

namespace Belle2 {

  class RCHandlerException : public NSMHandlerException {

  public:
    RCHandlerException(const std::string& comment = "");
    RCHandlerException(const char* comment, ...);
#if __GNUC__ >= 7
    virtual ~RCHandlerException() {}
#else
    virtual ~RCHandlerException() throw() {}
#endif

  };

}

#endif
