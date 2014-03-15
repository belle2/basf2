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

namespace Belle2 {
  /** This class is meant to be used as functor object for the sorting of the CDC Hits in the RecoTrack. */
  class SorterBaseCDCHit {
  public:
    /* Constructor. */
    SorterBaseCDCHit() {}

    virtual ~SorterBaseCDCHit() {}

    /** Functor performing the actual sorting. */
    virtual bool operator()() {return true;}
  };
}
