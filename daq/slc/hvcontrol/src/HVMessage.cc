#include "daq/slc/hvcontrol/HVMessage.h"

#include <daq/slc/base/StringUtil.h>

#include <cstring>
#include <iostream>

using namespace Belle2;

HVMessage::Type HVMessage::getMessageType(const char* ptype)
{
  const std::string pname = StringUtil::tolower(ptype);
  if (pname == "all") {
    return ALL;
  } else if (pname == "switch") {
    return SWITCH;
  } else if (pname == "rampup") {
    return RAMPUP_SPEED;
  } else if (pname == "rampdown") {
    return RAMPDOWN_SPEED;
  } else if (pname == "vdemand") {
    return VOLTAGE_DEMAND;
  } else if (pname == "vlimit") {
    return VOLTAGE_LIMIT;
  } else if (pname == "climit") {
    return CURRENT_LIMIT;
  } else if (pname == "state") {
    return STATE;
  } else if (pname == "vmon") {
    return VOLTAGE_MON;
  } else if (pname == "cmon") {
    return CURRENT_MON;
  } else if (pname == "store") {
    return STORE;
  } else if (pname == "recall") {
    return RECALL;
  } else if (pname == "slot") {
    return SLOT;
  } else if (pname == "channel") {
    return CHANNEL;
  }
  return UNKNOWN;
}

HVMessage::State HVMessage::getState(const char* pstate)
{
  const std::string pname = StringUtil::tolower(pstate);
  if (pname == "on") {
    return ON;
  } else if (pname == "off") {
    return OFF;
  } else if (pname == "ovp") {
    return OVP;
  } else if (pname == "ocp") {
    return OCP;
  } else if (pname == "err") {
    return ERR;
  } else if (pname == "rampdown") {
    return RAMPDOWN;
  } else if (pname == "rampup") {
    return RAMPUP;
  } else if (pname == "trip") {
    return TRIP;
  } else if (pname == "etrip") {
    return ETRIP;
  } else if (pname == "ilock") {
    return INTERLOCK;
  }
  return ERR;
}

const std::string HVMessage::getStateText(HVMessage::State state)
{
  switch (state) {
    case OFF: return "OFF";
    case ON: return "ON";
    case OVP: return "OVP";
    case OCP: return "OCP";
    case ERR: return "ERR";
    case RAMPUP: return "RAMPUP";
    case RAMPDOWN: return "RAMPDOWN";
    case TRIP: return "TRIP";
    case ETRIP: return "ETRIP";
    case INTERLOCK: return "ILOCK";
  }
  return "UNKNOWN";
}

HVMessage::HVMessage()
{
  m_crate = (int*)&m_pars[0];
  m_slot = (int*)&m_pars[1];
  m_channel = (int*)&m_pars[2];
  m_type = (int*)&m_pars[3];
}

HVMessage::HVMessage(int crate, int slot, int channel,
                     int switchon, float rampup,
                     float rampdown, float vdemand,
                     float vlimit, float climit)
{
  m_crate = (int*)&m_pars[0];
  m_slot = (int*)&m_pars[1];
  m_channel = (int*)&m_pars[2];
  m_type = (int*)&m_pars[3];
  *m_crate = crate;
  *m_slot = slot;
  *m_channel = channel;
  *m_type = HVMessage::ALL;
  int* s = (int*)&m_pars[4];
  *s = switchon;
  m_pars[5] = rampup;
  m_pars[6] = rampdown;
  m_pars[7] = vdemand;
  m_pars[8] = vlimit;
  m_pars[9] = climit;
}

HVMessage::HVMessage(int crate, int slot,
                     int channel, int type, int switchon)
{
  m_crate = (int*)&m_pars[0];
  m_slot = (int*)&m_pars[1];
  m_channel = (int*)&m_pars[2];
  m_type = (int*)&m_pars[3];
  *m_crate = crate;
  *m_slot = slot;
  *m_channel = channel;
  *m_type = type;
  int* s = (int*)&m_pars[4];
  *s = switchon;
}

HVMessage::HVMessage(int index, int type)
{
  m_crate = (int*)&m_pars[0];
  m_slot = (int*)&m_pars[1];
  m_channel = (int*)&m_pars[2];
  m_type = (int*)&m_pars[3];
  *m_crate = index;
  *m_type = type;
}

HVMessage::HVMessage(int crate, int slot,
                     int channel, bool switchon)
{
  m_crate = (int*)&m_pars[0];
  m_slot = (int*)&m_pars[1];
  m_channel = (int*)&m_pars[2];
  m_type = (int*)&m_pars[3];
  *m_crate = crate;
  *m_slot = slot;
  *m_channel = channel;
  *m_type = HVMessage::SWITCH;
  int* s = (int*)&m_pars[4];
  *s = switchon;
}

HVMessage::HVMessage(int crate, int slot, int channel,
                     int type, float value)
{
  m_crate = (int*)&m_pars[0];
  m_slot = (int*)&m_pars[1];
  m_channel = (int*)&m_pars[2];
  m_type = (int*)&m_pars[3];
  *m_crate = crate;
  *m_slot = slot;
  *m_channel = channel;
  *m_type = type;
  m_pars[4] = value;
}

HVMessage::HVMessage(int crate, int slot, int channel,
                     int state, float vmon, float cmon)
{
  m_crate = (int*)&m_pars[0];
  m_slot = (int*)&m_pars[1];
  m_channel = (int*)&m_pars[2];
  m_type = (int*)&m_pars[3];
  *m_crate = crate;
  *m_slot = slot;
  *m_channel = channel;
  *m_type = HVMessage::MON;
  int* s = (int*)&m_pars[4];
  *s = state;
  m_pars[5] = vmon;
  m_pars[6] = vmon;
}

HVMessage::HVMessage(const HVMessage& msg)
{
  m_crate = (int*)&m_pars[0];
  m_slot = (int*)&m_pars[1];
  m_channel = (int*)&m_pars[2];
  m_type = (int*)&m_pars[3];
  memcpy(m_pars, msg.m_pars, sizeof(m_pars));
}

HVMessage::HVMessage(const int* buf)
{
  m_crate = (int*)&m_pars[0];
  m_slot = (int*)&m_pars[1];
  m_channel = (int*)&m_pars[2];
  m_type = (int*)&m_pars[3];
  memcpy(m_pars, buf, sizeof(m_pars));
}

const HVMessage& HVMessage::operator=(const HVMessage& msg)
{
  m_crate = (int*)&m_pars[0];
  m_slot = (int*)&m_pars[1];
  m_channel = (int*)&m_pars[2];
  m_type = (int*)&m_pars[3];
  memcpy(m_pars, msg.m_pars, sizeof(m_pars));
  return *this;
}

int HVMessage::getIndex() const
{
  if (*m_type == HVMessage::STORE) {
    return *m_crate;
  } else if (*m_type == HVMessage::RECALL) {
    return *m_crate;
  }
  return 0;
}

bool HVMessage::getTurnOn() const
{
  if (*m_type == HVMessage::SWITCH) {
    int* v = (int*)(&m_pars[4]);
    return *v > 0;
  } else if (*m_type == HVMessage::ALL) {
    int* v = (int*)(&m_pars[4]);
    return *v > 0;
  }
  return 0;
}

float HVMessage::getRampUpSpeed() const
{
  if (*m_type == HVMessage::RAMPUP_SPEED) {
    return m_pars[4];
  } else if (*m_type == HVMessage::ALL) {
    return m_pars[5];
  }
  return 0;
}

float HVMessage::getRampDownSpeed() const
{
  if (*m_type == HVMessage::RAMPDOWN_SPEED) {
    return m_pars[4];
  } else if (*m_type == HVMessage::ALL) {
    return m_pars[6];
  }
  return 0;
}

float HVMessage::getVoltageDemand() const
{
  if (*m_type == HVMessage::VOLTAGE_DEMAND) {
    return m_pars[4];
  } else if (*m_type == HVMessage::ALL) {
    return m_pars[7];
  }
  return 0;
}

float HVMessage::getVoltageLimit() const
{
  if (*m_type == HVMessage::VOLTAGE_LIMIT) {
    return m_pars[4];
  } else if (*m_type == HVMessage::ALL) {
    return m_pars[8];
  }
  return 0;
}

float HVMessage::getCurrentLimit() const
{
  if (*m_type == HVMessage::CURRENT_LIMIT) {
    return m_pars[4];
  } else if (*m_type == HVMessage::ALL) {
    return m_pars[9];
  }
  return 0;
}

int HVMessage::getState() const
{
  if (*m_type == HVMessage::STATE) {
    int* v = (int*)(&m_pars[4]);
    return *v;
  } else if (*m_type == HVMessage::MON) {
    int* v = (int*)(&m_pars[4]);
    return *v;
  }
  return 0;
}

float HVMessage::getVoltageMonitor() const
{
  if (*m_type == HVMessage::VOLTAGE_MON) {
    return m_pars[4];
  } else if (*m_type == HVMessage::MON) {
    return m_pars[5];
  }
  return 0;
}

float HVMessage::getCurrentMonitor() const
{
  if (*m_type == HVMessage::CURRENT_MON) {
    return m_pars[4];
  } else if (*m_type == HVMessage::MON) {
    return m_pars[6];
  }
  return 0;
}


