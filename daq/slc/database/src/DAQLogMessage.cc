#include "daq/slc/database/DAQLogMessage.h"

#include <daq/slc/nsm/NSMMessage.h>

#include <daq/slc/base/Date.h>

using namespace Belle2;

const std::string DAQLogMessage::g_tablename = "logmessage";
const int DAQLogMessage::g_revision = 1;

DAQLogMessage::DAQLogMessage() throw()
{
  addInt("date", Date().get());
  addText("nodename", "");
  addChar("priority", 0);
  addText("message", "");
}

DAQLogMessage::DAQLogMessage(const std::string& nodename,
                             LogFile::Priority priority,
                             const std::string& message) throw()
{
  addInt("date", Date().get());
  addText("nodename", nodename);
  addChar("priority", 0);
  setPriority(priority);
  addText("message", message);
}

DAQLogMessage::DAQLogMessage(const std::string& nodename,
                             LogFile::Priority priority,
                             const std::string& message,
                             const Date& date) throw()
{
  addInt("date", date.get());
  addText("nodename", nodename);
  addChar("priority", 0);
  setPriority(priority);
  addText("message", message);
}

DAQLogMessage::DAQLogMessage(const DAQLogMessage& log) throw()
  : DBObject(log)
{
  addInt("date", 0);
  addText("nodename", "");
  addChar("priority", 0);
  addText("message", "");
}

bool DAQLogMessage::read(const NSMMessage& msg) throw()
{
  if (msg.getLength() > 0 && msg.getNParams() > 2) {
    setPriority((LogFile::Priority)msg.getParam(0));
    setDate(msg.getParam(1));
    setNodeName(msg.getNodeName());
    setMessage(msg.getData());
    return true;
  }
  return false;
}

void DAQLogMessage::setPriority(LogFile::Priority priority) throw()
{
  setChar("priority", (char)priority);
}

void DAQLogMessage::setNodeName(const std::string& name) throw()
{
  setText("nodename", name);
}

void DAQLogMessage::setMessage(const std::string& message) throw()
{
  setText("message", message);
}

void DAQLogMessage::setDate() throw()
{
  setDate(Date());
}

void DAQLogMessage::setDate(int date) throw()
{
  setInt("date", date);
}

void DAQLogMessage::setDate(const Date& date) throw()
{
  setDate(date.get());
}

LogFile::Priority DAQLogMessage::getPriority() const throw()
{
  return (LogFile::Priority) getChar("priority");
}

int DAQLogMessage::getPriorityInt() const throw()
{
  return getChar("priority");
}

const std::string DAQLogMessage::getNodeName() const throw()
{
  return getText("nodename");
}

const std::string DAQLogMessage::getMessage() const throw()
{
  return getText("message");
}

int DAQLogMessage::getDateInt() const throw()
{
  return getInt("date");
}

const Date DAQLogMessage::getDate() const throw()
{
  return Date(getInt("date"));
}

