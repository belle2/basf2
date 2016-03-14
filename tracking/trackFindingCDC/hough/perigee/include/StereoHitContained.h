/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLTaggedWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/RLTagged.h>

#include <tracking/trackFindingCDC/numerics/Sign.h>
#include <tracking/trackFindingCDC/numerics/numerics.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Predicate class to check for the containment of axial and stereo hits
     *  in some hough space part.
     *
     *  The hough space is parameterised as a template parameter and is a basic
     *  objected that has been broadened by a sweep in the directions of interested.
     */
    template<class AInBox>
    class StereoHitContained : public AInBox {

    public:
      /// Exposing the base constructor
      using AInBox::AInBox;

      /// The hough box which represents the hough space part to be investigated
      using HoughBox = typename AInBox::HoughBox;


      /** Checks if more than 66% of the hits in this segment are contained in the phi0 curv hough space
       *  Returns the sum of the individual hit weights in this case. Else returns NAN.*/
      inline Weight operator()(const CDCRecoSegment2D* const& recoSegment2D,
                               const HoughBox* const& houghBox)
      {
        size_t nHits = recoSegment2D->size();
        auto weightOfHit = [this, &houghBox](const Weight & totalWeight,
        const CDCRecoHit2D & recoHit2D) -> Weight {
          Weight hitWeight = this->operator()(&recoHit2D, houghBox);
          return std::isnan(hitWeight) ? totalWeight : totalWeight + hitWeight;
        };

        Weight totalWeight = std::accumulate(recoSegment2D->begin(),
                                             recoSegment2D->end(),
                                             static_cast<Weight>(0.0),
                                             weightOfHit);

        // Require a efficiency of 66%
        constexpr const Weight minEfficiency = 2.0 / 3;
        if (totalWeight > nHits * minEfficiency) {
          return totalWeight;
        } else {
          return NAN;
        }
      }

      /** Checks if the two dimensional reconstructed hit is contained in a phi0 curv hough space.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       */
      inline Weight operator()(const CDCRecoHit2D* const& recoHit2D,
                               const HoughBox* const& houghBox)
      {
        const CDCWire& wire = recoHit2D->getWire();
        const double signedDriftLength = recoHit2D->getSignedRefDriftLength();
        bool isIn = contains(*houghBox, wire, signedDriftLength);
        ERightLeft rlInfo = recoHit2D->getRLInfo();
        return isIn ? 1.0 + isValid(rlInfo) * m_rlWeightGain : NAN;
      }


      /** Checks if the wire hit is contained in a phi0 curv hough space.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       *  Accepts if either the right passage hypothesis or the left passage hypothesis
       *  is in the hough box.
       */
      inline Weight operator()(const CDCWireHit* const& wireHit,
                               const HoughBox* const& houghBox)
      {
        const CDCWire& wire = wireHit->getWire();
        const double driftLength = wireHit->getRefDriftLength();

        ERightLeft rlInfo = containsRightOrLeft(*houghBox, wire, driftLength);
        return isValid(rlInfo) ? 1.0 + std::abs(rlInfo) * m_rlWeightGain : NAN;
      }

      /** Checks if the wire hit is contained in a phi0 curv hough space.
       *  If the wire hit was already determined to be right or left of all tracks
       *  in a wider hough hough box up the hierarchy only evaluate for that orientation.
       *  If one of the right left passage hypothesis can be ruled out in this hough box
       *  signal so by tagging it.
       *  Note the that the RLTagged<WireHit*> is obtained as non-const reference to
       *  be able to write back the new right left passage hypothesis.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       *  Accepts if either the right passage hypothesis or the left passage hypothesis
       *  is in the hough box.
       */
      inline Weight operator()(CDCRLTaggedWireHit& rlTaggedWireHit,
                               const HoughBox* const& houghBox)
      {
        const CDCWire& wire = rlTaggedWireHit->getWire();
        const ERightLeft rlInfo = rlTaggedWireHit.getRLInfo();
        const double driftLength = rlTaggedWireHit->getRefDriftLength();

        ERightLeft newRLInfo =
          containsRightOrLeft(*houghBox, wire, driftLength, rlInfo);

        rlTaggedWireHit.setRLInfo(newRLInfo);
        return isValid(newRLInfo) ? 1.0 + std::abs(newRLInfo) * m_rlWeightGain : NAN;
      }

      /** Checks if a wire hit at a drift length is contained in the hough space part
       *  It investigates both right left passage hypotheses.
       *  The evaluation can be limited by the optional rlInfo parameter, for instance
       *  when one passage hypothese could already be ruled out.
       *
       *  @returns
       *      * ERightLeft::c_Unknown if both right and left are still possible.
       *      * ERightLeft::c_Left if only left is still possible.
       *      * ERightLeft::c_Right if only right is still possible.
       *      * ERightLeft::c_Invalid if non of the orientations is possible.
       */
      inline ERightLeft containsRightOrLeft(const HoughBox& houghBox,
                                            const CDCWire& wire,
                                            const double driftLength,
                                            const ERightLeft rlInfo = ERightLeft::c_Unknown)
      {
        bool isRightIn = rlInfo != ERightLeft::c_Left and contains(houghBox, wire, driftLength);
        bool isLeftIn = rlInfo != ERightLeft::c_Right and contains(houghBox, wire, -driftLength);

        if (isRightIn and isLeftIn) {
          return ERightLeft::c_Unknown;
        } else if (isRightIn) {
          return ERightLeft::c_Right;
        } else if (isLeftIn) {
          return ERightLeft::c_Left;
        } else {
          return ERightLeft::c_Invalid;
        }
      }

      /** Checks if a wire hit at a signed drift length is contained in the hough space part */
      inline bool contains(const HoughBox& houghBox,
                           const CDCWire& wire,
                           const double signedDriftLength)
      {
        const Vector2D& pos2D = wire.getRefPos2D();
        //const Vector2D& pos2D = wire.getWirePos2DAtZ(0);
        const Vector2D& movePerZ = wire.getMovePerZ();
        ILayer iCLayer(wire.getICLayer());
        //B2INFO("movePerZ = " << movePerZ);
        const ESign distSign = this->getDistanceSign(houghBox,
                                                     pos2D.x(),  pos2D.y(),
                                                     signedDriftLength,
                                                     movePerZ.x(), movePerZ.y(),
                                                     iCLayer);
        const bool isIn = distSign == ESign::c_Zero;
        return isIn;
      }

    public:
      /// Setter for the gain in weight for hits which rl passage could be uniquly resolved.
      void setRLWeightGain(const float& rlWeightGain)
      { m_rlWeightGain = rlWeightGain;}

      /// Getter for the gain in weight for hits which rl passage could be uniquly resolved.
      float setRLWeightGain() const
      { return m_rlWeightGain; }

    private:
      /** Weight gain for a hit which right left passage hypotheses could be uniquely resolved. */
      float m_rlWeightGain = 0;
    };

  } // end namespace TrackFindingCDC
} // end namespace Belle2
