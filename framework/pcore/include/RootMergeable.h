#pragma once
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/pcore/Mergeable.h>
#include <framework/logging/Logger.h>

#include <TFile.h>
#include <TList.h>
#include <TH1F.h>
#include <TH1D.h>
#include <TH2F.h>
#include <TNtuple.h>

namespace Belle2 {
  /** Wrap a root histogram or TNtuple to make them mergeable.
   *
   * To use it to save data in your module:
   * \code
     setPropertyFlags(c_Parallelprocessing | c_terminateInAllProcesses);
     \endcode
     create RootMergeable<X> in initalize (or in your constructor) of durability DataStore::c_Persistent,
     register it by calling registerInDataStore() and construct() the actual histogram.
     in terminate():
     \code
     if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
       //use TFile you created in initialize()
       mergeablePtr->write(m_file);
     }
     \endcode
   *
   * This should work out of the box for TNtuple, TH1F and TH2F. Additional template instantiations need
   * an entry in framework/pcore/include/linkdef.h, please contact the framework librarian if your use
   * case requires other classes.
   *
   * Be aware that for larger histograms, this way of sharing the data may not be a good idea.
   * E.g. for hundred thousand bins, basf2 would transfer about half a MegaByte in each event,
   * which may take a significant fraction of total processing time.
   *
   * TODO: after construction, I should probably do SetDirectory(null) directly, and offer a function to set it explicitly.
   *
   * \sa Mergeable
   */
  template <class T> class RootMergeable : public Mergeable {
  public:
#if defined(__CINT__) || defined(__ROOTCLING__) || defined(R__DICTIONARY_FILENAME)
    RootMergeable() : m_wrapped(nullptr) {  }
#else
    /** Constructor, forwards all arguments to T constructor. */
    template<class ...Args> RootMergeable(Args&& ... params) : m_wrapped(new T(std::forward<Args>(params)...)) { }
#endif

    virtual ~RootMergeable() { delete m_wrapped; }

    /** Replace wrapped object with p (takes ownership). */
    void assign(T* p) {
      delete m_wrapped;
      m_wrapped = p;
    }

    /** Get the wrapped root object. */
    T& get() { return *m_wrapped; }

    /** Get the wrapped root object. */
    const T& get() const { return *m_wrapped; }

    /** Write the wrapped object into 'file', overwriting existing objects of same name.
     *
     * This function should be prefered to calling Write() by hand.
     *
     * \note wrapped object must already be in 'file' before filled, or not part of any file at all. This function will throw an error and might crash if this is not the case.
     */
    void write(TFile* file) {
      if (m_wrapped->GetDirectory() != nullptr and m_wrapped->GetDirectory() != file) {
        B2ERROR("RootMergeable: wrapped object belongs to other file, Write() might crash. Make sure your histogram/ntuple already belongs to the file you want to save it to before filling (e.g. in initialize())");
      }
      file->cd();
      m_wrapped->Write(nullptr, TObject::kOverwrite);
      m_wrapped->SetDirectory(nullptr);
    }

    /** Merge object 'other' into this one.
     *
     * Your derived class should implement this function. You can static_cast 'other' to your own type
     * (when called, this and other are guaranteed to point to objects of the same type).
     *
     * Note that 'other' will be deleted after the merge, so make sure you copy all data from it that you will need.
     */
    virtual void merge(const Mergeable* other) {
      auto* otherMergeable = const_cast<RootMergeable<T>*>(static_cast<const RootMergeable<T>*>(other));
      TList list;
      list.SetOwner(false);
      list.Add(&otherMergeable->get());

      m_wrapped->Merge(&list);
    }

    /** Clear content of this object (e.g. set to zeroes).
     *
     * Called after sending the objects to another process. If no clearing is performed, the same data (e.g. histogram
     * entries) might be added again and again in each event.
     */
    virtual void clear() {
      m_wrapped->Reset();
    }

    /** An ugly little method that is called before event() for input and parallel processes.
     *
     * Main use case is to detach any attached TFile from this object. In the output process,
     * it can stay attached (and grow as much as it likes).
     */
    virtual void removeSideEffects() {
      m_wrapped->SetDirectory(NULL);
    }
  private:
    /** Wrapped root object. */
    T* m_wrapped;

    ClassDef(RootMergeable, 2); /**< Wrap a root histogram or ntuple to make them mergeable. */
  };
}
