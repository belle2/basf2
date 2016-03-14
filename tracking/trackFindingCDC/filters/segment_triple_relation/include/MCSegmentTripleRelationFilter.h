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

#include <tracking/trackFindingCDC/filters/segment_triple_relation/BaseSegmentTripleRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segment_triple/MCSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilterMixin.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    ///Class filtering the neighborhood of segment triples with monte carlo information
    class MCSegmentTripleRelationFilter:
      public MCSymmetricFilterMixin<Filter<Relation<CDCSegmentTriple> > > {

    private:
      /// Type of the super class
      typedef MCSymmetricFilterMixin<Filter<Relation<CDCSegmentTriple> > > Super;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

    public:
      /** Constructor. */
      MCSegmentTripleRelationFilter(bool allowReverse = true);

      /// Clears stored information for a former event
      virtual void clear() override final;

      /// Forwards the initialize method from the module
      virtual void initialize() override final;

      /// Forwards the terminate method from the module
      virtual void terminate() override final;

      /** Main filter method returning the weight of the neighborhood relation.
       *  Return NOT_A_NEIGHBOR if relation shall be rejected.*/
      virtual NeighborWeight operator()(const CDCSegmentTriple& triple,
                                        const CDCSegmentTriple& neighborTriple) override final;

      /// Setter for the allow reverse parameter
      void setAllowReverse(bool allowReverse) override
      {
        Super::setAllowReverse(allowReverse);
        m_mcSegmentTripleFilter.setAllowReverse(allowReverse);
      }

    private:
      /// Instance of the Monte Carlo segment triple filter for rejection of false cells.
      MCSegmentTripleFilter m_mcSegmentTripleFilter;

    }; // end class


  } //end namespace TrackFindingCDC
} //end namespace Belle2
