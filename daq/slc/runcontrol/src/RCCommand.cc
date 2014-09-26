#include "daq/slc/runcontrol/RCCommand.h"

#include "daq/slc/runcontrol/RCState.h"

using namespace Belle2;

const RCCommand RCCommand::LOAD(102, "LOAD");
const RCCommand RCCommand::START(103, "START");
const RCCommand RCCommand::STOP(104, "STOP");
const RCCommand RCCommand::RESUME(105, "RESUME");
const RCCommand RCCommand::PAUSE(106, "PAUSE");
const RCCommand RCCommand::RECOVER(107, "RECOVER");
const RCCommand RCCommand::ABORT(108, "ABORT");
const RCCommand RCCommand::TRIGFT(109, "TRIGFT");
const RCCommand RCCommand::STATECHECK(110, "STATECHECK");
const RCCommand RCCommand::BOOT(111, "BOOT");

const RCCommand& RCCommand::operator=(const std::string& label) throw()
{
  if (NSMCommand::operator=(label) != Enum::UNKNOWN) return *this;
  else if (label == LOAD.getLabel()) *this = LOAD;
  else if (label == START.getLabel()) *this = START;
  else if (label == STOP.getLabel()) *this = STOP;
  else if (label == RECOVER.getLabel()) *this = RECOVER;
  else if (label == RESUME.getLabel()) *this = RESUME;
  else if (label == PAUSE.getLabel()) *this = PAUSE;
  else if (label == ABORT.getLabel()) *this = ABORT;
  else if (label == TRIGFT.getLabel()) *this = TRIGFT;
  else if (label == STATECHECK.getLabel()) *this = STATECHECK;
  else if (label == BOOT.getLabel()) *this = BOOT;
  else *this = Enum::UNKNOWN;
  return *this;
}

const RCCommand& RCCommand::operator=(int id) throw()
{
  if (NSMCommand::operator=(id) != Enum::UNKNOWN) return *this;
  else if (id == LOAD.getId()) *this = LOAD;
  else if (id == START.getId()) *this = START;
  else if (id == STOP.getId()) *this = STOP;
  else if (id == RECOVER.getId()) *this = RECOVER;
  else if (id == RESUME.getId()) *this = RESUME;
  else if (id == PAUSE.getId()) *this = PAUSE;
  else if (id == ABORT.getId()) *this = ABORT;
  else if (id == STATECHECK.getId()) *this = STATECHECK;
  else if (id == BOOT.getId()) *this = BOOT;
  else if (id == TRIGFT.getId()) *this = TRIGFT;
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
  if ((*this == LOAD || *this == BOOT) &&
      state == RCState::NOTREADY_S) {
    return SUGGESTED;
  } else if ((*this == LOAD || *this == BOOT || *this == TRIGFT) &&
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
  if (*this == BOOT) return RCState::NOTREADY_S;
  else if (*this == LOAD) return RCState::READY_S;
  else if (*this == TRIGFT) return RCState::READY_S;
  else if (*this == START) return RCState::RUNNING_S;
  else if (*this == STOP) return RCState::READY_S;
  else if (*this == RESUME) return RCState::RUNNING_S;
  else if (*this == PAUSE) return RCState::PAUSED_S;
  else if (*this == RECOVER) return RCState::READY_S;
  else if (*this == ABORT) return RCState::NOTREADY_S;
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
