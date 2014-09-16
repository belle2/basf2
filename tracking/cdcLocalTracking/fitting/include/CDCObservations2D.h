/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCOBSERVATIONS2D_H
#define CDCOBSERVATIONS2D_H

#ifndef __CINT__
#include <Eigen/Dense>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>
#endif

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>


namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class serving as a storage of observed drift circles to present to the Riemann fitter
    class CDCObservations2D : public UsedTObject {

    public:
#ifndef __CINT__
      /// Matrix type used to wrap the raw memory chunk of values generated from the various hit types for structured vectorized access.
      typedef Eigen::Map< Eigen::Matrix< FloatType, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > > EigenObservationMatrix;
#endif

      /// Empty constructor.
      CDCObservations2D();

      /// Empty destructor.
      ~CDCObservations2D();

      /// Returns the number of observations stored
      size_t size() const
      { return  m_observations.size() / 4; }

      /// Returns true if there are no observations stored.
      bool empty() const
      { return m_observations.empty(); }

      /// Removes all observations stored
      void clear()
      { m_observations.clear(); }

      /// Reserves enough space for nObservations
      void reserve(const size_t& nObservations)
      { m_observations.reserve(nObservations * 4); }

      /// Getter for the x value of the observation at the given index.
      FloatType getX(const int& iObservation) const
      { return m_observations[iObservation * 4]; }

      /// Getter for the y value of the observation at the given index.
      FloatType getY(const int& iObservation) const
      { return m_observations[iObservation * 4 + 1]; }

      /// Getter for the signed drift radius of the observation at the given index.
      FloatType getDriftLength(const int& iObservation) const
      { return m_observations[iObservation * 4 + 2]; }

      /// Getter for the weight / inverse variance of the observation at the given index.
      FloatType getWeight(const int& iObservation) const
      { return m_observations[iObservation * 4 + 3]; }



      /// Appends the observed position - drift radius is assumed to be zero if not given
      void append(const FloatType& x,
                  const FloatType& y,
                  const FloatType& signedRadius = 0.0,
                  const FloatType& weight = 1.0) {

        if (std::isnan(x)) return;
        if (std::isnan(y)) return;

        if (std::isnan(signedRadius)) {
          B2WARNING("Signed radius is nan. Skipping observation");
          return;
        }

        if (std::isnan(weight)) {
          B2WARNING("Weight is nan. Skipping observation");
          return;
        }

        m_observations.push_back(x);
        m_observations.push_back(y);
        m_observations.push_back(signedRadius);
        m_observations.push_back(weight);
      }

      /// Appends the observed position - drift radius is assumed to be zero if not given
      void append(const Belle2::CDCLocalTracking::Vector2D& pos2D, const FloatType& signedRadius = 0.0, const FloatType& weight = 1.0)
      { append(pos2D.x(), pos2D.y(), signedRadius, weight); }

      /// Appends the observed position - drift radius is take a positiv number
      void append(const Belle2::CDCLocalTracking::CDCWireHit& wireHit)
      { append(wireHit.getRefPos2D(), wireHit.getRefDriftLength(), 1.0 / wireHit.getRefDriftLengthVariance()); }

      /// Appends the observed position - drift radius is signed number according to the orientation
      void append(const Belle2::CDCLocalTracking::CDCRLWireHit& rlWireHit)
      { append(rlWireHit.getRefPos2D(), rlWireHit.getSignedRefDriftLength(), 1.0 / rlWireHit.getRefDriftLengthVariance()); }

      /// Appends the two observed position - drift radius is signed number according to the orientation
      void append(const Belle2::CDCLocalTracking::CDCRLWireHitPair& rlWireHitPair) {
        append(rlWireHitPair.getFromRLWireHit());
        append(rlWireHitPair.getToRLWireHit());
      }

      /// Appends the three observed position - drift radius is signed number according to the orientation
      void append(const Belle2::CDCLocalTracking::CDCRLWireHitTriple& rlWireHitTriple) {
        append(rlWireHitTriple.getStartRLWireHit());
        append(rlWireHitTriple.getMiddleRLWireHit());
        append(rlWireHitTriple.getEndRLWireHit());
      }

      /// Appends the observed position - drift radius is signed number according to the orientation
      void append(const Belle2::CDCLocalTracking::CDCRecoHit2D& recoHit2D, bool usePosition = false) {
        if (usePosition) {
          const Vector2D& recoPos2D = recoHit2D.getRecoPos2D();
          const FloatType driftLength = 0.0;
          const FloatType weight = 1.0 / recoHit2D.getWireHit().getRefDriftLengthVariance();
          append(recoPos2D, driftLength, weight);
        } else {
          append(recoHit2D.getRLWireHit());
        }
      }

      /// Appends the observed position - drift radius is signed number according to the orientation
      void append(const Belle2::CDCLocalTracking::CDCRecoHit3D& recoHit3D, bool usePosition = false) {
        if (usePosition) {
          const Vector2D& recoPos2D = recoHit3D.getRecoPos2D();
          const FloatType driftLength = 0.0;
          const FloatType weight = 1.0 / recoHit3D.getWireHit().getRefDriftLengthVariance();
          append(recoPos2D, driftLength, weight);
        } else {
          const Vector2D& recoPos2D = recoHit3D.getRecoWirePos2D();
          const FloatType driftLength = recoHit3D.getSignedRecoDriftLength();
          const FloatType weight = 1.0 / recoHit3D.getRecoDriftLengthVariance();
          append(recoPos2D, driftLength, weight);
        }
      }


      /// Appends all reconstructed hits from the two dimensional segment, usePosition indicates whether the absolute position shall be used instead of the oriented wire hit information
      void append(const CDCRecoSegment2D& recoSegment2D, bool usePosition = false) {
        for (const CDCRecoHit2D & recoHit2D :  recoSegment2D) {
          append(recoHit2D, usePosition);
        }
      }

      /// Appends all reconstructed hits from the three dimensional segment, usePosition indicates whether the absolute position shall be used instead of the oriented wire hit information next to the reconstructed position
      void append(const CDCRecoSegment3D& recoSegment3D, bool usePosition = false) {
        for (const CDCRecoHit3D & recoHit3D :  recoSegment3D) {
          append(recoHit3D, usePosition);
        }
      }


      /// Appends all reconstructed hits from the two axial segments, usePosition indicates whether the absolute position shall be used instead of the oriented wire hit information.
      void append(const CDCAxialAxialSegmentPair& axialAxialSegmentPair,
                  bool usePosition = false) {
        const CDCRecoSegment2D* ptrStartSegment2D = axialAxialSegmentPair.getStart();
        if (ptrStartSegment2D) {
          const CDCRecoSegment2D& startSegment2D = *ptrStartSegment2D;
          append(startSegment2D, usePosition);
        }

        const CDCRecoSegment2D* ptrEndSegment2D = axialAxialSegmentPair.getEnd();
        if (ptrEndSegment2D) {
          const CDCRecoSegment2D& endSegment2D = *ptrEndSegment2D;
          append(endSegment2D, usePosition);
        }
      }

      /// Appends all the reference wire positions. Always use position since there is no other mode. For cross check to legendre finder.
      void append(const std::vector<const Belle2::CDCLocalTracking::CDCWire*>& wires,
                  bool usePosition  __attribute__((__unused__)) = false) {
        for (const CDCWire * ptrWire : wires) {
          if (not ptrWire) continue;
          const CDCWire& wire = *ptrWire;
          const Vector2D& wirePos = wire.getRefPos2D();
          const FloatType driftLength = 0.0;
          const FloatType weight = 1.0;
          append(wirePos, driftLength, weight);
        }
      }

      void append(const CDCWireHitSegment& wireHits, bool usePosition  = false) {
        for (const CDCWireHit * ptrWireHit : wireHits) {
          if (not ptrWireHit) continue;
          const CDCWireHit& wireHit = *ptrWireHit;
          const Vector2D& wirePos = wireHit.getRefPos2D();
          const FloatType driftLength = 0.0;
          // Try out using weighting the observations by the squared drift length.
          // Reduces the distance measure to a relative measure counting din drift lengths.
          // Using the squared drift length makes the chi2 unitless.
          // Limited at the bottom by the nominal uncertainty of the drift time (as introduced in CDC::SimpleTDCCountTranslator)
          const FloatType pseudoVariance = (usePosition ? 0.0 : wireHit.getRefDriftLength() * wireHit.getRefDriftLength()) + SIMPLE_DRIFT_LENGTH_VARIANCE;
          const FloatType weight = 1 / pseudoVariance;
          append(wirePos, driftLength, weight);
        }
      }



#ifndef __CINT__
      /// Appends all wire positions of the hits in the legendre track hits. Always use position since there is no other mode as long as there are no right left passage information available.
      void append(const std::vector<TrackFinderCDCLegendre::TrackHit*>& legendreTrackHits,
                  bool usePosition  __attribute__((unused)) = false) {
        for (const TrackFinderCDCLegendre::TrackHit * ptrLegendreTrackHit : legendreTrackHits) {
          if (not ptrLegendreTrackHit) continue;
          const TrackFinderCDCLegendre::TrackHit& legendreTrackHit = *ptrLegendreTrackHit;
          const TVector3&& wirePos = legendreTrackHit.getWirePosition();
          const FloatType driftLength = 0.0;
          const FloatType weight = 1.0;
          append(wirePos.X(), wirePos.Y(), driftLength, weight);
        }
      }
#endif
      /// Get the postion of the first observation.
      Vector2D getFrontPos2D() const
      { return empty() ? Vector2D() : Vector2D(getX(0), getY(0)); }

      /// Get the postion of the first observation.
      Vector2D getBackPos2D() const
      { return empty() ? Vector2D() : Vector2D(getX(size() - 1), getY(size() - 1)); }

      /// Calculate the total transvers travel distance traversed by these observations comparing the travel distance of first and last position
      FloatType getTotalPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpSBetween(getFrontPos2D(), getBackPos2D()); }

      /// Checks if the last position of these observations lies at greater travel distance than the first
      bool isForwardTrajectory(const CDCTrajectory2D& trajectory2D) const
      { return getTotalPerpS(trajectory2D) > 0.0; }

      /// Checks if the last observation in the vector lies greater or lower travel distance than the last observation.
      /** Returns:
       *  * FORWARD if the last observation lies behind the first.
       *  * BACKWARD if the last observation lies before the first.
       */
      ForwardBackwardInfo isCoaligned(const CDCTrajectory2D& trajectory2D) const
      { return sign(getTotalPerpS(trajectory2D)); }




      /// Extracts the observation center that is at the index in the middle.
      Vector2D getCentralPoint() const;

      /// Moves all observations passively such that the given vector becomes to origin of the new coordinate system
      void passiveMoveBy(const Vector2D& origin);

      /// Picks one observation as a reference point and transform all observations to that new origin
      Vector2D centralize();

      /// Returns the number of observations having a drift radius radius
      size_t getNObservationsWithDriftRadius() const;

#ifndef __CINT__
      //Hide this methods from CINT since it does not like the Eigen Library to much
      /// Returns the observations structured as an Eigen matrix
      /** This returns a reference to the stored observations. Note that operations may alter the content of the underlying memory and render it useless for subceeding calculations.*/
      EigenObservationMatrix getObservationMatrix();

      /// Constructs a symmetric matrix of weighted sums of x, y, r^2 and drift lengts as relevant for circle fits.
      /** Cumulates weights, x positions, y positions, quadratic polar radii and signed drift legnths and products thereof
       *  Returns symmetric matrix s with the following:
       *  * \f$ s_{00} = \sum w \f$
       *  * \f$ s_{01} = \sum x * w \f$
       *  * \f$ s_{02} = \sum y * w \f$
       *  * \f$ s_{03} = \sum (r^2 - l^2) * w \f$
       *  * \f$ s_{04} = \sum l * w \f$
       *
       *  * \f$ s_{11} = \sum x * x * w \f$
       *  * \f$ s_{12} = \sum x * y * w \f$
       *  * \f$ s_{13} = \sum x * (r^2 - l^2) * w \f$
       *  * \f$ s_{14} = \sum x * l * w \f$
       *
       *  * \f$ s_{22} = \sum y * y * w \f$
       *  * \f$ s_{23} = \sum y * (r^2 - l^2) * w \f$
       *  * \f$ s_{24} = \sum y * l * w \f$
       *
       *  * \f$ s_{33} = \sum (r^2 - l^2) * (r^2 - l^2) * w \f$
       *  * \f$ s_{34} = \sum (r^2 - l^2) * l * w \f$

       *  * \f$ s_{44} = \sum l * l * w \f$
       *  * + symmetric.
       */
      Eigen::Matrix<FloatType, 5, 5> getWXYRLSumMatrix();

      /// Constructs a symmetric matrix of weighted sums of x, y and drift lengts as relevant for line fits.
      /** Cumulates weights, x positions, y positions and signed drift legnths and products thereof
       *  Returns symmetric matrix s with the following:
       *  * \f$ s_{00} = \sum w \f$
       *  * \f$ s_{01} = \sum x * w \f$
       *  * \f$ s_{02} = \sum y * w \f$
       *  * \f$ s_{03} = \sum l * w \f$
       *
       *  * \f$ s_{11} = \sum x * x * w \f$
       *  * \f$ s_{12} = \sum x * y * w \f$
       *  * \f$ s_{13} = \sum x * l * w \f$
       *
       *  * \f$ s_{22} = \sum y * y * w \f$
       *  * \f$ s_{23} = \sum y * l * w \f$
       *
       *  * \f$ s_{33} = \sum (r^2 - l^2) * l * w \f$
       *  * + symmetric
       */
      Eigen::Matrix<FloatType, 4, 4> getWXYLSumMatrix();

      /// Constructs a symmetric matrix of weighted sums of x, y, r^2 as relevant for circle fits.
      /** Cumulates weights, x positions, y positions, quadratic polar radii and products thereof
       *  Returns symmetric matrix s with the following:
       *  * \f$ s_{00} = \sum w \f$
       *  * \f$ s_{01} = \sum x * w \f$
       *  * \f$ s_{02} = \sum y * w \f$
       *  * \f$ s_{03} = \sum (r^2 - l^2) * w \f$
       *
       *  * \f$ s_{11} = \sum x * x * w \f$
       *  * \f$ s_{12} = \sum x * y * w \f$
       *  * \f$ s_{13} = \sum x * (r^2 - l^2) * w \f$
       *
       *  * \f$ s_{22} = \sum y * y * w \f$
       *  * \f$ s_{23} = \sum y * (r^2 - l^2) * w \f$
       *  * \f$ s_{24} = \sum y * l * w \f$
       *
       *  * \f$ s_{33} = \sum (r^2 - l^2) * (r^2 - l^2) * w \f$
       *  * + symmetric.
       */
      Eigen::Matrix<FloatType, 4, 4> getWXYRSumMatrix();


      /// Constructs a symmetric matrix of weighted sums of x, y as relevant for line fits.
      /** Cumulates weights, x positions, y positions and products thereof
       *  Returns symmetric matrix s with the following:
       *  * \f$ s_{00} = \sum w \f$
       *  * \f$ s_{01} = \sum x * w \f$
       *  * \f$ s_{02} = \sum y * w \f$
       *
       *  * \f$ s_{11} = \sum x * x * w \f$
       *  * \f$ s_{12} = \sum x * y * w \f$
       *
       *  * \f$ s_{22} = \sum y * y * w \f$
       *  * + symmetric.
       */
      Eigen::Matrix<FloatType, 3, 3> getWXYSumMatrix();

#endif

    private:
      std::vector<FloatType> m_observations; ///< Memory for the individual observations. Arrangement of values is x,y, drift raduis, weight, x, y, .....

      /** ROOT Macro to make CDCObservation2D a ROOT class.*/
      ClassDefInCDCLocalTracking(CDCObservations2D, 1);

    }; //class

  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCOBSERVATIONS2D_H
