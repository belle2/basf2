#pragma once
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/pcore/Mergeable.h>

namespace Belle2 {

  /** Wrap an STL set to make it mergeable.
   *
   * Should work out-of-the-box with std::set and 'similar' containers.
   *
   * To use it to save data in your module:
   * \code
     setPropertyFlags(c_Parallelprocessing | c_terminateInAllProcesses);
     \endcode
     create SetMergeable<X> in initalize (or in your constructor) of durability DataStore::c_Persistent,
     register it by calling registerInDataStore() and construct() the actual container.
   *
   * Each template instance needs its own entry in framework/pcore/include/linkdef.h,
   * please contact the framework librarian if your use case requires other classes than those in there.
   *
   * \sa Mergeable
   */
  template <class T> class SetMergeable : public Mergeable {
  public:
    SetMergeable() : m_wrapped() {  }

    virtual ~SetMergeable() { }
    /** Constructor, forwards all arguments to T constructor. */
    template<class ...Args> explicit SetMergeable(Args&& ... params) : m_wrapped(std::forward<Args>(params)...) { }

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
    virtual void merge(const Mergeable* other) override
    {
      auto* otherMergeable = static_cast<const SetMergeable*>(other);
      for (const auto& element : otherMergeable->get()) {
        auto it = m_wrapped.find(element);
        if (it == m_wrapped.end()) {
          m_wrapped.insert(element);
        }
      }
    }

    /** Clear content of this object (e.g. set to zeroes).
     *
     * Called after sending the objects to another process. If no clearing is performed, the same data (e.g. histogram
     * entries) might be added again and again in each event.
     */
    virtual void clear() override
    {
      m_wrapped.clear();
    }

  private:
    /** Wrapped object. */
    T m_wrapped;

    ClassDefOverride(SetMergeable, 1); /**< Wrap an STL set to make it mergeable. */
  };
}
