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
  /** This class is meant to be used as functor object for the sorting of the VXD Clusters in the RecoTrack.
   *
   *  To change the Sorter, please make a different typedef in the RecoTrack.
   *  As the sorting depends on the geometry, alternative geometries, e.g. like a test-beam
   *  may require a different Sorter.
   */
  class SorterVXDHit {

    /** Constructor. */
    SorterVXDHit() {}

    /** Functor performing the actual sorting. */
    bool operator()() {return true;}
  };
}
