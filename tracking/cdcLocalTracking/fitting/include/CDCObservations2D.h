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
      typedef Eigen::Map< Eigen::Matrix< FloatType, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > > EigenObservationMatrix;
#endif

      /// Empty constructor.
      CDCObservations2D();

      /// Empty destructor.
      ~CDCObservations2D();

      /// Returns the number of observations stored
      size_t size() const
      { return  m_observations.size() / 3; }

      /// Removes all observations stored
      void clear()
      { m_observations.clear(); }

      /// Reserves enough space for nObservations
      void reserve(size_t nObservations)
      { m_observations.reserve(nObservations * 3); }

      /// Getter for the x value of the observation at the given index
      FloatType getX(const int iObservation) const
      { return m_observations[iObservation * 3]; }

      /// Getter for the y value of the observation at the  given index
      FloatType getY(const int iObservation) const
      { return m_observations[iObservation * 3 + 1]; }

      /// Getter for the signed drift radius of the observation at the  given index
      FloatType getSignedDriftLength(const int iObservation) const
      { return m_observations[iObservation * 3 + 2]; }


      /// Appends the observed position - drift radius is assumed to be zero
      void append(const Belle2::CDCLocalTracking::Vector2D& pos2D, const FloatType& signedRadius = 0.0) {
        m_observations.push_back(pos2D.x());
        m_observations.push_back(pos2D.y());
        m_observations.push_back(signedRadius);
      }

      /// Appends the observed position - drift radius is take a positiv number
      void append(const Belle2::CDCLocalTracking::CDCWireHit& wireHit)
      { append(wireHit.getRefPos2D(), wireHit.getRefDriftLength()); }

      /// Appends the observed position - drift radius is signed number according to the orientation
      void append(const Belle2::CDCLocalTracking::CDCRLWireHit& rlWireHit)
      { append(rlWireHit.getRefPos2D(), rlWireHit.getSignedRefDriftLength()); }

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
          append(recoHit2D.getRecoPos2D());
        } else {
          append(recoHit2D.getRLWireHit());
        }
      }

      /// Appends all reconstructed hits from the two dimensional segment, usePosition indicates whether the absolute position shall be used instead of the oriented wire hit information
      void append(const CDCRecoSegment2D& recoSegment2D, bool usePosition = false) {
        for (const CDCRecoHit2D & recoHit2D :  recoSegment2D) {
          append(recoHit2D, usePosition);
        }
      }

      /// Appends all reconstructed hits from the two axial segments, usePosition indicates whether the absolute position shall be used instead of the oriented wire hit information.
      void append(const CDCAxialAxialSegmentPair& axialAxialSegmentPair, bool usePosition = false) {
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


      /// Extracts the observation center that is at the index in the middle.
      Vector2D getCentralPoint() const;

      /// Moves all observations passively such that the given vector becomes to origin of the new coordinate system
      void centralize(const Vector2D& origin);

      /// Picks one observation as a reference point and transform all observations to that new origin
      Vector2D centralize();

      /// Returns the number of observations having a drift radius radius
      size_t getNObservationsWithDriftRadius() const;

#ifndef __CINT__
      //Hide this methods from CINT since it does not like the Eigen Library to much
      /// Returns the observations structured as an Eigen matrix
      /** This returns a reference to the stored observations. Note that operations may alter the content of the underlying memory and render it useless for subceeding calculations.*/
      EigenObservationMatrix getObservationMatrix();
#endif

    private:
      std::vector<FloatType> m_observations;

      /** ROOT Macro to make CDCObservation2D a ROOT class.*/
      ClassDefInCDCLocalTracking(CDCObservations2D, 1);

    }; //class

  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCOBSERVATIONS2D_H
