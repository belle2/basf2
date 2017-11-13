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
#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentTriple;

    /// Class filtering the neighborhood of segment triples with monte carlo information
    class MCSegmentTripleRelationFilter : public MCSymmetric<BaseSegmentTripleRelationFilter> {

    private:
      /// Type of the super class
      using Super = MCSymmetric<BaseSegmentTripleRelationFilter>;

    public:
      /// Constructor.
      MCSegmentTripleRelationFilter(bool allowReverse = true);

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Initialize the before event processing.
      void initialize() final;

    public:
      /**
       *  Main filter method returning the weight of the neighborhood relation.
       *  Return NAN if relation shall be rejected.
       */
      Weight operator()(const CDCSegmentTriple& fromTriple, const CDCSegmentTriple& toTriple) final;

    public:
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
