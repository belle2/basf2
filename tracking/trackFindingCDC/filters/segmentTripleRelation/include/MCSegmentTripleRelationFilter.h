/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentTripleRelation/BaseSegmentTripleRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTriple/MCSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilterMixin.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Class filtering the neighborhood of segment triples with monte carlo information
    class MCSegmentTripleRelationFilter:
      public MCSymmetricFilterMixin<BaseSegmentTripleRelationFilter > {

    private:
      /// Type of the super class
      using Super = MCSymmetricFilterMixin<BaseSegmentTripleRelationFilter >;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

    public:
      /** Constructor. */
      MCSegmentTripleRelationFilter(bool allowReverse = true);

      /** Main filter method returning the weight of the neighborhood relation.
       *  Return NAN if relation shall be rejected.*/
      Weight
      operator()(const CDCSegmentTriple& triple, const CDCSegmentTriple& neighborTriple) final;

      /// Setter for the allow reverse parameter
      void setAllowReverse(bool allowReverse) override
      {
        Super::setAllowReverse(allowReverse);
        m_mcSegmentTripleFilter.setAllowReverse(allowReverse);
      }

    private:
      /// Instance of the Monte Carlo segment triple filter for rejection of false cells.
      MCSegmentTripleFilter m_mcSegmentTripleFilter;
    };
  }
}
