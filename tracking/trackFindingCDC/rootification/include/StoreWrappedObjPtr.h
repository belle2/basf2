/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/rootification/StoreWrapper.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /** This class is for convenience access and registration of objects,
     *  that are stored inside the StoreWrapper on the DataStore.
     *
     *  It limits the user to registrations of transient store objects only,
     *  since objects wrapped by the StoreWrapper are usually not intended
     *  for streaming.
     */
    template<class T>
    class  StoreWrappedObjPtr : public StoreObjPtr<StoreWrapper<T> > {

    public:

#if 0 //use of static StoreObjPtr members is discouraged.
      /** Register an object, that should not be written to the output by default, in the data store.
       *
       *  This must be called in the initialization phase.
       *
       *  @param name        Name under which the object is stored.
       *  @param durability  Specifies lifetime of object in question.
       *  @param errorIfExisting  Flag whether an error will be reported if the object was already registered.
       *  @return            True if the registration succeeded.
       */
      static bool registerTransient(const std::string& name = "",
                                    DataStore::EDurability durability = DataStore::c_Event,
                                    bool errorIfExisting = false)
      {
        return StoreObjPtr<StoreWrapper<T> >::registerTransient(name, durability, errorIfExisting);
      }

      /** Check whether an object was registered before.
       *
       *  It will cause an error if the object does not exist.
       *  This must be called in the initialization phase.
       *
       *  @param name        Name under which the object is stored.
       *  @param durability  Specifies lifetime of object in question.
       *  @return            True if the object exists.
       */
      static bool required(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event)
      {
        return StoreObjPtr<StoreWrapper<T> >::required(name, durability);
      }

      /** Tell the data store about an optional input.
       *
       *  Mainly useful for creating diagrams of module inputs and outputs.
       *  This must be called in the initialization phase.
       *
       *  @param name        Name under which the object is stored.
       *  @param durability  Specifies lifetime of object in question.
       *  @return            True if the object exists.
       */
      static bool optional(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event)
      {
        return StoreObjPtr<StoreWrapper<T> >::optional(name, durability);
      }
#endif

    public:
      /** Constructor to access an object in the DataStore.
       *
       *  @param name       Name under which the object is stored in the DataStore.
       *                    If an empty string is supplied, the type name will be used.
       *  @param durability Decides durability map used for getting the accessed object.
       */
      StoreWrappedObjPtr(const std::string& name = "", DataStore::EDurability durability = DataStore::c_Event) :
        StoreObjPtr<StoreWrapper<T> >(name, durability)
      {}


      /** Register the object/array in the DataStore.
       *  This must be called in the initialization phase.
       *
       *  @param storeFlags ORed combination of DataStore::EStoreFlag flags. Defaults to c_DontWriteOut | c_ErrorIfAlreadyRegistered.
       *  @return            True if the registration succeeded.
       */
      bool registerInDataStore(DataStore::EStoreFlags storeFlags = DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered)
      {
        return StoreObjPtr<StoreWrapper<T> >::registerInDataStore(storeFlags);
      }

      /** Register the object/array in the DataStore.
       *  This must be called in the initialization phase.
       *
       *  @param name  If not empty, set non-default name for this object/array. This is permanent, so that e.g. after using registerInDataStore("myName") in initialize(), this object will continue refer to 'myName' in event().
       *  @param storeFlags ORed combination of DataStore::EStoreFlag flags. Defaults to c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered.
       *  @return            True if the registration succeeded.
       */
      bool registerInDataStore(const std::string& name,
                               DataStore::EStoreFlags storeFlags = DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered)
      {
        return StoreObjPtr<StoreWrapper<T> >::registerInDataStore(name, storeFlags);
      }

      /// Dereference to the wrapped object.
      T& operator*() const
      {
        return StoreObjPtr<StoreWrapper<T> >::operator*().get();
      }

      /// Forward pointer access to the wrapped object.
      T* operator->() const
      {
        return  &(operator*());
      }

    };
  }
}


