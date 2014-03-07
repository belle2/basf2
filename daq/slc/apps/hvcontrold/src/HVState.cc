#include "daq/slc/apps/hvcontrold/HVState.h"

using namespace Belle2;

const HVState HVState::OFF_S(2, "OFF", "HV_OFF");
const HVState HVState::STABLE_S(11, "STABLE", "HV_STABLE");
const HVState HVState::STANDBY_S(12, "STANDBY", "HV_STANDBY");
const HVState HVState::STANDBY2_S(13, "STANDBY2", "HV_STANDBY2");
const HVState HVState::STANDBY3_S(14, "STANDBY3", "HV_STANDBY3");
const HVState HVState::PEAK_S(15, "PEAK", "HV_PEAK");
const HVState HVState::TRANSITION_TS(31, "TRANSITION", "HV_TRANSITION");
const HVState HVState::RAMPINGUP_TS(32, "RAMPINGUP", "HV_RAMPINGUP");
const HVState HVState::RAMPINGDOWN_TS(33, "RAMPINGDOWN", "HV_RAMPINGDOWN");
const HVState HVState::ERROR_ES(41, "ERROR", "HV_ERROR");
const HVState HVState::TRIP_ES(42, "TRIP", "HV_TRIP");
const HVState HVState::OVER_CURRENT_ES(43, "OVER_CURRENT", "HV_OVER_CURRENT");
const HVState HVState::OVER_VOLTAGE_ES(44, "OVER_VOLTAGE", "HV_OVER_VOLTAGE");

bool HVState::isOff() const { return _id > 0 && _id < 10; }
bool HVState::isOn() const { return _id > 10 && _id < 20; }
bool HVState::isTransition() const { return _id > 30 && _id < 40; }
bool HVState::isError() const { return _id > 40; }

const HVState& HVState::operator=(const std::string& label) throw()
{
  if (label == OFF_S._label) *this = OFF_S;
  else if (label == STABLE_S._label) *this = STABLE_S;
  else if (label == STANDBY_S._label) *this = STANDBY_S;
  else if (label == STANDBY2_S._label) *this = STANDBY2_S;
  else if (label == STANDBY3_S._label) *this = STANDBY3_S;
  else if (label == PEAK_S._label) *this = PEAK_S;
  else if (label == TRANSITION_TS._label) *this = TRANSITION_TS;
  else if (label == RAMPINGUP_TS._label) *this = RAMPINGUP_TS;
  else if (label == RAMPINGDOWN_TS._label) *this = RAMPINGDOWN_TS;
  else if (label == ERROR_ES._label) *this = ERROR_ES;
  else if (label == TRIP_ES._label) *this = TRIP_ES;
  else if (label == OVER_CURRENT_ES._label) *this = OVER_CURRENT_ES;
  else if (label == OVER_VOLTAGE_ES._label) *this = OVER_VOLTAGE_ES;
  else *this = Enum::UNKNOWN;
  return *this;
}

const HVState& HVState::operator=(const char* label) throw()
{
  if (label != NULL)  *this = std::string(label);
  else *this = UNKNOWN;
  return *this;
}

const HVState& HVState::operator=(int id) throw()
{
  if (id == OFF_S._id) *this = OFF_S;
  else if (id == STABLE_S._id) *this = STABLE_S;
  else if (id == STANDBY_S._id) *this = STANDBY_S;
  else if (id == STANDBY2_S._id) *this = STANDBY2_S;
  else if (id == STANDBY3_S._id) *this = STANDBY3_S;
  else if (id == PEAK_S._id) *this = PEAK_S;
  else if (id == TRANSITION_TS._id) *this = TRANSITION_TS;
  else if (id == RAMPINGUP_TS._id) *this = RAMPINGUP_TS;
  else if (id == RAMPINGDOWN_TS._id) *this = RAMPINGDOWN_TS;
  else if (id == ERROR_ES._id) *this = ERROR_ES;
  else if (id == TRIP_ES._id) *this = TRIP_ES;
  else if (id == OVER_CURRENT_ES._id) *this = OVER_CURRENT_ES;
  else if (id == OVER_VOLTAGE_ES._id) *this = OVER_VOLTAGE_ES;
  else *this = Enum::UNKNOWN;
  return *this;
}
