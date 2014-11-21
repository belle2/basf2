/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SWITCHABLEROOTIFICATIONBASE_H_
#define SWITCHABLEROOTIFICATIONBASE_H_

#include "RootificationBase.h"

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/pybasf2/PyStoreArray.h>

#include <tracking/cdcLocalTracking/config/CDCLocalTrackingConfig.h>

#include <TObject.h>
#include <TClass.h>

namespace Belle2 {

  namespace CDCLocalTracking {

    // BASF2 defines
    // RelationsObject
    // ClassDef(ClassName, ClassVersion);
    // ClassImp(ClassName);
    // to be used for a full blown rootification.
    // Using theses as base class and macro in a user object
    // enables access from Python as well as access through the DataStore

    // In the scons build system you can activate the additional compile time macro using
    // scons --extra-ccflags='-DCDCLOCALTRACKING_USE_ROOT'.
    // Forgeting to give the flag should result in the correct
    // default behaviour, which is to build without ROOT support. This avoids that one
    // accidentally checks in a version with ROOT switch on.


#ifdef CDCLOCALTRACKING_USE_ROOT
    /// Typedef the normal TObject as the base class of the track finder in case the ROOT inheritance is switched on.
    typedef RootificationBase SwitchableRootificationBase;

#define CDCLOCALTRACKING_SwitchableClassDef(CLASSNAME, CLASSVERSION)  \
  ClassDef(CLASSNAME, CLASSVERSION);              \
public:                   \
  /** Register a *transient* store array on the DataStore.*/        \
  static void registerStoreArray()              \
  { StoreArray< CLASSNAME >::registerTransient(); }         \
  \
  /** Clear the StoreArray to an empty version. */          \
  static void clearStoreArray(){              \
    StoreArray< CLASSNAME > storeArray;             \
    const bool replace = true;                \
    storeArray.create(replace);               \
  }                     \
  \
private:                    \
  /** Get the default name of the StoreArray for this class.*/        \
  static std::string getStoreArrayName()            \
  { return DataStore::defaultArrayName< CLASSNAME >(); }        \
  \
public:                   \
  /** Looks up the StoreArray and returns it as a PyStoreArray. */    \
  static PyStoreArray getStoreArray()             \
  { return PyStoreArray(getStoreArrayName()); }           \
  \
  /** Copies the object to the StoreArray. */           \
  /** Returns a reference to the newly created object*/         \
  CLASSNAME * copyToStoreArray() const {            \
    StoreArray< CLASSNAME > storeArray;             \
    return storeArray.appendNew(*this);             \
  }                     \
  \
  /** Append new default constructed object to the StoreArray. */     \
  static CLASSNAME * appendNewToStoreArray() {            \
    StoreArray< CLASSNAME > storeArray;             \
    return storeArray.appendNew();              \
  }                     \
  \
public:                   \
  /** Register a *transient* RelationArray from this class */       \
  /** to the this class on the DataStore */           \
  static void registerInnerRelation()             \
  { registerRelationTo(getStoreArrayName()); }            \
  \
  /** Register a *transient* RelationArray from this class to the class refered to by its plural array name on the DataStore */ \
  static void registerRelationTo(const std::string& toArrayName){ \
    StoreArray< CLASSNAME > from;         \
    StoreAccessorBase to(toArrayName, DataStore::c_Event, TObject::Class(), true); \
    DataStore::Instance().registerRelation(from, to, DataStore::c_Event, DataStore::c_DontWriteOut); \
  }                 \
private:                \
   
#define CDCLOCALTRACKING_SwitchableClassImp(CLASSNAME) ClassImp(CLASSNAME);

#else
    // Do not use the ROOT inheritance in all Tracking objects
    // Use an empty base class instead
    /// Empty base class for the switched of ROOT inheritance
    class SwitchableRootificationBaseificationBase { ; };

    /// Typedef the empty class as the base class of the track finder in case the ROOT inheritance is switched off.
    typedef  SwitchableRootificationBaseificationBase SwitchableRootificationBase;

#define CDCLOCALTRACKING_SwitchableClassDef(CLASSNAME,CLASSVERSION)
#define CDCLOCALTRACKING_SwitchableClassImp(CLASSNAME)

#endif


  } //end namespace CDCLocalTracking

} //end namespace Belle2


#endif // SWITCHABLEROOTIFICATIONBASE_H_
