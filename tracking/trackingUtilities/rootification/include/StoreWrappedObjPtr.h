/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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


