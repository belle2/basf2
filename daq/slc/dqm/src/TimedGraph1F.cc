#include "daq/slc/dqm/TimedGraph1F.h"

#include "daq/slc/dqm/FloatArray.h"

using namespace Belle2;

TimedGraph1F::TimedGraph1F() throw() : TimedGraph1()
{
  m_data = new FloatArray(0);
}

TimedGraph1F::TimedGraph1F(const TimedGraph1F& h) throw() : TimedGraph1(h)
{
  m_data = new FloatArray(h.m_data->size());
}

TimedGraph1F::TimedGraph1F(const std::string& name, const std::string& title,
                           int nbinx, double xmin, double xmax)
throw() : TimedGraph1(name, title, nbinx, xmin, xmax)
{
  m_data = new FloatArray(nbinx);
  m_time_v.resize(nbinx);
}

TimedGraph1F::~TimedGraph1F() throw() {}

std::string TimedGraph1F::getDataType() const throw()
{
  return std::string("TGF");
}

