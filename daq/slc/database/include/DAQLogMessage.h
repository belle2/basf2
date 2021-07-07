/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_DAQLogMessage_h
#define _Belle2_DAQLogMessage_h

#include "daq/slc/base/Date.h"

#include "daq/slc/system/LogFile.h"

#include <string>

namespace Belle2 {

  class DAQLogMessage {

  public:
    DAQLogMessage();

    DAQLogMessage(const std::string& nodename,
                  LogFile::Priority priority,
                  const std::string& message);

    DAQLogMessage(const std::string& nodename,
                  LogFile::Priority priority,
                  const std::string& message,
                  const Date& date);

    DAQLogMessage(const std::string& nodename,
                  const std::string& priority,
                  const std::string& message,
                  const Date& date);

    DAQLogMessage(const std::string& nodename,
                  LogFile::Priority priority,
                  const std::string& category,
                  const std::string& message);

    DAQLogMessage(const std::string& nodename,
                  LogFile::Priority priority,
                  int category,
                  const std::string& message);

    DAQLogMessage(const std::string& nodename,
                  LogFile::Priority priority,
                  const std::string& category,
                  const std::string& message,
                  const Date& date);

    DAQLogMessage(const std::string& nodename,
                  LogFile::Priority priority,
                  int category,
                  const std::string& message,
                  const Date& date);

    DAQLogMessage(const DAQLogMessage& log);
    virtual ~DAQLogMessage() {}

  public:
    void setId(int id) { m_id = id; }
    void setPriority(const std::string& priority);
    void setPriority(LogFile::Priority priority);
    void setCategory(int category);
    void setCategory(const std::string& category);
    void setNodeName(const std::string& name);
    void setMessage(const std::string& message);
    void setDate();
    void setDate(int date);
    void setDate(const Date& date);
    int getId() const { return m_id; }
    LogFile::Priority getPriority() const;
    int getCategory() const;
    const std::string getCategoryName() const;
    int getPriorityInt() const;
    const std::string getPriorityText() const;
    const std::string& getNodeName() const;
    const std::string& getMessage() const;
    int getDateInt() const;
    const Date getDate() const;

  private:
    int m_date;
    std::string m_nodename;
    int m_priority;
    std::string m_message;
    int m_id;
    int m_category;

  };

}

#endif
