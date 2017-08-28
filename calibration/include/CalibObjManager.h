#pragma once

#include <string>
#include <memory>
#include <TDirectory.h>

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

    /** Change the object that we're managing, this is used as a template for creating future/missing objects.
      * We take ownership of this object, should be guaranteed by using unique_ptr.
      */
    void addObject(std::string name, std::unique_ptr<CalibRootObjBase> object)
    {
      if (m_templateObjects.find(name) != m_templateObjects.end()) {
        m_templateObjects[name].reset();
      }
      m_templateObjects[name] = std::move(object);
    }

    CalibRootObjBase* getObject(std::string name, Belle2::Calibration::KeyType expRun)
    {
      std::string fullName = name + "_" + std::to_string(expRun.first) + std::to_string(expRun.second);
      CalibRootObjBase* calObj = dynamic_cast<CalibRootObjBase*>(m_dir->FindObject(fullName.c_str()));
      if (!calObj) {
        calObj = dynamic_cast<CalibRootObjBase*>(m_templateObjects[name]->Clone(fullName.c_str()));
        calObj->setIOV(expRun);
        m_dir->Add(calObj);
      }
      return calObj;
    }

  private:

    /// The TDirectory where all of our managed objects should be found, and where we should create new ones
    TDirectory* m_dir;

    /**
     * The object type that we are managing, this is a template object for all future objects for each (Exp,Run).
     * This should not contain data itself.
     */
    std::map<std::string, std::unique_ptr<CalibRootObjBase>> m_templateObjects;
  };
}
