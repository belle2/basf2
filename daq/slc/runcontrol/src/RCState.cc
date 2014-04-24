#include "daq/slc/runcontrol/RCState.h"

using namespace Belle2;

const RCState RCState::OFF_S(1, "OFF");
const RCState RCState::INITIAL_S(2, "INITIAL");
const RCState RCState::CONFIGURED_S(3, "CONFIGURED");
const RCState RCState::READY_S(4, "READY");
const RCState RCState::RUNNING_S(5, "RUNNING");
const RCState RCState::PAUSED_S(6, "PAUSED");
const RCState RCState::BOOTING_TS(7, "BOOTING");
const RCState RCState::LOADING_TS(8, "LOADING");
const RCState RCState::STARTING_TS(9, "STARTING");
const RCState RCState::STOPPING_TS(10, "STOPPING");
const RCState RCState::ERROR_ES(11, "ERROR");
const RCState RCState::FATAL_ES(12, "FATAL");
const RCState RCState::RECOVERING_RS(13, "RECOVERING");
const RCState RCState::ABORTING_RS(14, "ABORTING");

const RCState& RCState::operator=(const std::string& label) throw()
{
  if (label == INITIAL_S._label) *this = INITIAL_S;
  else if (label == CONFIGURED_S._label) *this = CONFIGURED_S;
  else if (label == READY_S._label) *this = READY_S;
  else if (label == RUNNING_S._label) *this = RUNNING_S;
  else if (label == PAUSED_S._label) *this = PAUSED_S;
  else if (label == BOOTING_TS._label) *this = BOOTING_TS;
  else if (label == LOADING_TS._label) *this = LOADING_TS;
  else if (label == STARTING_TS._label) *this = STARTING_TS;
  else if (label == STOPPING_TS._label) *this = STOPPING_TS;
  else if (label == ERROR_ES._label) *this = ERROR_ES;
  else if (label == FATAL_ES._label) *this = FATAL_ES;
  else if (label == RECOVERING_RS._label) *this = RECOVERING_RS;
  else if (label == ABORTING_RS._label) *this = ABORTING_RS;
  else *this = Enum::UNKNOWN;
  return *this;
}

const RCState& RCState::operator=(const char* label) throw()
{
  if (label != NULL)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

const RCState& RCState::operator=(int id) throw()
{
  if (id == INITIAL_S._id) *this = INITIAL_S;
  else if (id == CONFIGURED_S._id) *this = CONFIGURED_S;
  else if (id == READY_S._id) *this = READY_S;
  else if (id == RUNNING_S._id) *this = RUNNING_S;
  else if (id == PAUSED_S._id) *this = PAUSED_S;
  else if (id == BOOTING_TS._id) *this = BOOTING_TS;
  else if (id == LOADING_TS._id) *this = LOADING_TS;
  else if (id == STARTING_TS._id) *this = STARTING_TS;
  else if (id == STOPPING_TS._id) *this = STOPPING_TS;
  else if (id == ERROR_ES._id) *this = ERROR_ES;
  else if (id == FATAL_ES._id) *this = FATAL_ES;
  else if (id == RECOVERING_RS._id) *this = RECOVERING_RS;
  else if (id == ABORTING_RS._id) *this = ABORTING_RS;
  else *this = Enum::UNKNOWN;
  return *this;
}

RCState RCState::next() const throw()
{
  if (*this == BOOTING_TS) return CONFIGURED_S;
  else if (*this == LOADING_TS) return READY_S;
  else if (*this == STARTING_TS) return RUNNING_S;
  else if (*this == STOPPING_TS) return READY_S;
  else if (*this == RECOVERING_RS) return READY_S;
  else if (*this == ABORTING_RS) return INITIAL_S;
  else return Enum::UNKNOWN;
}
