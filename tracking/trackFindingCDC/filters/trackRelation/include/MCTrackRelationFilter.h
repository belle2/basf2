/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/trackRelation/BaseTrackRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of track relations based on MC information.
    class MCTrackRelationFilter : public MCSymmetric<BaseTrackRelationFilter > {

    private:
      /// Type of the super class
      using Super = MCSymmetric<BaseTrackRelationFilter>;

    public:
      /**
       *  Constructor also setting the switch ,
       *  if the reversed version of a track relation (in comparision to MC truth) shall be accepted.
       */
      explicit MCTrackRelationFilter(bool allowReverse = false);

    public:
      /// Checks if a track relation is a good combination.
      Weight operator()(const CDCTrack& fromTrack, const CDCTrack& toTrack) final;
    };
  }
}
