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
#include <RooDataSet.h>

namespace Belle2 {
  /** Wrap an RooDataSet to make it mergeable.
   *
   * To use it to save data in your module:
   * \code
     setPropertyFlags(c_Parallelprocessing | c_terminateInAllProcesses);
     \endcode
     create RooDataSetMergeable in initalize (or in your constructor) of durability DataStore::c_Persistent,
     register it by calling registerInDataStore() and construct() the actual container.
   *
   * Each template instance needs its own entry in framework/pcore/include/linkdef.h,
   * please contact the framework librarian if your use case requires other classes than those in there.
   *
   * \sa Mergeable
   */
  class RooDataSetMergeable : public Mergeable {
  public:
    RooDataSetMergeable() : m_wrapped() {  }

    virtual ~RooDataSetMergeable() { }
#if defined(__CINT__) || defined(__ROOTCLING__) || defined(R__DICTIONARY_FILENAME)
#else
    /** Constructor, forwards all arguments to T constructor. */
    template<class ...Args> RooDataSetMergeable(Args&& ... params) : m_wrapped(std::forward<Args>(params)...) { }
#endif

    /** Get the wrapped standard object. */
    RooDataSet& get() { return m_wrapped; }

    /** Get the wrapped standard object. */
    const RooDataSet& get() const { return m_wrapped; }

    /** Merge object 'other' into this one.
     *
     * Your derived class should implement this function. You can static_cast 'other' to your own type
     * (when called, this and other are guaranteed to point to objects of the same type).
     *
     * Note that 'other' will be deleted after the merge, so make sure you copy all data from it that you will need.
     */
    virtual void merge(const Mergeable* other)
    {
      auto* otherMergeable = const_cast<RooDataSetMergeable*>(static_cast<const RooDataSetMergeable*>(other));
      m_wrapped.append(otherMergeable->get());
    }

    /** Clear content of this object (e.g. set to zeroes).
     *
     * Called after sending the objects to another process. If no clearing is performed, the same data (e.g. histogram
     * entries) might be added again and again in each event.
     */
    virtual void clear()
    {
      m_wrapped.reset();
    }

  private:
    /** Wrapped object. */
    RooDataSet m_wrapped;

    ClassDef(RooDataSetMergeable, 1); /**< Wrap an STL map to make it mergeable. */
  };
}
