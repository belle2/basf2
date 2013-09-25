#include "RCState.hh"

using namespace B2DAQ;

const RCState RCState::INITIAL_S(101, "RC_INITIAL", "INITIAL");
const RCState RCState::CONFIGURED_S(102, "RC_CONFIGURED", "CONFIGURED");
const RCState RCState::READY_S(103, "RC_READY", "READY");
const RCState RCState::RUNNING_S(104, "RC_RUNNING", "RUNNING");
const RCState RCState::PAUSED_S(105, "RC_PAUSED", "PAUSED");
const RCState RCState::BOOTING_TS(201, "RC_BOOTING", "BOOTING");
const RCState RCState::LOADING_TS(202, "RC_LOADING", "LOADING");
const RCState RCState::STARTING_TS(203, "RC_STARTING", "STARTING");
const RCState RCState::STOPPING_TS(204, "RC_STOPPING", "STOPPING");
const RCState RCState::ERROR_ES(301, "RC_ERROR", "ERROR");
const RCState RCState::FATAL_ES(302, "RC_FATAL", "FATAL");
const RCState RCState::RECOVERING_RS(402, "RC_RECOVERING", "RECOVERING");
const RCState RCState::ABORTING_RS(403, "RC_ABORTING", "ABORTING");

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
  else *this = UNKNOWN;
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
  else return UNKNOWN;
}
