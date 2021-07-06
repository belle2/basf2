/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/database/DAQLogMessage.h"

#include <daq/slc/base/Date.h>

using namespace Belle2;

DAQLogMessage::DAQLogMessage()
{
  m_date = Date().get();
  m_nodename = "";
  m_priority = 0;
  m_message = "";
  m_category = 13;
  m_id = -1;
}

DAQLogMessage::DAQLogMessage(const std::string& nodename,
                             LogFile::Priority priority,
                             const std::string& message)
{
  m_date = Date().get();
  m_nodename = nodename;
  m_priority = (int)priority;
  m_message = message;
  m_category = 13;
  m_id = -1;
}

DAQLogMessage::DAQLogMessage(const std::string& nodename,
                             LogFile::Priority priority,
                             const std::string& message,
                             const Date& date)
{
  m_date = date.get();
  m_nodename = nodename;
  m_priority = (int)priority;
  m_message = message;
  m_category = 13;
  m_id = -1;
}

DAQLogMessage::DAQLogMessage(const std::string& nodename,
                             LogFile::Priority priority,
                             const std::string& category,
                             const std::string& message)
{
  m_date = Date().get();
  m_nodename = nodename;
  m_priority = (int)priority;
  m_message = message;
  setCategory(category);
  m_id = -1;
}

DAQLogMessage::DAQLogMessage(const std::string& nodename,
                             LogFile::Priority priority,
                             int category,
                             const std::string& message)
{
  m_date = Date().get();
  m_nodename = nodename;
  m_priority = (int)priority;
  m_message = message;
  setCategory(category);
  m_id = -1;
}

DAQLogMessage::DAQLogMessage(const std::string& nodename,
                             LogFile::Priority priority,
                             const std::string& category,
                             const std::string& message,
                             const Date& date)
{
  m_date = date.get();
  m_nodename = nodename;
  m_priority = (int)priority;
  m_message = message;
  setCategory(category);
  m_id = -1;
}

DAQLogMessage::DAQLogMessage(const std::string& nodename,
                             LogFile::Priority priority,
                             int category,
                             const std::string& message,
                             const Date& date)
{
  m_date = date.get();
  m_nodename = nodename;
  m_priority = (int)priority;
  m_message = message;
  setCategory(category);
  m_id = -1;
}

DAQLogMessage::DAQLogMessage(const DAQLogMessage& log)
{
  m_date = log.getDateInt();
  m_nodename = log.getNodeName();
  m_priority = log.getPriorityInt();
  m_message = log.getMessage();
  m_category = log.getCategory();
  m_id = log.getId();
}

void DAQLogMessage::setPriority(LogFile::Priority priority)
{
  m_priority = (int)priority;
}

void DAQLogMessage::setNodeName(const std::string& name)
{
  m_nodename = name;
}

int DAQLogMessage::getCategory() const
{
  return m_category;
}

const std::string DAQLogMessage::getCategoryName() const
{
  switch (m_category) {
    case 1: return "PXD";
    case 2: return "SVD";
    case 3: return "CDC";
    case 4: return "TOP";
    case 5: return "ARICH";
    case 6: return "ECL";
    case 7: return "KLM";
    case 8: return "TRG";
    case 9: return "TTD";
    case 10: return "HLT";
    case 11: return "HVC";
    case 12: return "MON";
    case 13: return "DAQ";
    default: break;
  }
  return "";
}

void DAQLogMessage::setCategory(int category)
{
  m_category = category;
}

void DAQLogMessage::setCategory(const std::string& category)
{
  if (category == "PXD") {
    m_category = 1;
  } else if (category == "SVD") {
    m_category = 2;
  } else if (category == "CDC") {
    m_category = 3;
  } else if (category == "TOP") {
    m_category = 4;
  } else if (category == "ARICH") {
    m_category = 5;
  } else if (category == "ECL") {
    m_category = 6;
  } else if (category == "KLM") {
    m_category = 7;
  } else if (category == "TRG") {
    m_category = 8;
  } else if (category == "TTD") {
    m_category = 9;
  } else if (category == "HLT") {
    m_category = 10;
  } else if (category == "HVC") {
    m_category = 11;
  } else if (category == "MON") {
    m_category = 12;
  } else if (category == "DAQ") {
    m_category = 13;
  } else {
    m_category = -1;
  }
}

void DAQLogMessage::setMessage(const std::string& message)
{
  m_message = message;
}

void DAQLogMessage::setDate()
{
  setDate(Date());
}

void DAQLogMessage::setDate(int date)
{
  m_date = date;
}

void DAQLogMessage::setDate(const Date& date)
{
  m_date = date.get();
}

LogFile::Priority DAQLogMessage::getPriority() const
{
  return (LogFile::Priority) m_priority;
}

int DAQLogMessage::getPriorityInt() const
{
  return m_priority;
}

const std::string& DAQLogMessage::getNodeName() const
{
  return m_nodename;
}

const std::string& DAQLogMessage::getMessage() const
{
  return m_message;
}

int DAQLogMessage::getDateInt() const
{
  return m_date;
}

const Date DAQLogMessage::getDate() const
{
  return Date(m_date);
}

const std::string DAQLogMessage::getPriorityText() const
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
