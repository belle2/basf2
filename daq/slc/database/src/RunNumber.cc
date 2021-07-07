/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/database/RunNumber.h"

using namespace Belle2;

RunNumber::RunNumber()
  : m_node(), m_runtype(), m_expno(0), m_runno(0),
    m_id(0), m_record_time(0)
{
}

RunNumber::RunNumber(const std::string& node,
                     const std::string& runtype,
                     int expno, int runno,
                     int id, long long record_time)
  : m_node(node), m_runtype(runtype), m_expno(expno), m_runno(runno),
    m_id(id), m_record_time(record_time)
{
}
