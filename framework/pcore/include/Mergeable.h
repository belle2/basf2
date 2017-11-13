/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

class TCollection;
class TDirectory;

namespace Belle2 {
  /** Abstract base class for objects that can be merged.
   *
   * This is mainly intended for objects of c_Persistent durability in the data store,
   * where those inheriting from this class will be merged when using parallel processing.
   *
   * For storing TH1F, TNtuple and similar things, see RootMergeable<T>. For
   * STL maps, see MapMergeable<T>.
   *
   * The special handling for these objects is performed in DataStoreStreamer.
   */
  class Mergeable : public TObject {
  public:
    Mergeable() { }
    virtual ~Mergeable() { }

    /** Merge object 'other' into this one.
     *
     * Your derived class must implement this function. You can static_cast 'other' to your own type
     * (when called, this and other are guaranteed to point to objects of the same type).
     *
     * Note that 'other' will be deleted after the merge, so make sure you copy all data from it that you will need.
     */
    virtual void merge(const Mergeable* other) = 0;

    /** Clear content of this object (e.g. set to zeroes).
     *
     * Called after sending the objects to another process and after forking processes to
     * ensure there is at most one copy of the same data. If no clearing is performed,
     * the same data (e.g. histogram entries) might be added again and again in each event.
     */
    virtual void clear() = 0;

    /** An ugly little method that is called before event() for input and worker processes.
     *
     * Main use case is to detach any attached TFile from this object. In the output process,
     * it can stay attached (and grow as much as it likes).
     */
    virtual void removeSideEffects() {}

    /** Allow merging using TFileMerger if saved directly to a file.
     *
     * \note dictionaries containing your Mergeable class need to be loaded, so 'hadd' will not work currently.
     */
    virtual Long64_t Merge(TCollection* hlist);
    /** Root-like Reset function for "template compatibility" with ROOT objects. Alias for clear(). */
    virtual void Reset() {clear();}
    /** Root-like SetDirectory function for "template compatibility" with ROOT objects. Does nothing. */
    virtual void SetDirectory(TDirectory*) {}
  private:
    ClassDef(Mergeable, 0); /**< Abstract base class for objects that can be merged. */
  };
}
