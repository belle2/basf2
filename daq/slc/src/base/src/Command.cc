#include "base/Command.h"

#include "base/State.h"

using namespace Belle2;

const Command Command::UNKNOWN(-1, "UNKNOWN", "UNKNOWN");
const Command Command::BOOT(101, "BOOT", "BOOT");
const Command Command::LOAD(102, "LOAD", "LOAD");
const Command Command::START(103, "START", "START");
const Command Command::STOP(104, "STOP", "STOP");
const Command Command::RESUME(105, "RESUMEN", "RESUME");
const Command Command::PAUSE(106, "PAUSE", "PAUSE");
const Command Command::ABORT(107, "ABORT", "ABORT");
const Command Command::STATECHECK(201, "STATECHECK", "STATECHECK");
const Command Command::SETPARAMS(202, "SETPARAMS", "SETPARAMS");
const Command Command::TRIGFT(301, "TRIGFT", "TRIGFT");
const Command Command::DATA(401, "DATA", "DATA");
const Command Command::OK(1, "OK", "OK");
const Command Command::ERROR(2, "ERROR", "ERROR");

const Command& Command::operator=(const std::string& label) throw()
{
  if (label == BOOT._label) *this = BOOT;
  else if (label == LOAD._label) *this = LOAD;
  else if (label == START._label) *this = START;
  else if (label == STOP._label) *this = STOP;
  else if (label == RESUME._label) *this = RESUME;
  else if (label == PAUSE._label) *this = PAUSE;
  else if (label == ABORT._label) *this = ABORT;
  else if (label == STATECHECK._label) *this = STATECHECK;
  else if (label == SETPARAMS._label) *this = SETPARAMS;
  else if (label == TRIGFT._label) *this = TRIGFT;
  else if (label == OK.getLabel()) *this = OK;
  else if (label == ERROR.getLabel()) *this = ERROR;
  else if (label == DATA.getLabel()) *this = DATA;
  else *this = Enum::UNKNOWN;
  return *this;
}

const Command& Command::operator=(const char* label) throw()
{
  if (label != NULL)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

int Command::isAvailable(const State& state) const throw()
{
  if (*this == BOOT && state == State::INITIAL_S) {
    return SUGGESTED;
  } else if (*this == BOOT && (state == State::CONFIGURED_S ||
                               state == State::READY_S)) {
    return ENABLED;
  } else if (*this == LOAD && state == State::CONFIGURED_S) {
    return SUGGESTED;
  } else if (*this == LOAD && state == State::READY_S) {
    return ENABLED;
  } else if (*this == START && state == State::READY_S) {
    return SUGGESTED;
  } else if (*this == STOP && (state == State::RUNNING_S ||
                               state == State::PAUSED_S)) {
    return SUGGESTED;
  } else if (*this == PAUSE && state == State::RUNNING_S) {
    return ENABLED;
  } else if (*this == RESUME && state == State::PAUSED_S) {
    return SUGGESTED;
  } else if (*this == STATECHECK || *this == SETPARAMS ||
             *this == OK || *this == TRIGFT || *this == DATA ||
             *this == ERROR || *this == ABORT) {
    return ENABLED;
  } else {
    return DISABLED;
  }
}

State Command::nextState() const throw()
{
  if (*this == BOOT) return State::CONFIGURED_S;
  else if (*this == LOAD) return State::READY_S;
  else if (*this == START) return State::RUNNING_S;
  else if (*this == STOP) return State::READY_S;
  else if (*this == RESUME) return State::RUNNING_S;
  else if (*this == PAUSE) return State::PAUSED_S;
  else if (*this == ABORT) return State::INITIAL_S;
  else return UNKNOWN;
}
