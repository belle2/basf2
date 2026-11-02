/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_RCCommand_h
#define _Belle2_RCCommand_h

#include <daq/slc/nsm/NSMCommand.h>

#include "daq/slc/runcontrol/RCState.h"

namespace Belle2 {

  class RCCommand : public NSMCommand {

  public:
    static const RCCommand CONFIGURE;
    static const RCCommand BOOT;
    static const RCCommand LOAD;
    static const RCCommand START;
    static const RCCommand STOP;
    static const RCCommand RECOVER;
    static const RCCommand RESUME;
    static const RCCommand PAUSE;
    static const RCCommand ABORT;
    static const RCCommand STATUS;

  public:
    RCCommand() {}
    // cppcheck-suppress noExplicitConstructor ; implicit conversion is part of the Enum-style API
    RCCommand(const Enum& e) : NSMCommand(e) {}
    // cppcheck-suppress noExplicitConstructor ; implicit conversion is part of the Enum-style API
    RCCommand(const NSMCommand& e) : NSMCommand(e) {}
    RCCommand(const RCCommand& cmd) : NSMCommand(cmd) {}
    explicit RCCommand(const std::string& label) { *this = label; }
    // cppcheck-suppress noExplicitConstructor ; implicit conversion is part of the Enum-style API
    RCCommand(const char* label) { *this = label; }
    explicit RCCommand(int id) { *this = id; }
    ~RCCommand() {}

  protected:
    RCCommand(int id, const char* label)
      : NSMCommand(id, label) {}

  public:
    int isAvailable(const RCState& state) const;
    RCState nextState() const;
    RCState nextTState() const;

  public:

    /**
     * Operator =.
     */
    const RCCommand& operator=(const RCCommand& command);
    // cppcheck-suppress duplInheritedMember ; the derived class deliberately provides its own
    const RCCommand& operator=(const std::string& label);
    // cppcheck-suppress duplInheritedMember ; the derived class deliberately provides its own
    const RCCommand& operator=(const char* label);
    // cppcheck-suppress duplInheritedMember ; the derived class deliberately provides its own
    const RCCommand& operator=(int id);

  };

}

#endif
