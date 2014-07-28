#pragma once
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/pcore/Mergeable.h>
#include <framework/logging/Logger.h>

#include <map>

namespace Belle2 {
  /** Wrap a standard map to make them mergeable.
   *
   * To use it to save data in your module:
   * \code
     setPropertyFlags(c_Parallelprocessing | c_terminateInAllProcesses);
     \endcode
     create MapMergeable<X> in initalize (call register(As)Transient("", DataStore::c_Persistent) and construct())
   *
   * This should work out of the box for std::map. Additional template instantiations need
   * an entry in framework/pcore/include/linkdef.h, please contact the framework librarian if your use
   * case requires other classes.
   *
   * \sa Mergeable
   */
  template <class T>
  class MapMergeable : public Mergeable {
  public:
    MapMergeable() : m_wrapped(nullptr) {  }

    virtual ~MapMergeable() { delete m_wrapped; }
#if defined(__CINT__) || defined(__ROOTCLING__) || defined(R__DICTIONARY_FILENAME)
#else
    /** Constructor, forwards all arguments to T constructor. */
    template<class ...Args> MapMergeable(Args&& ... params) : m_wrapped(new T(std::forward<Args>(params)...)) { }
#endif

    /** Get the wrapped standard object. */
    T& get() { return m_wrapped; }

    /** Get the wrapped standard object. */
    const T& get() const { return m_wrapped; }

    /** Merge object 'other' into this one.
     *
     * Your derived class should implement this function. You can static_cast 'other' to your own type
     * (when called, this and other are guaranteed to point to objects of the same type).
     *
     * Note that 'other' will be deleted after the merge, so make sure you copy all data from it that you will need.
     */
    virtual void merge(const Mergeable* other) {
      auto* otherMergeable = static_cast<const MapMergeable*>(other);
      for (const auto & element : otherMergeable) {
        auto it = m_wrapped.find(element);
        if (it != m_wrapped.end()) {
          it->second += element.second;
        } else {
          m_wrapped.insert(element);
        }
      }
    }

    /** Clear content of this object (e.g. set to zeroes).
     *
     * Called after sending the objects to another process. If no clearing is performed, the same data (e.g. histogram
     * entries) might be added again and again in each event.
     */
    virtual void clear() {
      m_wrapped.clear();
    }

  private:
    /** Wrapped root object. */
    T m_wrapped;

    ClassDef(MapMergeable, 1); /**< Wrap a standard object to make them mergeable. */
  };
}
