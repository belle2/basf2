#include "daq/slc/database/RunNumberInfo.h"

using namespace Belle2;

RunNumberInfo::RunNumberInfo()
  : m_expno(0), m_runno(0),
    m_subno(0), m_id(0), m_record_time(0)
{

}

RunNumberInfo::RunNumberInfo(int expno, int runno, int subno,
                             int id, long long record_time)
  : m_expno(expno), m_runno(runno),
    m_subno(subno), m_id(id),
    m_record_time(record_time)
{
}
