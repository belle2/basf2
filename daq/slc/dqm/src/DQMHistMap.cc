#include "daq/slc/dqm/DQMHistMap.h"

using namespace Belle2;

void DQMHistMap::addHist(TH1* hist, const char* name)
{
  if (!hasHist(name)) {
    _hist_m.insert(TH1Map::value_type(name, hist));
  } else {
    _hist_m[name] = hist;
  }
}

TH1* DQMHistMap::getHist(const std::string& name)
{
  return (hasHist(name)) ? _hist_m[name] : NULL;
}

bool DQMHistMap::hasHist(const std::string& name)
{
  return _hist_m.find(name) != _hist_m.end();
}

void DQMHistMap::clear()
{
  for (TH1Map::iterator it = _hist_m.begin();
       it != _hist_m.end(); it++) {
    delete it->second;
  }
  _hist_m = TH1Map();
}
