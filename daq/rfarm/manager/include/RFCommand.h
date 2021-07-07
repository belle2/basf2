/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_RFCommand_hh
#define _Belle2_RFCommand_hh

#include "daq/slc/nsm/NSMCommand.h"

namespace Belle2 {

  class State;

  class RFCommand : public NSMCommand {

  public:
    static const RFCommand CONFIGURE;
    static const RFCommand UNCONFIGURE;
    static const RFCommand START;
    static const RFCommand STOP;
    static const RFCommand RESTART;
    static const RFCommand PAUSE;
    static const RFCommand RESUME;
    static const RFCommand STATUS;

  public:
    RFCommand() {}
    RFCommand(const Enum& e) : NSMCommand(e) {}
    RFCommand(const NSMCommand& cmd) : NSMCommand(cmd) {}
    RFCommand(const RFCommand& cmd) : NSMCommand(cmd) {}
    RFCommand(const char* label) { *this = label; }
    ~RFCommand() {}

  protected:
    RFCommand(int id, const char* label)
      : NSMCommand(id, label) {}

  public:
    //virtual int isAvailable(const State& state) const;
    State nextState() const;

  public:
    const RFCommand& operator=(const std::string& label);
    const RFCommand& operator=(const char* label);

  };

}

#endif


