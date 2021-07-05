/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_RCHandlerFatalException_h
#define _Belle2_RCHandlerFatalException_h

#include <daq/slc/runcontrol/RCHandlerException.h>

namespace Belle2 {

  class RCHandlerFatalException : public RCHandlerException {

  public:
    RCHandlerFatalException(const std::string& comment = "");
    RCHandlerFatalException(const char* comment, ...);
#if __GNUC__ >= 7
    virtual ~RCHandlerFatalException() {}
#else
    virtual ~RCHandlerFatalException() throw() {}
#endif

  };

}

#endif
