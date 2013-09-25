#include "RCCommand.hh"

#include "RCState.hh"

using namespace B2DAQ;

const RCCommand RCCommand::UNKNOWN(-1, "RC_UNKNOWN", "UNKNOWN");
const RCCommand RCCommand::BOOT(101, "RC_BOOT", "BOOT");
const RCCommand RCCommand::REBOOT(102, "RC_REBOOT", "REBOOT");
const RCCommand RCCommand::LOAD(103, "RC_LOAD", "LOAD");
const RCCommand RCCommand::RELOAD(104, "RC_RELOAD", "RELOAD");
const RCCommand RCCommand::START(105, "RC_START", "START");
const RCCommand RCCommand::STOP(106, "RC_STOP", "STOP");
const RCCommand RCCommand::RESUME(107, "RC_RESUMEN", "RESUME");
const RCCommand RCCommand::PAUSE(108, "RC_PAUSE", "PAUSE");
const RCCommand RCCommand::RECOVER(202, "RC_RECOVER", "RECOVER");
const RCCommand RCCommand::ABORT(203, "RC_ABORT", "ABORT");
const RCCommand RCCommand::STATECHECK(301, "RC_STATECHECK", "STATECHECK");
const RCCommand RCCommand::STATE(302, "RC_STATE", "STATE");
const RCCommand RCCommand::SET(401, "RC_SET", "SET");

const RCCommand& RCCommand::operator=(const std::string& label) throw()
{
  if (label == BOOT._label) *this = BOOT;
  else if (label == REBOOT._label) *this = REBOOT;
  else if (label == LOAD._label) *this = LOAD;
  else if (label == RELOAD._label) *this = RELOAD;
  else if (label == START._label) *this = START;
  else if (label == STOP._label) *this = STOP;
  else if (label == RESUME._label) *this = RESUME;
  else if (label == PAUSE._label) *this = PAUSE;
  else if (label == RECOVER._label) *this = RECOVER;
  else if (label == ABORT._label) *this = ABORT;
  else if (label == STATECHECK._label) *this = STATECHECK;
  else if (label == STATE._label) *this = STATE;
  else if (label == SET._label) *this = SET;
  else if (label == Command::OK.getLabel()) *this = Command::OK;
  else if (label == Command::ERROR.getLabel()) *this = Command::ERROR;
  else *this = Enum::UNKNOWN;
  return *this;
}

const RCCommand& RCCommand::operator=(const char* label) throw()
{
  if (label != NULL)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

int RCCommand::isAvailable(const State& state) const throw()
{
  if (*this == BOOT && state == RCState::INITIAL_S) {
    return SUGGESTED;
  } else if (*this == REBOOT && state == RCState::CONFIGURED_S) {
    return ENABLED;
  } else if (*this == LOAD && state == RCState::CONFIGURED_S) {
    return SUGGESTED;
  } else if (*this == RELOAD && state == RCState::READY_S) {
    return ENABLED;
  } else if (*this == START && state == RCState::READY_S) {
    return SUGGESTED;
  } else if (*this == STOP && (state == RCState::RUNNING_S ||
                               state == RCState::PAUSED_S)) {
    return SUGGESTED;
  } else if (*this == PAUSE && state == RCState::RUNNING_S) {
    return ENABLED;
  } else if (*this == RESUME && state == RCState::PAUSED_S) {
    return SUGGESTED;
  } else if (*this == STATE || *this == SET || *this == STATECHECK || *this == OK ||
             *this == ERROR || *this == RECOVER || *this == ABORT) {
    return ENABLED;
  } else {
    return DISABLED;
  }
}

State RCCommand::nextState() const throw()
{
  if (*this == BOOT) return RCState::CONFIGURED_S;
  else if (*this == REBOOT) return RCState::CONFIGURED_S;
  else if (*this == LOAD) return RCState::READY_S;
  else if (*this == RELOAD) return RCState::READY_S;
  else if (*this == START) return RCState::RUNNING_S;
  else if (*this == STOP) return RCState::READY_S;
  else if (*this == RESUME) return RCState::RUNNING_S;
  else if (*this == PAUSE) return RCState::PAUSED_S;
  else return UNKNOWN;
}
