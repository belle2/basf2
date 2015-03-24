#ifndef _Belle2_RunNumber_h
#define _Belle2_RunNumber_h

#include <vector>
#include <string>

namespace Belle2 {

  class RunNumber {

  public:
    RunNumber();
    RunNumber(const std::string& config,
              int expno, int runno, int subno,
              bool isstart = true,
              int id = 0, long long record_time = 0);
    ~RunNumber() throw() {}

  public:
    const std::string& getConfig() const throw() { return m_config; }
    int getExpNumber() const throw() { return m_expno; }
    int getRunNumber() const throw() { return m_runno; }
    int getSubNumber() const throw() { return m_subno; }
    bool isStart() const throw() { return m_isstart; }
    int getId() const throw() { return m_id; }
    long long getRecordTime() const throw() { return m_record_time; }
    void setConfig(const std::string& config) throw() { m_config = config; }
    void setExpNumber(int expno) throw() { m_expno = expno; }
    void setRunNumber(int runno) throw() { m_runno = runno; }
    void setSubNumber(int subno) throw() { m_subno = subno; }
    void setStart(bool isstart) throw() { m_isstart = isstart; }
    void setId(int id) throw() { m_id = id; }
    void setRecordTime(long long record_time) throw()
    {
      m_record_time = record_time;
    }

  private:
    std::string m_config;
    int m_expno;
    int m_runno;
    int m_subno;
    bool m_isstart;
    int m_id;
    long long m_record_time;

  };

  typedef std::vector<RunNumber> RunNumberList;

}

#endif
