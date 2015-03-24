#include "daq/slc/database/DAQLogMessage.h"

#include <daq/slc/nsm/NSMMessage.h>

#include <daq/slc/base/Date.h>

using namespace Belle2;

const std::string DAQLogMessage::g_tablename = "logmessage";
const int DAQLogMessage::g_revision = 1;

DAQLogMessage::DAQLogMessage() throw()
{
  m_date = Date().get();
  m_nodename = "";
  m_priority = 0;
  m_message = "";
}

DAQLogMessage::DAQLogMessage(const std::string& nodename,
                             LogFile::Priority priority,
                             const std::string& message) throw()
{
  m_date = Date().get();
  m_nodename = nodename;
  m_priority = (int)priority;
  m_message = message;
}

DAQLogMessage::DAQLogMessage(const std::string& nodename,
                             LogFile::Priority priority,
                             const std::string& message,
                             const Date& date) throw()
{
  m_date = date.get();
  m_nodename = nodename;
  m_priority = (int)priority;
  m_message = message;
}

DAQLogMessage::DAQLogMessage(const DAQLogMessage& log) throw()
{
  m_date = log.getDateInt();
  m_nodename = log.getNodeName();
  m_priority = log.getPriorityInt();
  m_message = log.getMessage();
}

bool DAQLogMessage::read(const NSMMessage& msg) throw()
{
  if (msg.getLength() > 0 && msg.getNParams() > 2) {
    setPriority((LogFile::Priority)msg.getParam(0));
    setDate(msg.getParam(1));
    StringList s = StringUtil::split(msg.getData(), '\n');
    if (s[0].size() == 0) {
      setNodeName(msg.getNodeName());
    } else {
      setNodeName(s[0]);
    }
    setMessage(StringUtil::join(s, "<br/>", 1));
    return true;
  }
  return false;
}

void DAQLogMessage::setPriority(LogFile::Priority priority) throw()
{
  m_priority = (int)priority;
}

void DAQLogMessage::setNodeName(const std::string& name) throw()
{
  m_nodename = name;
}

void DAQLogMessage::setMessage(const std::string& message) throw()
{
  m_message = message;
}

void DAQLogMessage::setDate() throw()
{
  setDate(Date());
}

void DAQLogMessage::setDate(int date) throw()
{
  m_date = date;
}

void DAQLogMessage::setDate(const Date& date) throw()
{
  m_date = date.get();
}

LogFile::Priority DAQLogMessage::getPriority() const throw()
{
  return (LogFile::Priority) m_priority;
}

int DAQLogMessage::getPriorityInt() const throw()
{
  return m_priority;
}

const std::string& DAQLogMessage::getNodeName() const throw()
{
  return m_nodename;
}

const std::string& DAQLogMessage::getMessage() const throw()
{
  return m_message;
}

int DAQLogMessage::getDateInt() const throw()
{
  return m_date;
}

const Date DAQLogMessage::getDate() const throw()
{
  return Date(m_date);
}

const std::string DAQLogMessage::getPriorityText() const throw()
{
  switch (getPriority()) {
    case LogFile::DEBUG:   return "DEBUG";
    case LogFile::INFO:    return "INFO";
    case LogFile::NOTICE:  return "NOTICE";
    case LogFile::WARNING: return "WARNING";
    case LogFile::ERROR: return "ERROR";
    case LogFile::FATAL: return "FATAL";
    default:
      break;
  }
  return "UNKNOWN";
}
