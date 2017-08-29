#pragma once

#include <string>
#include <memory>
#include <TDirectory.h>
#include <TList.h>

#include <calibration/Utilities.h>
#include <calibration/dataobjects/CalibRootObjBase.h>

namespace Belle2 {

  class CalibObjManager {

  public:
    /// Constructor
    CalibObjManager(TDirectory* dir = nullptr) : m_dir(dir) {};

    /** Destructor: Every object we are managing is ultimately either saved into a file to write out,
      * or was read in from an open file, or was created via merging objects together and passed out.
      * Ultimately the objects that are saved or read from a file are memory managed by a TFile/TDirectory.
      * The created/merged objects are returned and are the responsibility of the user to clean up once finished with them.
      * So we are only really responsible for cleaning up the template object which belongs to no directory and
      * is never used directly by an outside user.
      */
    virtual ~CalibObjManager() {}

    /// Change the directory that we will be using to find/store all our objects, we don't own it
    void setDirectory(TDirectory* dir) {m_dir = dir;}

    /** Add a new object to manage, this is used as a template for creating future/missing objects.
      * We take ownership of this object, should be guaranteed by using unique_ptr.
      */
    void addObject(std::string name, std::unique_ptr<CalibRootObjBase> object)
    {
      if (m_templateObjects.find(name) != m_templateObjects.end()) {
        m_templateObjects[name].reset();
      }
      m_templateObjects[name] = std::move(object);
    }

    void createDirectories()
    {
      for (auto& x : m_templateObjects) {
        if (m_dir->GetDirectory(x.first.c_str()) == 0) {
          m_dir->mkdir(x.first.c_str());
          B2DEBUG(100, "Made TDirectory: " << x.first);
        }
      }
    }

    CalibRootObjBase* getObject(std::string name, Belle2::Calibration::KeyType expRun)
    {
      TDirectory* objectDir = m_dir->GetDirectory(name.c_str());
      std::string fullName = name + getSuffix(expRun);
      // Try looking in the memory resident objects only first
      CalibRootObjBase* calObj = dynamic_cast<CalibRootObjBase*>(objectDir->FindObject(fullName.c_str()));
      if (!calObj) {
        // If there aren't any then either there isn't an object at all, or the only objects are file resident
        // We'll create a new memory resident object to use now, and merge with any file resident ones later.
        B2DEBUG(100, "No memory resident copies of the object with the name " << fullName << ". Will make one.");
        calObj = dynamic_cast<CalibRootObjBase*>(m_templateObjects[name]->Clone(fullName.c_str()));
        calObj->setIOV(expRun);
        objectDir->Add(calObj);
      }
      return calObj;
    }

//        TKey* calObjKey = objectDir->FindKey(fullName.c_str());
//        if(calObjKey) {
//          // If there was a file resident one, read it into a memory resident object
//          B2DEBUG(100, "Only file resisdent object exists for object with the name " << fullName << ". Making a memory resident version.");
//          calObj = dynamic_cast<CalibRootObjBase*>(calObjKey->ReadObj());
//        }
//        else {

    void writeCurrentObjects(const EventMetaData& emd)
    {
      for (auto& x : m_templateObjects) {
        B2DEBUG(100, "We are writing the current Exp,Run object for " << x.first + getSuffix(emd) << " to its TDirectory.");
        TDirectory* objectDir = m_dir->GetDirectory(x.first.c_str());
        CalibRootObjBase* objMemory = dynamic_cast<CalibRootObjBase*>(objectDir->FindObject((x.first + getSuffix(emd)).c_str()));
        // An unfortunate issue is that there can be only one memory resident object in a directory with the same Key/Name.
        // Therefore when we dir->Get("name;1") the file resident one, it has the same Key as the memory resident object which
        // then causes the deletion of the object. To prevent this we do a quick tmp naming while checking for file resident objects,
        // and rename back at the end.
        objMemory->SetName("tmpCalibrationCollector");
        CalibRootObjBase* objFile = dynamic_cast<CalibRootObjBase*>(objectDir->Get((x.first + getSuffix(emd) + ";1").c_str()));
        if (objMemory && objFile) {
          B2DEBUG(100, "Found both memory AND file resident copies of the same object with the name " << x.first
                  << ". Merging memory resident with file resident before writing.");
          TList* listObj = new TList();
          listObj->Add(objFile);
          objMemory->Merge(listObj);
          delete listObj;
        }
        objMemory->SetName((x.first + getSuffix(emd)).c_str());
        objectDir->WriteTObject(objMemory, objMemory->GetName(), "Overwrite");
      }
    }

    void clearCurrentObjects(const EventMetaData& emd)
    {
      for (auto& x : m_templateObjects) {
        B2DEBUG(100, "We are deleting the current Exp,Run object for " << x.first + getSuffix(emd));
        TDirectory* objectDir = m_dir->GetDirectory(x.first.c_str());
        // Should only delete objects that are memory resident
        objectDir->Delete((x.first + getSuffix(emd)).c_str());
      }
    }

  private:

    /// The TDirectory where all of our managed objects should be found, and where we should create new ones
    TDirectory* m_dir;

    /**
     * The object type that we are managing, this is a template object for all future objects for each (Exp,Run).
     * This should not contain data itself.
     */
    std::map<std::string, std::unique_ptr<CalibRootObjBase>> m_templateObjects;

    /// We rename objects based on the Exp,Run that they contain so we need to generate a nice naming convention
    std::string getSuffix(const Belle2::Calibration::KeyType& key)
    {
      return "_" + std::to_string(key.first) + "." + std::to_string(key.second);
    }

    std::string getSuffix(const EventMetaData& emd)
    {
      Belle2::Calibration::KeyType key = std::make_pair(emd.getExperiment(), emd.getRun());
      return getSuffix(key);
    }
  };
}
