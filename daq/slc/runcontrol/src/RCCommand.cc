#include "daq/slc/runcontrol/RCCommand.h"

#include "daq/slc/runcontrol/RCState.h"

using namespace Belle2;

const RCCommand RCCommand::CONFIGURE(101, "RC_CONFIGURE");
const RCCommand RCCommand::LOAD(102, "RC_LOAD");
const RCCommand RCCommand::START(103, "RC_START");
const RCCommand RCCommand::STOP(104, "RC_STOP");
const RCCommand RCCommand::RESUME(105, "RC_RESUME");
const RCCommand RCCommand::PAUSE(106, "RC_PAUSE");
const RCCommand RCCommand::RECOVER(107, "RC_RECOVER");
const RCCommand RCCommand::ABORT(108, "RC_ABORT");
const RCCommand RCCommand::BOOT(109, "RC_BOOT");
const RCCommand RCCommand::STATUS(110, "RC_STATUS");

const RCCommand& RCCommand::operator=(const std::string& label) throw()
{
  if (NSMCommand::operator=(label) != Enum::UNKNOWN) return *this;
  else if (label == CONFIGURE.getLabel()) *this = CONFIGURE;
  else if (label == LOAD.getLabel()) *this = LOAD;
  else if (label == START.getLabel()) *this = START;
  else if (label == STOP.getLabel()) *this = STOP;
  else if (label == RECOVER.getLabel()) *this = RECOVER;
  else if (label == RESUME.getLabel()) *this = RESUME;
  else if (label == PAUSE.getLabel()) *this = PAUSE;
  else if (label == ABORT.getLabel()) *this = ABORT;
  else if (label == BOOT.getLabel()) *this = BOOT;
  else if (label == STATUS.getLabel()) *this = STATUS;
  else *this = Enum::UNKNOWN;
  return *this;
}

const RCCommand& RCCommand::operator=(int id) throw()
{
  if (NSMCommand::operator=(id) != Enum::UNKNOWN) return *this;
  else if (id == CONFIGURE.getId()) *this = CONFIGURE;
  else if (id == LOAD.getId()) *this = LOAD;
  else if (id == START.getId()) *this = START;
  else if (id == STOP.getId()) *this = STOP;
  else if (id == RECOVER.getId()) *this = RECOVER;
  else if (id == RESUME.getId()) *this = RESUME;
  else if (id == PAUSE.getId()) *this = PAUSE;
  else if (id == ABORT.getId()) *this = ABORT;
  else if (id == BOOT.getId()) *this = BOOT;
  else if (id == STATUS.getId()) *this = STATUS;
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
  const RCCommand& cmd(*this);
  if ((cmd == LOAD || cmd == CONFIGURE) &&
      state == RCState::NOTREADY_S) {
    return SUGGESTED;
    /*  } else if ((cmd == LOAD || cmd == CONFIGURE) &&
             state == RCState::READY_S) {
    return ENABLED;
    */
  } else if (cmd == START && state == RCState::READY_S) {
    return SUGGESTED;
  } else if (cmd == STOP && (state == RCState::RUNNING_S ||
                             state == RCState::PAUSED_S)) {
    return SUGGESTED;
  } else if (cmd == PAUSE && state == RCState::RUNNING_S) {
    return ENABLED;
  } else if (cmd == RESUME && state == RCState::PAUSED_S) {
    return ENABLED;
  } else if (cmd == ABORT) {
    return ENABLED;
  } else if (cmd == RECOVER && state == RCState::ERROR_ES) {
    return ENABLED;
  } else if (cmd == BOOT && state == RCState::NOTREADY_S) {
    return ENABLED;
  } else {
    return DISABLED;
  }
}

RCState RCCommand::nextState() const throw()
{
  const RCCommand& cmd(*this);
  if (cmd == LOAD) return RCState::READY_S;
  else if (cmd == START) return RCState::RUNNING_S;
  else if (cmd == STOP) return RCState::READY_S;
  else if (cmd == RESUME) return RCState::RUNNING_S;
  else if (cmd == PAUSE) return RCState::PAUSED_S;
  else if (cmd == RECOVER) return RCState::READY_S;
  else if (cmd == ABORT) return RCState::NOTREADY_S;
  else if (cmd == BOOT) return RCState::NOTREADY_S;
  else return Enum::UNKNOWN;
}

RCState RCCommand::nextTState() const throw()
{
  const RCCommand& cmd(*this);
  if (cmd == CONFIGURE) return RCState::CONFIGURING_TS;
  else if (cmd == LOAD) return RCState::LOADING_TS;
  else if (cmd == START) return RCState::STARTING_TS;
  else if (cmd == STOP) return RCState::STOPPING_TS;
  else if (cmd == RESUME) return RCState::RUNNING_S;
  else if (cmd == PAUSE) return RCState::PAUSED_S;
  else if (cmd == RECOVER) return RCState::RECOVERING_RS;
  else if (cmd == ABORT) return RCState::ABORTING_RS;
  else if (cmd == BOOT) return RCState::BOOTING_RS;
  else return Enum::UNKNOWN;
}
