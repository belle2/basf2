#pragma once

#include <string>
#include <memory>

#include <TDirectory.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>

#include <calibration/Utilities.h>
#include <calibration/dataobjects/CalibRootObjBase.h>
#include <calibration/dataobjects/CalibRootObj.h>

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
      * is never used directly by an outside user except when creating object that is passed in for the template.
      */
    virtual ~CalibObjManager() {}

    /// Change the directory that we will be using to find/store all our objects, we don't own it
    void setDirectory(TDirectory* dir) {m_dir = dir;}

    /** Add a new object to manage, this is used as a template for creating future/missing objects.
      * We take ownership of this object but cannot guarantee that a user won't alter the wrapped ROOT object :(
      */
    void addObject(std::string name, std::unique_ptr<CalibRootObjBase> object);

    void writeCurrentObjects(const Calibration::ExpRun& expRun);

    void clearCurrentObjects(const Calibration::ExpRun& expRun);
    /** Each object gets its own TDirectory under the main manager directory to store its objects.
      * We create them using this function.
      */
    void createDirectories();

    template<typename T>
    T* getObject(std::string name, Belle2::Calibration::KeyType expRun)
    {
      TDirectory* objectDir = m_dir->GetDirectory(name.c_str());
      std::string fullName = name + getSuffix(expRun);
      T* obj;
      objectDir->GetObject(fullName.c_str(), obj);

      if (!obj) {
        B2DEBUG(100, "No object with the name " << fullName << " in " << objectDir->GetPath() << ". Will make one.");
        obj = dynamic_cast<T*>(m_templateObjects[name]->cloneHeldObj(fullName));
        obj->SetDirectory(objectDir);
        obj->Reset();
        // Did SetDirectory work? Or was it a dummy class method?
        T* objTest;
        objectDir->GetObject(fullName.c_str(), objTest);
        if (!objTest) {
          B2DEBUG(100, "SetDirectory was a dummy. Adding to Object to TDirectory manually.");
          objectDir->Add(obj);
        }
      }
      return obj;
    }

  private:

    /// The TDirectory where all of our managed objects should be found, and where we should create new ones
    TDirectory* m_dir;

    /**
     * The objects that we are managing, these are template objects for all future objects for each (Exp,Run).
     * They should not contain data themselves.
     */
    std::map<std::string, std::unique_ptr<CalibRootObjBase>> m_templateObjects;

    /// We rename objects based on the Exp,Run that they contain so we need to generate a nice naming convention
    std::string getSuffix(const Belle2::Calibration::KeyType& key);

    /// Sometimes it's nice to just pass in the EventMetaData instead of converting manually
    std::string getSuffix(const EventMetaData& emd);
  };
}
