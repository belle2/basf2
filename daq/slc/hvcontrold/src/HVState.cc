#include "HVState.h"

using namespace Belle2;

const HVState HVState::HVOFF_S(101, "HVOFF", "HVOFF");
const HVState HVState::STANDBY1_S(102, "STANDBY1", "STANDBY1");
const HVState HVState::STANDBY2_S(103, "STANDBY2", "STANDBY2");
const HVState HVState::PEAK_S(104, "PEAK", "PEAK");

const HVState& HVState::operator=(const std::string& label) throw()
{
  if (label == HVOFF_S._label) *this = HVOFF_S;
  else if (label == STANDBY1_S._label) *this = STANDBY1_S;
  else if (label == STANDBY2_S._label) *this = STANDBY2_S;
  else if (label == PEAK_S._label) *this = PEAK_S;
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
  if (id == HVOFF_S._id) *this = HVOFF_S;
  else if (id == STANDBY1_S._id) *this = STANDBY1_S;
  else if (id == STANDBY2_S._id) *this = STANDBY2_S;
  else if (id == PEAK_S._id) *this = PEAK_S;
  else *this = Enum::UNKNOWN;
  return *this;
}

State HVState::next() const throw()
{
  if (*this == BOOTING_TS) return STANDBY1_S;
  else if (*this == LOADING_TS) return STANDBY2_S;
  else if (*this == STARTING_TS) return PEAK_S;
  else return UNKNOWN;
}
