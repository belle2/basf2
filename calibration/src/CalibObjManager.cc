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

void CalibObjManager::writeCurrentObjects(const ExpRun& expRun)
{
  for (auto& x : m_templateObjects) {
    string calObjName = x.first + getSuffix(expRun);
    B2DEBUG(100, "We are writing the current Exp,Run object for " << calObjName << " to its TDirectory.");
    TDirectory* objDir = m_dir->GetDirectory(x.first.c_str());
    TNamed* objMemory = dynamic_cast<TNamed*>(objDir->FindObject(calObjName.c_str()));
    objDir->WriteTObject(objMemory, objMemory->GetName(), "Overwrite");
  }
}

void CalibObjManager::clearCurrentObjects(const ExpRun& expRun)
{
  for (auto& x : m_templateObjects) {
    B2DEBUG(100, "We are deleting all the in-memory objects " << x.first);
    TDirectory* objectDir = m_dir->GetDirectory(x.first.c_str());
    // Should only delete objects that are memory resident
    objectDir->DeleteAll();
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
