#include "daq/slc/hvcontrol/HVState.h"

using namespace Belle2;

const HVState HVState::OFF_S(2, "OFF");
const HVState HVState::STABLE_S(11, "STABLE");
const HVState HVState::STANDBY_S(12, "STANDBY");
const HVState HVState::SHOULDER_S(13, "SHOULDER");
const HVState HVState::PEAK_S(14, "PEAK");
const HVState HVState::TRANSITION_TS(31, "TRANSITION");
const HVState HVState::RAMPINGUP_TS(32, "RAMPINGUP");
const HVState HVState::RAMPINGDOWN_TS(33, "RAMPINGDOWN");
const HVState HVState::RECOVERING_TS(34, "RECOVERING");
const HVState HVState::TURNINGON_TS(35, "TURNINGON");
const HVState HVState::TURNINGOFF_TS(35, "TURNINGOFF");
const HVState HVState::ERROR_ES(41, "ERROR");
const HVState HVState::TRIP_ES(42, "TRIP");
const HVState HVState::OVER_CURRENT_ES(43, "OVER_CURRENT");
const HVState HVState::OVER_VOLTAGE_ES(44, "OVER_VOLTAGE");
const HVState HVState::MASKED(51, "MASKED");

bool HVState::isOff() const { return getId() > 0 && getId() < 10; }
bool HVState::isOn() const { return getId() > 10 && getId() < 20; }
bool HVState::isTransition() const { return getId() > 30 && getId() < 40; }
bool HVState::isError() const { return getId() > 40; }

const HVState& HVState::operator=(const std::string& label) throw()
{
  if (label == OFF_S.getLabel()) *this = OFF_S;
  else if (label == STABLE_S.getLabel()) *this = STABLE_S;
  else if (label == STANDBY_S.getLabel()) *this = STANDBY_S;
  else if (label == SHOULDER_S.getLabel()) *this = SHOULDER_S;
  else if (label == PEAK_S.getLabel()) *this = PEAK_S;
  else if (label == TRANSITION_TS.getLabel()) *this = TRANSITION_TS;
  else if (label == RAMPINGUP_TS.getLabel()) *this = RAMPINGUP_TS;
  else if (label == RAMPINGDOWN_TS.getLabel()) *this = RAMPINGDOWN_TS;
  else if (label == TURNINGON_TS.getLabel()) *this = TURNINGON_TS;
  else if (label == TURNINGOFF_TS.getLabel()) *this = TURNINGOFF_TS;
  else if (label == RECOVERING_TS.getLabel()) *this = RECOVERING_TS;
  else if (label == ERROR_ES.getLabel()) *this = ERROR_ES;
  else if (label == TRIP_ES.getLabel()) *this = TRIP_ES;
  else if (label == OVER_CURRENT_ES.getLabel()) *this = OVER_CURRENT_ES;
  else if (label == OVER_VOLTAGE_ES.getLabel()) *this = OVER_VOLTAGE_ES;
  else if (label == MASKED.getLabel()) *this = MASKED;
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
  if (id == OFF_S.getId()) *this = OFF_S;
  else if (id == STABLE_S.getId()) *this = STABLE_S;
  else if (id == STANDBY_S.getId()) *this = STANDBY_S;
  else if (id == SHOULDER_S.getId()) *this = SHOULDER_S;
  else if (id == PEAK_S.getId()) *this = PEAK_S;
  else if (id == TRANSITION_TS.getId()) *this = TRANSITION_TS;
  else if (id == RAMPINGUP_TS.getId()) *this = RAMPINGUP_TS;
  else if (id == RAMPINGDOWN_TS.getId()) *this = RAMPINGDOWN_TS;
  else if (id == RECOVERING_TS.getId()) *this = RECOVERING_TS;
  else if (id == TURNINGON_TS.getId()) *this = TURNINGON_TS;
  else if (id == TURNINGOFF_TS.getId()) *this = TURNINGOFF_TS;
  else if (id == ERROR_ES.getId()) *this = ERROR_ES;
  else if (id == TRIP_ES.getId()) *this = TRIP_ES;
  else if (id == OVER_CURRENT_ES.getId()) *this = OVER_CURRENT_ES;
  else if (id == OVER_VOLTAGE_ES.getId()) *this = OVER_VOLTAGE_ES;
  else if (id == MASKED.getId()) *this = MASKED;
  else *this = Enum::UNKNOWN;
  return *this;
}
