#include "daq/slc/runcontrol/RCConfig.h"

using namespace Belle2;

RCConfig::RCConfig() : m_obj(), m_expno(0),
  m_runno(0), m_subno(0)
{

}

RCConfig::RCConfig(const RCConfig& config)
  : m_obj(config.m_obj), m_expno(config.m_expno),
    m_runno(config.m_runno), m_subno(config.m_subno)
{

}

void RCConfig::print() throw()
{
  m_obj.print();
}
