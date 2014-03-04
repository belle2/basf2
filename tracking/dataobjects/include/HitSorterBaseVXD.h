/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {
  /** This class is meant to be used as functor object for the sorting of the VXD Hits in the RecoTrack. */
  class HitSorterBaseVXD : public TObject {

    /** Constructor. */
    HitSorterBaseVXD() {}

    /** Virtual Destructor, as this is intendet to be a base class. */
    virtual ~HitSorterBaseVXD() {}
    /**
    //FIXME Probably for the VXD as well a reasonable default can be attached.
    //However, if we want to stay in the dataobjects, we aren't allowed to be dependent
    // on the geometry. I'm not sure, if we can sort the hits without the radius information only
    // based in the VxdId.
    // For special cases like the testbeam, this is definitively not possible.
     *
     */
    virtual bool operator()() {return true;}

    /** ROOTification with streaming. */
    ClassDef(HitSorterBaseVXD, 1)
  };
}
