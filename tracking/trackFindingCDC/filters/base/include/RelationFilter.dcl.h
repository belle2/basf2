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

      /// Selects the objects possibly related to the given one from the given pool of objects.
      virtual std::vector<AObject*> getPossibleTos(AObject* from,
                                                   const std::vector<AObject*>& objects) const;

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
