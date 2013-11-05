#include "base/State.h"

using namespace Belle2;

const State State::INITIAL_S(101, "INITIAL", "INITIAL");
const State State::CONFIGURED_S(102, "CONFIGURED", "CONFIGURED");
const State State::READY_S(103, "READY", "READY");
const State State::RUNNING_S(104, "RUNNING", "RUNNING");
const State State::PAUSED_S(105, "PAUSED", "PAUSED");
const State State::BOOTING_TS(201, "BOOTING", "BOOTING");
const State State::LOADING_TS(202, "LOADING", "LOADING");
const State State::STARTING_TS(203, "STARTING", "STARTING");
const State State::STOPPING_TS(204, "STOPPING", "STOPPING");
const State State::ERROR_ES(301, "ERROR", "ERROR");
const State State::FATAL_ES(302, "FATAL", "FATAL");
const State State::ABORTING_RS(403, "ABORTING", "ABORTING");

const State& State::operator=(const std::string& label) throw()
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
  else if (label == ABORTING_RS._label) *this = ABORTING_RS;
  else *this = Enum::UNKNOWN;
  return *this;
}

const State& State::operator=(const char* label) throw()
{
  if (label != NULL)  *this = std::string(label);
  else *this = UNKNOWN;
  return *this;
}

const State& State::operator=(int id) throw()
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
  else if (id == ABORTING_RS._id) *this = ABORTING_RS;
  else *this = Enum::UNKNOWN;
  return *this;
}

State State::next() const throw()
{
  if (*this == BOOTING_TS) return CONFIGURED_S;
  else if (*this == LOADING_TS) return READY_S;
  else if (*this == STARTING_TS) return RUNNING_S;
  else if (*this == STOPPING_TS) return READY_S;
  else return UNKNOWN;
}
