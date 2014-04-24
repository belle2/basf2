#include "daq/slc/runcontrol/RCCommand.h"

#include "daq/slc/runcontrol/RCState.h"

using namespace Belle2;

const RCCommand RCCommand::BOOT(101, "BOOT");
const RCCommand RCCommand::LOAD(102, "LOAD");
const RCCommand RCCommand::START(103, "START");
const RCCommand RCCommand::STOP(104, "STOP");
const RCCommand RCCommand::RECOVER(108, "RECOVER");
const RCCommand RCCommand::RESUME(105, "RESUMEN");
const RCCommand RCCommand::PAUSE(106, "PAUSE");
const RCCommand RCCommand::ABORT(107, "ABORT");
const RCCommand RCCommand::STATECHECK(201, "STATECHECK");
const RCCommand RCCommand::TRIGFT(301, "TRIGFT");

const RCCommand& RCCommand::operator=(const std::string& label) throw()
{
  if (NSMCommand::operator=(label) != Enum::UNKNOWN) return *this;
  else if (label == BOOT._label) *this = BOOT;
  else if (label == LOAD._label) *this = LOAD;
  else if (label == START._label) *this = START;
  else if (label == STOP._label) *this = STOP;
  else if (label == RECOVER._label) *this = RECOVER;
  else if (label == RESUME._label) *this = RESUME;
  else if (label == PAUSE._label) *this = PAUSE;
  else if (label == ABORT._label) *this = ABORT;
  else if (label == STATECHECK._label) *this = STATECHECK;
  else if (label == TRIGFT._label) *this = TRIGFT;
  else *this = Enum::UNKNOWN;
  return *this;
}

const RCCommand& RCCommand::operator=(int id) throw()
{
  if (NSMCommand::operator=(id) != Enum::UNKNOWN) return *this;
  else if (id == BOOT._id) *this = BOOT;
  else if (id == LOAD._id) *this = LOAD;
  else if (id == START._id) *this = START;
  else if (id == STOP._id) *this = STOP;
  else if (id == RECOVER._id) *this = RECOVER;
  else if (id == RESUME._id) *this = RESUME;
  else if (id == PAUSE._id) *this = PAUSE;
  else if (id == ABORT._id) *this = ABORT;
  else if (id == STATECHECK._id) *this = STATECHECK;
  else if (id == TRIGFT._id) *this = TRIGFT;
  else *this = Enum::UNKNOWN;
  return *this;
}

const RCCommand& RCCommand::operator=(const char* label) throw()
{
  if (label != NULL)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

int RCCommand::isAvailable(const RCState& state) const throw()
{
  if (*this == BOOT && state == RCState::INITIAL_S) {
    return SUGGESTED;
  } else if (*this == BOOT && (state == RCState::CONFIGURED_S ||
                               state == RCState::READY_S)) {
    return ENABLED;
  } else if (*this == LOAD && state == RCState::CONFIGURED_S) {
    return SUGGESTED;
  } else if ((*this == LOAD || *this == TRIGFT) &&
             state == RCState::READY_S) {
    return ENABLED;
  } else if (*this == START && state == RCState::READY_S) {
    return SUGGESTED;
  } else if (*this == STOP && (state == RCState::RUNNING_S ||
                               state == RCState::PAUSED_S)) {
    return SUGGESTED;
  } else if (*this == PAUSE && state == RCState::RUNNING_S) {
    return ENABLED;
  } else if (*this == RESUME && state == RCState::PAUSED_S) {
    return ENABLED;
  } else if (*this == RECOVER || *this == STATECHECK ||
             *this == ABORT || *this == RECOVER) {
    return ENABLED;
  } else if (state == RCState::ERROR_ES) {
    return ENABLED;
  } else {
    return DISABLED;
  }
}

RCState RCCommand::nextState() const throw()
{
  if (*this == BOOT) return RCState::CONFIGURED_S;
  else if (*this == LOAD) return RCState::READY_S;
  else if (*this == TRIGFT) return RCState::READY_S;
  else if (*this == START) return RCState::RUNNING_S;
  else if (*this == STOP) return RCState::READY_S;
  else if (*this == RESUME) return RCState::RUNNING_S;
  else if (*this == PAUSE) return RCState::PAUSED_S;
  else if (*this == RECOVER) return RCState::READY_S;
  else if (*this == ABORT) return RCState::INITIAL_S;
  else return Enum::UNKNOWN;
}

RCState RCCommand::nextTState() const throw()
{
  if (*this == BOOT) return RCState::BOOTING_TS;
  else if (*this == LOAD) return RCState::LOADING_TS;
  else if (*this == TRIGFT) return RCState::LOADING_TS;
  else if (*this == START) return RCState::STARTING_TS;
  else if (*this == STOP) return RCState::STOPPING_TS;
  else if (*this == RESUME) return RCState::RUNNING_S;
  else if (*this == PAUSE) return RCState::PAUSED_S;
  else if (*this == RECOVER) return RCState::RECOVERING_RS;
  else if (*this == ABORT) return RCState::ABORTING_RS;
  else return Enum::UNKNOWN;
}
