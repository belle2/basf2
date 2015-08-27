/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/numerics/SignType.h>

#include <list>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    class LegendreHit;
    class CDCTrack;
    class CDCRecoHit3D;
    class Vector2D;

    class TrackMergerNew {
    public:

      /**
       * This class is to be used static only.
       */
      TrackMergerNew() = delete;

      /** Tries to split back-to-back tracks into two different tracks */
      static CDCTrack& splitBack2BackTrack(CDCTrack& trackCandidate);

      /** Tries to split back-to-back tracks into two different tracks */
      static bool checkBack2BackTrack(CDCTrack& trackCandidate);

      static SignType getChargeSign(CDCTrack& track) ;

      static int getCurvatureSignWrt(const CDCRecoHit3D& hit, Vector2D xy) ;

      static double getPhi(const CDCRecoHit3D& hit) ;

      static void deleteAllMarkedHits(CDCTrack& track);

    private:


      /** Some typedefs for the results of the merging process */
      typedef unsigned int TrackCandidateIndex;
      typedef std::pair<TrackCandidateIndex, CDCTrack*> TrackCandidateWithIndex;
      typedef double Probability;
      typedef std::pair<CDCTrack*, Probability> BestMergePartner;

      /**
       * After the candidate-to-merge finding, some hits are marked as bad. This method resets them.
       * @param otherTrackCandidate to reset
       */
      static void resetHits(CDCTrack& otherTrackCandidate);

      /**
       * This parameter is the minimum probability a fit must have to lead to the result: merge
       */
      static constexpr double m_minimum_probability_to_be_merged = 0.3;
    };
  }
}
