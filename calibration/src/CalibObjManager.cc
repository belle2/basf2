#include <calibration/CalibObjManager.h>

#include <string>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <TKey.h>
#include <TTree.h>

using namespace std;
using namespace Belle2;
using namespace Calibration;

namespace Belle2 {
  template<>
  TTree* CalibObjManager::cloneObj(TTree* source, std::string newName) const
  {
    B2DEBUG(100, "Held object is a TTree which will be have CloneTree() called.");
    // Construct the TTree by making a copy
    // of the source TTree
    TTree* dest = source->CloneTree(0);
    dest->SetName(newName.c_str());
    return dest;
  }

  void CalibObjManager::deleteHeldObjects()
  {
    m_templateObjects.clear();
  }

  void CalibObjManager::addObject(string name, shared_ptr<TNamed> object)
  {
    if (m_templateObjects.find(name) != m_templateObjects.end()) {
      m_templateObjects[name].reset();
    }
    m_templateObjects[name] = object;
  }

  void CalibObjManager::createDirectories()
  {
    for (auto& x : m_templateObjects) {
      if (m_dir->GetDirectory(x.first.c_str()) == 0) {
        m_dir->mkdir(x.first.c_str());
        TDirectory* objectDir = m_dir->GetDirectory(x.first.c_str());
        objectDir->SetWritable(true);
        B2DEBUG(100, "Made TDirectory: " << x.first);
      }
    }
  }

  void CalibObjManager::createExpRunDirectories(ExpRun& expRun) const
  {
    for (auto& x : m_templateObjects) {
      TDirectory* objectDir = m_dir->GetDirectory(x.first.c_str());
      string dirName = x.first + getSuffix(expRun);
      TDirectory* newDir = objectDir->GetDirectory(dirName.c_str());
      if (!newDir) {
        newDir = objectDir->mkdir(dirName.c_str());
        newDir->SetWritable(true);
        B2DEBUG(100, "Made TDirectory " << newDir->GetPath());
      }
    }
  }

  void CalibObjManager::writeCurrentObjects(const ExpRun& expRun)
  {
    for (auto& x : m_templateObjects) {
      TDirectory* objectDir = m_dir->GetDirectory((x.first + '/' + getObjectExpRunName(x.first, expRun)).c_str());
      B2DEBUG(100, "Writing for " << x.first);
      for (auto key : * (objectDir->GetList())) {
        B2DEBUG(100, "Writing for " << key->GetName());
        TNamed* objMemory = dynamic_cast<TNamed*>(objectDir->FindObject(key->GetName()));
        if (objMemory) {
          objectDir->WriteTObject(objMemory, key->GetName(), "Overwrite");
        }
      }
    }
  }

  void CalibObjManager::clearCurrentObjects(const ExpRun& expRun)
  {
    for (auto& x : m_templateObjects) {
      TDirectory* objectDir = m_dir->GetDirectory((x.first + '/' + getObjectExpRunName(x.first, expRun)).c_str());
      B2DEBUG(100, "We are deleting all the in-memory + file objects " << objectDir->GetPath());
      objectDir->DeleteAll();
    }
  }

  unsigned int CalibObjManager::getHighestIndexObject(const string name, const TDirectory* dir) const
  {
    unsigned int index = 0;
    // Try from the list of objects
    for (auto key : * (dir->GetList())) {
      string keyName = key->GetName();
      if (keyName.find(name) != std::string::npos) {
        B2DEBUG(1000, "Found previous Object " << keyName << " in the directory " << dir->GetPath());
        unsigned int currentIndex = extractKeyIndex(keyName);
        if (currentIndex > index) {
          index = currentIndex;
        }
      }
    }
    // Try from List of keys
    for (auto key : * (dir->GetListOfKeys())) {
      string keyName = key->GetName();
      if (keyName.find(name) != std::string::npos) {
        B2DEBUG(1000, "Found previous Key " << keyName << " in the directory " << dir->GetPath());
        unsigned int currentIndex = extractKeyIndex(keyName);
        if (currentIndex > index) {
          index = currentIndex;
        }
      }
    }
    B2DEBUG(1000, "Returning highest index " << index);
    return index;
  }

  string CalibObjManager::getSuffix(const ExpRun& expRun) const
  {
    return "_" + encodeExpRun(expRun);
  }

  string CalibObjManager::getSuffix(const EventMetaData& emd) const
  {
    const ExpRun key = make_pair(emd.getExperiment(), emd.getRun());
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
}
