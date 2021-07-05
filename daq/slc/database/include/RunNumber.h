/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_RunNumber_h
#define _Belle2_RunNumber_h

#include <vector>
#include <string>

namespace Belle2 {

  class RunNumber {

  public:
    RunNumber();
    RunNumber(const std::string& node,
              const std::string& runtype,
              int expno, int runno,
              int id = 0, long long record_time = 0);
    ~RunNumber() {}

  public:
    const std::string& getNode() const { return m_node; }
    const std::string& getRunType() const { return m_runtype; }
    int getExpNumber() const { return m_expno; }
    int getRunNumber() const { return m_runno; }
    int getId() const { return m_id; }
    long long getRecordTime() const { return m_record_time; }
    void setNode(const std::string& node) { m_node = node; }
    void setRunType(const std::string& runtype) { m_runtype = runtype; }
    void setExpNumber(int expno) { m_expno = expno; }
    void setRunNumber(int runno) { m_runno = runno; }
    void setId(int id) { m_id = id; }
    void setRecordTime(long long record_time)
    {
      m_record_time = record_time;
    }

  private:
    std::string m_node;
    std::string m_runtype;
    int m_expno;
    int m_runno;
    int m_id;
    long long m_record_time;

  };

  typedef std::vector<RunNumber> RunNumberList;

}

#endif
