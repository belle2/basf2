#include "daq/slc/dqm/MonitorState.h"

using namespace Belle2;

const MonitorState MonitorState::UNKNOWN(-1, "UNKNOWN");
const MonitorState MonitorState::NOTUSED(0, "NOTUSED");
const MonitorState MonitorState::BOOTED(1, "BOOTED");
const MonitorState MonitorState::READY(2, "READY");
const MonitorState MonitorState::RUNNING(3, "RUNNNING");
const MonitorState MonitorState::ERROR(101, "ERROR");

MonitorState::MonitorState(int value, const std::string& text)
  : m_value(value), m_text(text) {}

MonitorState::~MonitorState() throw() {}

MonitorState::MonitorState()
{
  *this = UNKNOWN;
}

const MonitorState& MonitorState::operator=(int value) throw()
{
  if (value == BOOTED.getValue()) {
    *this = BOOTED;
  } else if (value == READY.getValue()) {
    *this = READY;
  } else if (value == NOTUSED.getValue()) {
    *this = NOTUSED;
  } else if (value == RUNNING.getValue()) {
    *this = RUNNING;
  } else if (value == ERROR.getValue()) {
    *this = ERROR;
  } else {
    *this = UNKNOWN;
  }
  return *this;
}

const MonitorState& MonitorState::operator=(const MonitorState& state)
throw()
{
  m_value = state.getValue();
  m_text = state.getText();
  return *this;
}

bool MonitorState::operator==(int value) const throw()
{
  return m_value == value;
}

bool MonitorState::operator==(const MonitorState& state) const throw()
{
  return m_value == state.m_value;
}

namespace Belle2 {

  bool operator==(int value, const MonitorState& state) throw()
  {
    return value == state.getValue();
  }

  bool operator!=(int value, const MonitorState& state) throw()
  {
    return value != state.getValue();
  }

};
