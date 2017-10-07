#include <calibration/CalibObjManager.h>

#include <string>

#include <TKey.h>

using namespace std;
using namespace Belle2;
using namespace Calibration;

void CalibObjManager::addObject(string name, unique_ptr<CalibRootObjBase> object)
{
  if (m_templateObjects.find(name) != m_templateObjects.end()) {
    m_templateObjects[name].reset();
  }
  m_templateObjects[name] = move(object);
}

void CalibObjManager::createDirectories()
{
  for (auto& x : m_templateObjects) {
    if (m_dir->GetDirectory(x.first.c_str()) == 0) {
      m_dir->mkdir(x.first.c_str());
      B2DEBUG(100, "Made TDirectory: " << x.first);
    }
  }
}

string CalibObjManager::getSuffix(const KeyType& key)
{
  return "_" + to_string(key.first) + "." + to_string(key.second);
}

string CalibObjManager::getSuffix(const EventMetaData& emd)
{
  KeyType key = make_pair(emd.getExperiment(), emd.getRun());
  return getSuffix(key);
}
