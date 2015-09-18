#include "daq/slc/database/RunNumber.h"

#include <iostream>

using namespace Belle2;

RunNumber::RunNumber()
  : m_config(), m_expno(0), m_runno(0),
    m_subno(0), m_isstart(true), m_id(0), m_record_time(0)
{

}

RunNumber::RunNumber(const std::string& config,
                     int expno, int runno, int subno,
                     bool isstart, int id, long long record_time)
  : m_config(config), m_expno(expno), m_runno(runno),
    m_subno(subno), m_isstart(isstart), m_id(id),
    m_record_time(record_time)
{
}
