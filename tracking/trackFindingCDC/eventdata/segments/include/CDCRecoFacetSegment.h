/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOFACETSEGMENT_H
#define CDCRECOFACETSEGMENT_H

#include <tracking/trackFindingCDC/eventdata/collections/CDCRecoFacetVector.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    // for now a typedef is enough
    // may get additional methods if necessary
    /// A segment consisting of adjacent reconstructed tangents
    typedef CDCRecoFacetVector CDCRecoFacetSegment;

  }
}


#endif // CDCRECOFACETSEGMENT



