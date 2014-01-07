#include "daq/slc/dqm/DQMHistMap.h"

using namespace Belle2;

void DQMHistMap::addHist(TH1* hist)
{
  _hist_m.insert(TH1Map::value_type(hist->GetName(), hist));
}

TH1* DQMHistMap::getHist(const std::string& name)
{
  return (hasHist(name)) ? _hist_m[name] : NULL;
}

bool DQMHistMap::hasHist(const std::string& name)
{
  return _hist_m.find(name) != _hist_m.end();
}
