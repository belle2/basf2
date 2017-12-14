#include "daq/slc/dqm/DQMHistMap.h"

using namespace Belle2;

void DQMHistMap::addHist(TH1* hist,
                         const std::string& dir,
                         const std::string& name)
{
  if (!hasHist(name)) {
    m_hist_m.insert(TH1Map::value_type(name, hist));
    if (m_dir_m.find(name) == m_dir_m.end()) {
      m_ndirs++;
    }
    m_dir_m.insert(std::map<std::string, std::string>::value_type(name, dir));
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
  m_dir_m = std::map<std::string, std::string>();
  m_ndirs = 0;
}
