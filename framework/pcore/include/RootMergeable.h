/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/pcore/Mergeable.h>
#include <framework/logging/Logger.h>

#include <TROOT.h>
#include <TFile.h>
#include <TList.h>
#include <TH1F.h>
#include <TH1D.h>
#include <TH2F.h>
#include <TNtuple.h>

namespace Belle2 {
  /** Wrap a root histogram or TNtuple to make it mergeable.
   *
   * To use it to save data in your module:
   * \code
     setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);
     \endcode
     create RootMergeable<X> in initalize (or in your constructor) of durability DataStore::c_Persistent,
     register it by calling registerInDataStore() and construct() the actual histogram. Especially for larger
     TTrees, you should also create a TFile and cd() into it before creating the histogram. To actually save
     the objects, use the following in terminate() to ensure this is done only in the output process (where
     the data from all events will be collected):
     \code
     if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
       //use TFile you created in initialize()
       mergeablePtr->write(m_file);
     }
     \endcode
   *
   * This should work out of the box for TTree, TNtuple, TH1F, TH2F, TH1D, and TH2D. Additional template instantiations need
   * an entry in framework/pcore/include/linkdef.h, please contact the framework librarian if your use
   * case requires other classes.
   *
   * Be aware that for larger histograms, this way of sharing the data may not be a good idea.
   * E.g. for hundred thousand bins, basf2 would transfer about half a megabyte in each event,
   * which may take a significant fraction of total processing time. Trees or Ntuples will however
   * only transfer the newly added data, which is probably manageable.
   *
   * To deal with ownership issues arising from objects belonging to TFiles, each
   * RootMergeable object is added to the global list gROOT->GetListOfCleanups(),
   * which will ensure RootMergeable::RecursiveRemove() is called when m_wrapped is
   * deleted.
   *
   * \sa Mergeable
   */
  template <class T> class RootMergeable : public Mergeable {
  public:
    /** default constructor for root. */
    RootMergeable() : m_wrapped(nullptr) { }
    /** Constructor, forwards all arguments to T constructor. */
    template<class ...Args> explicit RootMergeable(Args&& ... params) : m_wrapped(new T(std::forward<Args>(params)...))
    {
      m_wrapped->SetBit(TObject::kMustCleanup); //ensure RecursiveRemove() is called
      gROOT->GetListOfCleanups()->Add(this);
    }

    virtual ~RootMergeable()
    {
      gROOT->GetListOfCleanups()->Remove(this);
      delete m_wrapped;
    }

    /** Replace wrapped object with p (takes ownership). */
    void assign(T* p)
    {
      delete m_wrapped;
      m_wrapped = p;
      m_wrapped->SetBit(TObject::kMustCleanup);
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
    void write(TDirectory* file)
    {
      if (m_wrapped->GetDirectory() != nullptr and m_wrapped->GetDirectory() != file) {
        B2ERROR("RootMergeable: wrapped object belongs to other file, Write() might crash. Make sure your histogram/ntuple already belongs to the file you want to save it to before filling (e.g. in initialize())");
      }
      file->cd();
      m_wrapped->Write(nullptr, TObject::kOverwrite);
      removeSideEffects();
    }

    /** Merge object 'other' into this one.
     *
     * Your derived class should implement this function. You can static_cast 'other' to your own type
     * (when called, this and other are guaranteed to point to objects of the same type).
     *
     * Note that 'other' will be deleted after the merge, so make sure you copy all data from it that you will need.
     */
    virtual void merge(const Mergeable* other) override
    {
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
    virtual void clear() override
    {
      m_wrapped->Reset();
    }

    /** An ugly little method that is called before event() for input and worker processes.
     *
     * Main use case is to detach any attached TFile from this object. In the output process,
     * it can stay attached (and grow as much as it likes).
     */
    virtual void removeSideEffects() override
    {
      if (!m_wrapped) return;

      m_wrapped->SetDirectory(nullptr);
      //if we are the only owner, this becomes unnecessary
      gROOT->GetListOfCleanups()->Remove(this);
    }

    /** Called from ROOT if obj is deleted. Kill pointer to avoid double free. */
    virtual void RecursiveRemove(TObject* obj) override
    {
      if (obj == m_wrapped)
        m_wrapped = nullptr;
    }

  private:
    /** Wrapped root object. */
    T* m_wrapped;

    ClassDefOverride(RootMergeable, 2); /**< Wrap a root histogram or ntuple to make them mergeable. */
  };
}
