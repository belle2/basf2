#include "daq/slc/base/Command.h"

#include "daq/slc/base/State.h"

using namespace Belle2;

const Command Command::UNKNOWN(-1, "UNKNOWN", "UNKNOWN");
const Command Command::BOOT(101, "BOOT", "BOOT");
const Command Command::LOAD(102, "LOAD", "LOAD");
const Command Command::START(103, "START", "START");
const Command Command::STOP(104, "STOP", "STOP");
const Command Command::RECOVER(108, "RECOVER", "RECOVER");
const Command Command::RESUME(105, "RESUMEN", "RESUME");
const Command Command::PAUSE(106, "PAUSE", "PAUSE");
const Command Command::ABORT(107, "ABORT", "ABORT");
const Command Command::STATECHECK(201, "STATECHECK", "STATECHECK");
const Command Command::STATE(202, "STATE", "STATE");
const Command Command::TRIGFT(301, "TRIGFT", "TRIGFT");
const Command Command::LOG(302, "LOG", "LOG");
const Command Command::DATA(401, "DATA", "DATA");
const Command Command::SAVE(402, "SAVE", "SAVE");
const Command Command::RECALL(403, "RECALL", "RECALL");
const Command Command::ACTIVATE(404, "ACTIVATE", "ACTIVATE");
const Command Command::INACTIVATE(405, "INACTIVATE", "INACTIVATE");
const Command Command::OK(1, "OK", "OK");
const Command Command::ERROR(2, "ERROR", "ERROR");
const Command Command::FATAL(3, "FATAL", "FATAL");

const Command& Command::operator=(const std::string& label) throw()
{
  if (label == BOOT._label) *this = BOOT;
  else if (label == LOAD._label) *this = LOAD;
  else if (label == START._label) *this = START;
  else if (label == STOP._label) *this = STOP;
  else if (label == RECOVER._label) *this = RECOVER;
  else if (label == RESUME._label) *this = RESUME;
  else if (label == PAUSE._label) *this = PAUSE;
  else if (label == ABORT._label) *this = ABORT;
  else if (label == STATECHECK._label) *this = STATECHECK;
  else if (label == STATE._label) *this = STATE;
  else if (label == TRIGFT._label) *this = TRIGFT;
  else if (label == LOG._label) *this = LOG;
  else if (label == OK.getLabel()) *this = OK;
  else if (label == ERROR.getLabel()) *this = ERROR;
  else if (label == FATAL.getLabel()) *this = FATAL;
  else if (label == DATA.getLabel()) *this = DATA;
  else if (label == SAVE.getLabel()) *this = SAVE;
  else if (label == RECALL.getLabel()) *this = RECALL;
  else if (label == ACTIVATE.getLabel()) *this = ACTIVATE;
  else if (label == INACTIVATE.getLabel()) *this = INACTIVATE;
  else *this = Enum::UNKNOWN;
  return *this;
}

const Command& Command::operator=(int id) throw()
{
  if (id == BOOT._id) *this = BOOT;
  else if (id == LOAD._id) *this = LOAD;
  else if (id == START._id) *this = START;
  else if (id == STOP._id) *this = STOP;
  else if (id == RECOVER._id) *this = RECOVER;
  else if (id == RESUME._id) *this = RESUME;
  else if (id == PAUSE._id) *this = PAUSE;
  else if (id == ABORT._id) *this = ABORT;
  else if (id == STATECHECK._id) *this = STATECHECK;
  else if (id == STATE._id) *this = STATE;
  else if (id == TRIGFT._id) *this = TRIGFT;
  else if (id == LOG._id) *this = LOG;
  else if (id == OK.getId()) *this = OK;
  else if (id == ERROR.getId()) *this = ERROR;
  else if (id == FATAL.getId()) *this = FATAL;
  else if (id == DATA.getId()) *this = DATA;
  else if (id == SAVE.getId()) *this = SAVE;
  else if (id == RECALL.getId()) *this = RECALL;
  else if (id == ACTIVATE.getId()) *this = ACTIVATE;
  else if (id == INACTIVATE.getId()) *this = INACTIVATE;
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
  } else if (*this == RECOVER || *this == STATECHECK || *this == STATE ||
             *this == OK || *this == TRIGFT || *this == DATA ||
             *this == SAVE || *this == RECALL || *this == ACTIVATE ||
             *this == INACTIVATE || *this == RECOVER || *this == FATAL ||
             *this == ERROR || *this == ABORT || *this == LOG) {
    return ENABLED;
  } else if (state == State::ERROR_ES) {
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
  else if (*this == RECOVER) return State::READY_S;
  else if (*this == ABORT) return State::INITIAL_S;
  else return UNKNOWN;
}

State Command::nextTState() const throw()
{
  if (*this == BOOT) return State::BOOTING_TS;
  else if (*this == LOAD) return State::LOADING_TS;
  else if (*this == START) return State::STARTING_TS;
  else if (*this == STOP) return State::STOPPING_TS;
  else if (*this == RESUME) return State::RUNNING_S;
  else if (*this == PAUSE) return State::PAUSED_S;
  else if (*this == RECOVER) return State::RECOVERING_RS;
  else if (*this == ABORT) return State::ABORTING_RS;
  else return UNKNOWN;
}
