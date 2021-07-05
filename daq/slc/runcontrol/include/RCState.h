/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_RCState_hh
#define _Belle2_RCState_hh

#include <daq/slc/nsm/NSMState.h>

namespace Belle2 {

  class RCState : public NSMState {

  public:
    static const RCState OFF_S;
    static const RCState NOTREADY_S;
    static const RCState READY_S;
    static const RCState RUNNING_S;
    static const RCState PAUSED_S;
    static const RCState CONFIGURING_TS;
    static const RCState LOADING_TS;
    static const RCState STARTING_TS;
    static const RCState STOPPING_TS;
    static const RCState ERROR_ES;
    static const RCState FATAL_ES;
    static const RCState RECOVERING_RS;
    static const RCState ABORTING_RS;
    static const RCState BOOTING_RS;

  public:
    RCState() : NSMState(Enum::UNKNOWN) {}
    RCState(const Enum& e) : NSMState(e) {}
    RCState(const NSMState& num) : NSMState(num) {}
    RCState(const RCState& st) : NSMState(st) {}
    RCState(const char* st) { *this = st; }
    RCState(const std::string& st) { *this = st; }
    RCState(int id) { *this = id; }
    ~RCState()  {}

  protected:
    RCState(int id, const char* label)
      : NSMState(id, label) {}

  public:
    bool isStable() const { return getId() > 1 && getId() <= 5; }
    bool isTransition() const { return getId() > 5 && getId() <= 9; }
    bool isError() const { return getId() > 9 && getId() <= 11; }
    bool isRecovering() const { return getId() > 11 && getId() <= 14; }
    RCState next() const;

  public:
    const RCState& operator=(const std::string& msg);
    const RCState& operator=(const char* msg);
    const RCState& operator=(int id);

  };

}

#endif
