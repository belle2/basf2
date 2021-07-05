/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentPairRelation/BaseSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPair/MCSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class filtering the neighborhood of axial stereo segment pairs with monte carlo information
    class MCSegmentPairRelationFilter : public MCSymmetric<BaseSegmentPairRelationFilter> {

    private:
      /// Type of the super class
      using Super = MCSymmetric<BaseSegmentPairRelationFilter>;

    public:
      /// Constructor setting to default reversal symmetry.
      explicit MCSegmentPairRelationFilter(bool allowReverse = true);

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Initialize the before event processing.
      void initialize() final;

    public:
      /**
       *  Main filter method returning the weight of the neighborhood relation.
       *  Return NAN if relation shall be rejected.
       */
      Weight operator()(const CDCSegmentPair& fromSegmentPair, const CDCSegmentPair& toSegmentPair) final;

    public:
      /// Setter for the allow reverse parameter
      void setAllowReverse(bool allowReverse) final {
        Super::setAllowReverse(allowReverse);
        m_mcSegmentPairFilter.setAllowReverse(allowReverse);
      }

    private:
      /// Instance of the Monte Carlo axial stereo segment filter for rejection of false cells.
      MCSegmentPairFilter m_mcSegmentPairFilter;
    };
  }
}
