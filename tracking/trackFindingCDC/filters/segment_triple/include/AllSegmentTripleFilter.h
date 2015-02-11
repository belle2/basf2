/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALLSEGMENTTRIPLEFILTER_H_
#define ALLSEGMENTTRIPLEFILTER_H_

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackFindingCDC/algorithms/CellWeight.h>

#include "BaseSegmentTripleFilter.h"

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of segment triples based on simple criterions
    class AllSegmentTripleFilter : public BaseSegmentTripleFilter {

    public:
      /** Constructor. */
      AllSegmentTripleFilter() {;}

      /** Destructor.*/
      virtual ~AllSegmentTripleFilter() {;}

    public:
      /// Clears all remember information from the last event
      virtual void clear() override final {;}

      /// Forwards the modules initialize to the filter
      virtual void initialize() override final {;}

      /// Forwards the modules initialize to the filter
      virtual void terminate() override final {;}

      /// All implementation returns the size of the segment triples accepting all.
      virtual CellWeight isGoodSegmentTriple(const CDCSegmentTriple& triple) override final;

    }; // end class AllSegmentTripleFilter

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //ALLSEGMENTTRIPLEFILTER_H_
