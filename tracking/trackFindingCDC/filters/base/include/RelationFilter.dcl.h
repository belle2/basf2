/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Base class for filtering the neighborhood of objects
    template <class AObject>
    class RelationFilter : public Filter<Relation<AObject> > {

    public:
      /// Default constructor
      RelationFilter();

      /// Default destructor
      virtual ~RelationFilter();

      /// Filters the given collection of objects for the once neighboring the given one
      virtual std::vector<AObject*> getPossibleNeighbors(
        AObject* from,
        const typename std::vector<AObject*>::const_iterator& itBegin,
        const typename std::vector<AObject*>::const_iterator& itEnd) const;

      /**
       *  Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NAN to reject all segment neighbors.
       */
      virtual Weight operator()(const AObject& from, const AObject& to);

      /**
       *  Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.
       */
      Weight operator()(const Relation<AObject>& relation) override;
    };
  }
}
