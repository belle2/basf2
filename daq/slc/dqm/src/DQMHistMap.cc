#include "daq/slc/dqm/DQMHistMap.h"

using namespace Belle2;

void DQMHistMap::addHist(TH1* hist, const char* name)
{
  if (!hasHist(name)) {
    m_hist_m.insert(TH1Map::value_type(name, hist));
  } else {
    m_hist_m[name] = hist;
  }
}

TH1* DQMHistMap::getHist(const std::string& name)
{
  return (hasHist(name)) ? m_hist_m[name] : NULL;
}

bool DQMHistMap::hasHist(const std::string& name)
{
  return m_hist_m.find(name) != m_hist_m.end();
}

void DQMHistMap::clear()
{
  for (TH1Map::iterator it = m_hist_m.begin();
       it != m_hist_m.end(); it++) {
    delete it->second;
  }
  m_hist_m = TH1Map();
}
