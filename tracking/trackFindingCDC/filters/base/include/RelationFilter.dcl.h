/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>

#include <vector>

#include <cmath>

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
       * Filter over a vector of relations. We need to override it since derrived relation classes do not implement a call with a pointer to a relation.
       */
      virtual std::vector<float> operator()(const std::vector <Relation<AObject>*>& objs) override
      {
        std::vector<float> out(objs.size());
        for (size_t iObj = 0; iObj < objs.size(); iObj += 1) {
          if (objs[iObj]) {
            out[iObj] = operator()(*objs[iObj]);
          } else {
            out[iObj] = NAN;
          }
        }
        return out;
      }

      /**
       *  Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.
       */
      Weight operator()(const Relation<AObject>& relation) override;
    };
  }
}
