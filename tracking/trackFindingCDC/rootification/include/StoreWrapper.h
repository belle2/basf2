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
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /** This template functions as a wrapper for objects
     *  that do not inherit from TObject to be put on the DataStore.
     *
     *  It there for overcomes the limitation, that objects on the DataStore
     *  must have TObject at the bottom of the inheritance chain, which would
     *  be to invasive and costly in terms of memory consumption an creation overhead.
     *  Objects wrapped like this cannot be written to output.
     */
    template<class T>
    class StoreWrapper : public RelationsObject {

    public:
      /// Constant getter for the the contained object.
      const T& get() const
      { return m_item; }

      /// Getter for the the contained object.
      T& get()
      { return m_item; }

      /// Alias getter for the the contained object to stress that the wrapper is being removed.
      T& unwrap()
      { return m_item; }

      /// Forwarding -> access to the wrapped object as if the wrapper would be a pointer to it.
      T* operator->() { return &m_item; }

      /// Forwarding -> access to the wrapped object as if the wrapper would be a pointer to it.
      const T* operator->() const { return &m_item; }

      /// Unpacks the wrapped object as if the wrapper was a pointer to it.
      T& operator*() { return m_item; }

      /// Unpacks the wrapped object as if the wrapper was a pointer to it.
      const T& operator*() const { return m_item; }

      /// Forwards an iteration request from python to the wrapped object. If iteration is feasable depends on the wrapped object.
      const T& __iter__() const { return m_item; }

    public:
      /// Memory for the wrapped item.
      T m_item;

    private:
      /// ROOT Macro to unconditionally make StoreWrapper a ROOT class.
      ClassDef(StoreWrapper, 1);
    };

  }

}


