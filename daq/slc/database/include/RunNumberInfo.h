#ifndef _Belle2_RunNumber_h
#define _Belle2_RunNumber_h

#include <vector>
#include <string>

namespace Belle2 {

  class RunNumberInfo {

  public:
    RunNumberInfo();
    RunNumberInfo(int expno, int runno, int subno,
                  int id = 0, long long record_time = 0);
    ~RunNumberInfo() throw() {}

  public:
    int getExpNumber() const throw() { return m_expno; }
    int getRunNumber() const throw() { return m_runno; }
    int getSubNumber() const throw() { return m_subno; }
    int getId() const throw() { return m_id; }
    long long setRecordTime() const throw() { return m_record_time; }
    void setExpNumber(int expno) throw() { m_expno = expno; }
    void setRunNumber(int runno) throw() { m_runno = runno; }
    void setSubNumber(int subno) throw() { m_subno = subno; }
    void setId(int id) throw() { m_id = id; }
    void setRecordTime(long long record_time) throw() {
      m_record_time = record_time;
    }

  private:
    int m_expno;
    int m_runno;
    int m_subno;
    int m_id;
    long long m_record_time;

  };

  typedef std::vector<RunNumberInfo> RunNumberInfoList;

}

#endif
