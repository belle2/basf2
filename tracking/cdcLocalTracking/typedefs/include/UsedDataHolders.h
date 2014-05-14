/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef USEDDATAHOLDERS_H
#define USEDDATAHOLDERS_H

// this file should be removed
// it only remains since the refactoring is not yet completely finished
// use the header eventdata/CDCEventData.h for all dataobjects defined
// and include others as needed


#include <tracking/cdcLocalTracking/numerics/numerics.h>
#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>

//fitting
#include <tracking/cdcLocalTracking/fitting/CDCRiemannFitter.h>
#include <tracking/cdcLocalTracking/fitting/CDCSZFitter.h>

//neighborhood
#include <tracking/cdcLocalTracking/algorithms/WeightedNeighborhood.h>


namespace Belle2 {
  namespace CDCLocalTracking {

    //some additional transient types for exchange between objects
    //typedef std::vector<const CDCRecoFacet*>  CDCRecoFacetPtrSegment;
    //typedef CDCRecoTangentVector CDCRecoTangentCollection; //for now just used for io to python

  }
} // namespace Belle2
#endif

// USEDDATAHOLDERS
