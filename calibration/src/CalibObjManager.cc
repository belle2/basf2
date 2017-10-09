#include <calibration/CalibObjManager.h>

#include <string>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <TKey.h>

using namespace std;
using namespace Belle2;
using namespace Calibration;

const string CalibObjManager::TMPPREFIX = "TMPCALIB_";

string CalibObjManager::addPrefix(const string& name) const
{
  return TMPPREFIX + name;
}

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
  TDirectory* oldDir = gDirectory;
  for (auto& x : m_templateObjects) {
    TDirectory* objectDir = m_dir->GetDirectory((x.first + '/' + getObjectExpRunName(x.first, expRun)).c_str());
    objectDir->cd();
    string prefixedName = addPrefix(x.first);
    TNamed* objMemory = dynamic_cast<TNamed*>(objectDir->FindObject(prefixedName.c_str()));
    if (objMemory) {
      // We have found a memory resident object for this exp,run and want to write it
      // But is there already an object written from this same exp,run being evaluated previously?
      // Loop over the keys to find any previous objects and the highest previous index.
      unsigned int index = 0;
      for (auto key : * (gDirectory->GetListOfKeys())) {
        string keyName = key->GetName();
        B2DEBUG(100, "Found previous Key " << keyName << " in the directory " << gDirectory->GetPath());
        unsigned int currentIndex = extractKeyIndex(keyName);
        if (currentIndex > index) {
          index = currentIndex;
        }
      }
      B2DEBUG(100, "Highest previous index for object " << x.first
              << " for Exp.Run = " << encodeExpRun(expRun)
              << " was " << index);
      objMemory->SetName((x.first + "_" + to_string(index + 1)).c_str());
      B2DEBUG(100, "We are writing the object " << objMemory->GetName() << " to its TDirectory.");
      gDirectory->WriteTObject(objMemory, objMemory->GetName(), "Overwrite");
    }
  }
  oldDir->cd();
}

void CalibObjManager::clearCurrentObjects(const ExpRun& expRun)
{
  for (auto& x : m_templateObjects) {
    TDirectory* objectDir = m_dir->GetDirectory((x.first + '/' + getObjectExpRunName(x.first, expRun)).c_str());
    B2DEBUG(100, "We are deleting all the in-memory objects " << objectDir->GetPath());
    // Should only delete objects that are memory resident
    objectDir->DeleteAll();
  }
}

string CalibObjManager::getSuffix(const ExpRun& expRun) const
{
  return "_" + encodeExpRun(expRun);
}

string CalibObjManager::getSuffix(const EventMetaData& emd) const
{
  ExpRun key = make_pair(emd.getExperiment(), emd.getRun());
  return getSuffix(key);
}

string CalibObjManager::getObjectExpRunName(const string& name, const ExpRun& expRun) const
{
  return name + getSuffix(expRun);
}

unsigned int CalibObjManager::extractKeyIndex(string& keyName) const
{
  vector<string> strs;
  boost::split(strs, keyName, boost::is_any_of("_"));
  string indexString = strs.back();
  return stoi(indexString);
}
