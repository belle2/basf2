#pragma once

#include <string>
#include <memory>
#include <TDirectory.h>
#include <TList.h>
#include <TKey.h>

#include <calibration/Utilities.h>
#include <calibration/dataobjects/CalibRootObjBase.h>
#include <calibration/dataobjects/CalibRootObjNew.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <calibration/Utilities.h>

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
      * We take ownership of this object but cannot guarantee that a user won't alter the wrapped ROOT object :(
      */
    void addObject(std::string name, std::unique_ptr<CalibRootObjBase> object);

    /** Each object gets its own TDirectory under the main manager directory to store its objects.
      * We create them using this function.
      */
    void createDirectories();

    /** Writes the current (exp,run) collector objects to the requested directories.
      * We write the CalibRootObj<T> for now but will replace this in the terminate().
      * If a file resident object of the same name already exists it means that we've
      * run over several input files containing the same (exp,run). We merge
      * with the file resident copy and replace it.
      */
    void writeCurrentObjects(const Calibration::ExpRun& emd);

    /// Any memory resident CalibRootObj are removed from the directory
    void clearCurrentObjects(const Calibration::ExpRun& emd);

    /**
      * This is a weird one. The problem is that we want to store the 'raw' ROOT objects in our final output data.
      * This is because TChain will not work with non-TTree objects, so to avoid potentially putting massive TTree objects into
      * memory during the Algorithm step we have to store the TTrees (and by extension everything else) 'raw'.
      * However up until now we've been writing the 'wrapped' CalibRootObj into the TDirectory. This sounds silly until you realise
      * that we have to be able to both write AND retrieve them from disk and then potentially call Merge on the retrieved objects.
      * Therefore we have to able to cast to an object that knows how to call Merge i.e. our CalibRootObj.
      *
      * The basic idea is to store CalibRootObj objects until we have finished doing ANY merging i.e. during terminate()
      * Then we retrieve the objects one by one from the TDirectory and replace them with the raw object they contain.
      */
    void replaceObjects();

    /** Return the requested collector object for this (exp,run) by gettin or creating one.
      * We wrap the object in a CalibRootObj and add it the direcotry so that the object
      * can take advantage of any improvements we want to add to that class, and so that a
      * Merge method is guaranteed to exist for our purposes later.
      */
    template<typename T>
    T* getObject(std::string name, Belle2::Calibration::KeyType expRun)
    {
      TDirectory* objectDir = m_dir->GetDirectory(name.c_str());
      std::string fullName = addPrefix(name) + getSuffix(expRun);
      // Try looking in the memory resident objects only first
      CalibRootObjNew<T>* calObj = dynamic_cast<CalibRootObjNew<T>*>(objectDir->FindObject(fullName.c_str()));
      if (!calObj) {
        // If there aren't any then either there isn't an object at all, or the only objects are file resident
        // We'll create a new memory resident object to use for now since any file resident TTree will have lost
        // its connection to the branch variables the user defined, We'll merge this new object with any file resident
        // object later.
        B2DEBUG(100, "No memory resident copies of the object with the name " << fullName << ". Will make one.");
        calObj = dynamic_cast<CalibRootObjNew<T>*>(m_templateObjects[name]->Clone(fullName.c_str()));
        objectDir->Add(calObj);
      }
      return calObj->getObject();
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

    /// Our temporary CalibRootObj objects need names which won't conflict with the raw objects
    std::string addPrefix(std::string name);

    /// Sometimes when retrieving CalibRootObj objects from a TDirectory we want to remove a previous prefix
    std::string removePrefix(const char* name);

    const static std::string PREFIX;
  };
}
