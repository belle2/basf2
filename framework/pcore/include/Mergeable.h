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


#include <TObject.h>

namespace Belle2 {
  /** Abstract base class for objects that can be merged.
   *
   * This is mainly intended for objects of c_Persistent durability in the data store,
   * where those inheriting from this class will be merged when using parallel processing.
   */
  class Mergeable : public TObject {
  public:
    Mergeable() { }
    virtual ~Mergeable() { }
    /** Merge object 'other' into this one.
     *
     * Your derived class should implement this function. You can static_cast 'other' to your own type
     * (when called, this and other are guaranteed to point to objects of the same type).
     *
     * Note that 'other' will be deleted after the merge, so make sure you copy all data from it that you will need.
     */
    virtual void merge(const Mergeable* other) = 0;

    virtual void clear() = 0;
  private:
    ClassDef(Mergeable, 0); /**< Abstract base class for objects that can be merged. */
  };
}
