#include <calibration/CalibObjManager.h>

#include <string>

using namespace Belle2;

const std::string CalibObjManager::PREFIX = "TmpCalibRootObj_";

void CalibObjManager::addObject(std::string name, std::unique_ptr<CalibRootObjBase> object)
{
  if (m_templateObjects.find(name) != m_templateObjects.end()) {
    m_templateObjects[name].reset();
  }
  m_templateObjects[name] = std::move(object);
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

void CalibObjManager::writeCurrentObjects(const EventMetaData& emd)
{
  for (auto& x : m_templateObjects) {
    std::string calObjName = addPrefix(x.first) + getSuffix(emd);
    B2DEBUG(100, "We are writing the current Exp,Run object for " << calObjName << " to its TDirectory.");
    TDirectory* objDir = m_dir->GetDirectory(x.first.c_str());
    CalibRootObjBase* objMemory = dynamic_cast<CalibRootObjBase*>(objDir->FindObject(calObjName.c_str()));
    // An unfortunate issue is that there can be only one memory resident object in a directory with the same Key/Name.
    // Therefore when we dir->Get("name;1") the file resident one, it has the same Key as the memory resident object which
    // then causes the deletion of the object. To prevent this we do a quick tmp naming while checking for file resident objects,
    // and rename back at the end.
    objMemory->SetName("tmpCalibrationCollector");
    CalibRootObjBase* objFile = dynamic_cast<CalibRootObjBase*>(objDir->Get((calObjName + ";1").c_str()));
    if (objMemory && objFile) {
      B2DEBUG(100, "Found both memory AND file resident copies of the same object with the name " << calObjName
              << ". Merging memory resident with file resident before writing.");
      TList objList;
      objList.SetOwner(false);
      objList.Add(objFile);
      objMemory->Merge(&objList);
    }
    objMemory->SetName(calObjName.c_str());
    objDir->WriteTObject(objMemory, objMemory->GetName(), "Overwrite");
  }
}

void CalibObjManager::clearCurrentObjects(const EventMetaData& emd)
{
  for (auto& x : m_templateObjects) {
    std::string calObjName = addPrefix(x.first) + getSuffix(emd);
    B2DEBUG(100, "We are deleting the current Exp,Run object for " << calObjName);
    TDirectory* objectDir = m_dir->GetDirectory(x.first.c_str());
    // Should only delete objects that are memory resident
    objectDir->Delete(calObjName.c_str());
  }
}

void CalibObjManager::replaceObjects()
{
  for (auto& x : m_templateObjects) {
    TDirectory* objectDir = m_dir->GetDirectory(x.first.c_str());
    objectDir->cd();
    TKey* key = 0;
    TIter nextkey(objectDir->GetListOfKeys());
    while ((key = (TKey*)nextkey())) {
      // Get the disk resident object
      CalibRootObjBase* objFile = dynamic_cast<CalibRootObjBase*>(key->ReadObj());
      if (objFile) {
        B2DEBUG(100, "Found file resident copy of the object with the name " << key->GetName()
                << ". Replacing it with the wrapped TObject.");
        objFile->setObjectName(removePrefix(key->GetName()).c_str());
        objFile->write(objectDir);
        std::string calObjName = objFile->GetName();
        objectDir->Delete((calObjName + ";*").c_str());
      }
    }
    objectDir->DeleteAll();
    m_dir->cd();
  }
}

std::string CalibObjManager::getSuffix(const Belle2::Calibration::KeyType& key)
{
  return "_" + std::to_string(key.first) + "." + std::to_string(key.second);
}

std::string CalibObjManager::getSuffix(const EventMetaData& emd)
{
  Belle2::Calibration::KeyType key = std::make_pair(emd.getExperiment(), emd.getRun());
  return getSuffix(key);
}

std::string CalibObjManager::addPrefix(std::string name)
{
  return PREFIX + name;
}

std::string CalibObjManager::removePrefix(const char* name)
{
  std::string strname = name;
  strname.erase(0, PREFIX.length());
  return strname;
}
