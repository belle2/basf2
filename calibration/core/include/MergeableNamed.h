#pragma once
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: David Dossett                                            *
 *               Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <TNamed.h>

class TCollection;
class TDirectory;

namespace Belle2 {
  /** Abstract base class for objects that can be merged but also named.
   *
   * Mostly copied from framework/pcore/Mergeable.h
   *
   * This is mainly intended to be a base class for custom objects we save via the CAF.
   * We do less TDirectory/TFile management here compared to Mergeable since we aren't
   * using the datastore and don't need to disconnect from TFiles etc. We'll be explicitly
   * managing that in the CAF (see CalibObjManager).
   */
  class MergeableNamed : public TNamed {
  public:
    MergeableNamed() : TNamed() { }
    virtual ~MergeableNamed() { }

    /** Merge object 'other' into this one.
     *
     * Your derived class must implement this function. You can static_cast 'other' to your own type
     * (when called, this and other are guaranteed to point to objects of the same type).
     *
     * Note that 'other' will be deleted after the merge, so make sure you copy all data from it that you will need.
     */
    virtual void merge(const MergeableNamed* other) = 0;

    /// Clear content of this object (e.g. set to zeroes). Called by the Reset() function
    virtual void clear() = 0;

    // Make a new
//   virtual TObject* clone() const = 0;

    /// Allow merging using TFileMerger if saved directly to a file.
    virtual Long64_t Merge(TCollection* hlist);
    /** Root-like Reset function for "template compatibility" with ROOT objects. Alias for clear(). */
    virtual void Reset() {clear();}
    /** Root-like Clone function for "template compatibility" with ROOT objects. Calls clone() */
//    virtual TObject* Clone(const char* newname = "") const {clone();}
    /** Root-like SetDirectory function for "template compatibility" with ROOT objects. Does nothing. */
    virtual void SetDirectory(TDirectory*) {}
  private:
    ClassDef(MergeableNamed, 0); /**< Abstract base class for objects that can be merged and named. */
  };
}
