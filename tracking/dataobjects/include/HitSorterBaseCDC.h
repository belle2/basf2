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
  /** This class is meant to be used as functor object for the sorting of the CDC Hits in the RecoTrack. */
  class HitSorterBaseCDC : public TObject {

    /* Constructor. */
    HitSorterBaseCDC() {}

    /** As this is potentially as Base class, the destructor needs to be virtual. */
    virtual ~HitSorterBaseCDC() {}

    /** Sorter operation for the set of CDCHits in the RecoTrack.
     *
     *  FIXME For the CDC a valid Sorter as default should be quite easy.
     */
    virtual bool operator()() {return true;}

    /** ROOTification of object; Streaming may make sense for special cases like testbeam. */
    ClassDef(HitSorterBaseCDC, 1);
  };
}
