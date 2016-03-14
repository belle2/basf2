/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segment_pair_relation/BaseSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segment_pair/MCSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilterMixin.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    ///Class filtering the neighborhood of axial stereo segment pairs with monte carlo information
    class MCSegmentPairRelationFilter :
      public MCSymmetricFilterMixin<Filter<Relation<CDCSegmentPair> > > {

    private:
      /// Type of the super class
      typedef MCSymmetricFilterMixin<Filter<Relation<CDCSegmentPair> > > Super;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

    public:
      /** Constructor setting to default reversal symmetry. */
      MCSegmentPairRelationFilter(bool allowReverse = true);

    public:
      /// Used to prepare the Monte Carlo information for this event.
      virtual void beginEvent() override final;

      /// Forwards the modules initialize to the filter
      virtual void initialize() override final;

      /// Forwards the modules initialize to the filter
      virtual void terminate() override final;

      /// Main filter method returning the weight of the neighborhood relation. Return NOT_A_NEIGHBOR if relation shall be rejected.
      virtual NeighborWeight operator()(const CDCSegmentPair& fromSegmentPair,
                                        const CDCSegmentPair& toSegmentPair) override final;

    public:
      /// Setter for the allow reverse parameter
      void setAllowReverse(bool allowReverse) override
      {
        Super::setAllowReverse(allowReverse);
        m_mcSegmentPairFilter.setAllowReverse(allowReverse);
      }

    private:
      /// Instance of the Monte Carlo axial stereo segment filter for rejection of false cells.
      MCSegmentPairFilter m_mcSegmentPairFilter;

    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2
