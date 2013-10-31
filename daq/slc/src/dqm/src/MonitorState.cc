#include "MonitorState.h"

using namespace Belle2;

const MonitorState MonitorState::UNKNOWN(-1, "UNKNOWN");
const MonitorState MonitorState::NOTUSED(0, "NOTUSED");
const MonitorState MonitorState::BOOTED(1, "BOOTED");
const MonitorState MonitorState::READY(2, "READY");
const MonitorState MonitorState::RUNNING(3, "RUNNNING");
const MonitorState MonitorState::ERROR(101, "ERROR");

MonitorState::MonitorState(int value, const std::string& text)
  : _value(value), _text(text) {}

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
  _value = state.getValue();
  _text = state.getText();
  return *this;
}

bool MonitorState::operator==(int value) const throw()
{
  return _value == value;
}

bool MonitorState::operator==(const MonitorState& state) const throw()
{
  return _value == state._value;
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
