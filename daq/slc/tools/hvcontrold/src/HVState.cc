#include "HVState.h"

using namespace Belle2;

const HVState HVState::OFF_STABLE_S(101, "OFF_STABLE", "HV_OFF_STABLE");
const HVState HVState::OFF_ERROR_ES(111, "OFF_ERROR", "HV_OFF_ERROR");
const HVState HVState::ON_STABLE_S(201, "ON_STABLE", "HV_ON_STABLE");
const HVState HVState::ON_TRIP_ES(211, "ON_TRIP", "HV_ON_TRIP");
const HVState HVState::ON_OCP_ES(212, "ON_OCP", "HV_ON_OCP");
const HVState HVState::ON_OVP_ES(213, "ON_OVP", "HV_ON_OVP");
const HVState HVState::ON_RAMPINGUP_TS(221, "ON_RAMPINGUP", "HV_ON_RAMPINGUP");
const HVState HVState::ON_RAMPINGDOWN_TS(222, "ON_RAMPINGDOWN", "HV_ON_RAMPINGDOWN");

bool HVState::isOn() const { return _id > 200; }
bool HVState::isStable() const { return (_id % 100) / 10 == 0; }
bool HVState::isError() const { return (_id % 100) / 10 == 1; }

const HVState& HVState::operator=(const std::string& label) throw()
{
  if (label == OFF_STABLE_S._label) *this = OFF_STABLE_S;
  else if (label == OFF_ERROR_ES._label) *this = OFF_ERROR_ES;
  else if (label == ON_STABLE_S._label) *this = ON_STABLE_S;
  else if (label == ON_TRIP_ES._label) *this = ON_TRIP_ES;
  else if (label == ON_OCP_ES._label) *this = ON_OCP_ES;
  else if (label == ON_OVP_ES._label) *this = ON_OVP_ES;
  else if (label == ON_RAMPINGUP_TS._label) *this = ON_RAMPINGUP_TS;
  else if (label == ON_RAMPINGDOWN_TS._label) *this = ON_RAMPINGDOWN_TS;
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
  if (id == OFF_STABLE_S._id) *this = OFF_STABLE_S;
  else if (id == OFF_ERROR_ES._id) *this = OFF_ERROR_ES;
  else if (id == ON_STABLE_S._id) *this = ON_STABLE_S;
  else if (id == ON_TRIP_ES._id) *this = ON_TRIP_ES;
  else if (id == ON_OCP_ES._id) *this = ON_OCP_ES;
  else if (id == ON_OVP_ES._id) *this = ON_OVP_ES;
  else if (id == ON_RAMPINGUP_TS._id) *this = ON_RAMPINGUP_TS;
  else if (id == ON_RAMPINGDOWN_TS._id) *this = ON_RAMPINGDOWN_TS;
  else *this = Enum::UNKNOWN;
  return *this;
}
