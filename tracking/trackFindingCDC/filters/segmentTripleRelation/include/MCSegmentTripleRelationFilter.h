/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      explicit MCSegmentTripleRelationFilter(bool allowReverse = true);

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
