/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/runcontrol/RCState.h"

using namespace Belle2;

const RCState RCState::OFF_S(1, "OFF");
const RCState RCState::NOTREADY_S(2, "NOTREADY");
const RCState RCState::READY_S(3, "READY");
const RCState RCState::RUNNING_S(4, "RUNNING");
const RCState RCState::PAUSED_S(5, "PAUSED");
const RCState RCState::LOADING_TS(6, "LOADING");
const RCState RCState::STARTING_TS(7, "STARTING");
const RCState RCState::STOPPING_TS(8, "STOPPING");
const RCState RCState::CONFIGURING_TS(9, "CONFIGURING");
const RCState RCState::ERROR_ES(10, "ERROR");
const RCState RCState::FATAL_ES(11, "FATAL");
const RCState RCState::BOOTING_RS(12, "BOOTING");
const RCState RCState::RECOVERING_RS(13, "RECOVERING");
const RCState RCState::ABORTING_RS(14, "ABORTING");

const RCState& RCState::operator=(const std::string& label)
{
  if (label == NOTREADY_S.getLabel()) *this = NOTREADY_S;
  else if (label == READY_S.getLabel()) *this = READY_S;
  else if (label == RUNNING_S.getLabel()) *this = RUNNING_S;
  else if (label == PAUSED_S.getLabel()) *this = PAUSED_S;
  else if (label == LOADING_TS.getLabel()) *this = LOADING_TS;
  else if (label == STARTING_TS.getLabel()) *this = STARTING_TS;
  else if (label == STOPPING_TS.getLabel()) *this = STOPPING_TS;
  else if (label == CONFIGURING_TS.getLabel()) *this = CONFIGURING_TS;
  else if (label == ERROR_ES.getLabel()) *this = ERROR_ES;
  else if (label == FATAL_ES.getLabel()) *this = FATAL_ES;
  else if (label == RECOVERING_RS.getLabel()) *this = RECOVERING_RS;
  else if (label == ABORTING_RS.getLabel()) *this = ABORTING_RS;
  else if (label == BOOTING_RS.getLabel()) *this = BOOTING_RS;
  else *this = Enum::UNKNOWN;
  return *this;
}

const RCState& RCState::operator=(const char* label)
{
  if (label != NULL)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

const RCState& RCState::operator=(int id)
{
  if (id == NOTREADY_S.getId()) *this = NOTREADY_S;
  else if (id == READY_S.getId()) *this = READY_S;
  else if (id == RUNNING_S.getId()) *this = RUNNING_S;
  else if (id == PAUSED_S.getId()) *this = PAUSED_S;
  else if (id == LOADING_TS.getId()) *this = LOADING_TS;
  else if (id == STARTING_TS.getId()) *this = STARTING_TS;
  else if (id == STOPPING_TS.getId()) *this = STOPPING_TS;
  else if (id == CONFIGURING_TS.getId()) *this = CONFIGURING_TS;
  else if (id == ERROR_ES.getId()) *this = ERROR_ES;
  else if (id == FATAL_ES.getId()) *this = FATAL_ES;
  else if (id == RECOVERING_RS.getId()) *this = RECOVERING_RS;
  else if (id == ABORTING_RS.getId()) *this = ABORTING_RS;
  else if (id == BOOTING_RS.getId()) *this = BOOTING_RS;
  else *this = Enum::UNKNOWN;
  return *this;
}

RCState RCState::next() const
{
  if (*this == LOADING_TS) return READY_S;
  else if (*this == STARTING_TS) return RUNNING_S;
  else if (*this == STOPPING_TS) return READY_S;
  else if (*this == RECOVERING_RS) return READY_S;
  else if (*this == ABORTING_RS) return NOTREADY_S;
  else if (*this == BOOTING_RS) return NOTREADY_S;
  else if (*this == CONFIGURING_TS) return NOTREADY_S;
  else return Enum::UNKNOWN;
}
