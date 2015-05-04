/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Nils Braun                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#ifndef __CINT__
#include <tuple>
#endif

#include <TVector3.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>

namespace Belle2 {
  class CDCHit;

  namespace TrackFindingCDC {

    class TrackCandidate;
    class CDCWireHit;

    /** CDC Hit Class used for pattern recognition in the Legendre plain. */
    class TrackHit {
    public:

      /**
       * Numeration used for marking hits
       */
      enum HitUsage {
        not_used = 0,
        used_in_track = 1,
        used_in_cand = 2,
        bad = 3,
        background = 4
      };

      /** For root: */
      TrackHit() : m_underlayingWireHit(nullptr), m_wirePosition(), m_conformalPosition(),
        m_conformalDriftLength(), m_zReference(0), m_hitUsage(HitUsage::not_used) { }

      /** Constructor to create a TrackHit from a CDCHit object.
       * Some member variables of CDCHit are copied and other to CDCTrackHit specific variables are initialized
       * (e.g. the position of the hit wire in normal space and in the conformal plane).
       * The parameter iHit is unused!
       */
      TrackHit(const CDCHit* hit, int iHit);

      /** Constructor to create a TrackHit from a CDCWireHit object.
       * Some member variables of CDCHit are copied and other to CDCTrackHit specific variables are initialized
       * (e.g. the position of the hit wire in normal space and in the conformal plane).
       */
      TrackHit(const CDCWireHit* wireHit)
      {
        initializeFromWireHit(wireHit);
      }

      /** Return the index in the store array of the original CDCHit*/
      inline int getStoreIndex() const { return m_underlayingWireHit->getStoreIHit(); }

      /** Returns the unique layer number. */
      inline int getLayerId() const { return m_underlayingWireHit->getWireID().getICLayer(); }

      /** Returns the wire number. */
      inline int getWireId() const { return m_underlayingWireHit->getWireID().getIWire(); }

      /** Returns the superlayer number (0-8). */
      inline int getSuperlayerId() const { return m_underlayingWireHit->getISuperLayer(); }

      /** Returns the DriftTime.*/
      inline double getDriftLength() const { return m_underlayingWireHit->getRefDriftLength(); }

      /** Returns the Uncertainty of the Drift Time.*/
      inline double getSigmaDriftLength() const { return m_underlayingWireHit->getRefDriftLengthVariance(); }

      /** Returns true for a Hit in an axial layer, false for a Hit in a stereo layer. */
      inline bool getIsAxial() const { return m_underlayingWireHit->getStereoType() == AXIAL; }

      /** Returns position of forward wire end */
      inline TVector3 getForwardWirePosition() const { return m_underlayingWireHit->getWire().getSkewLine().forward3D(); }

      /** Returns position of backward wire end */
      inline TVector3 getBackwardWirePosition() const { return m_underlayingWireHit->getWire().getSkewLine().backward3D(); }

      /** Returns the Hit position (X coordinate) in the conformal plane.*/
      inline double getConformalX() const { return m_conformalPosition.x(); }

      /** Returns the Hit position (Y coordinate) in the conformal plane.*/
      inline double getConformalY() const { return m_conformalPosition.y(); }

      /** Returns the drift time in the conformal plane (with r << x,y).*/
      inline double getConformalDriftLength() const { return m_conformalDriftLength; }

      /** Shows how hit was used */
      inline HitUsage getHitUsage() const { return m_hitUsage; }

      /** Sets how hit was used */
      void setHitUsage(HitUsage hitUsage) { m_hitUsage = hitUsage; }

      /** Get pointer to original CDCHit */
      const CDCHit* getOriginalCDCHit() const { return m_underlayingWireHit->getHit(); }

      const CDCWireHit* getUnderlayingCDCWireHit() const { return m_underlayingWireHit; }

      /** Get Z reference */
      double getZReference() const { return m_zReference; }

      /** Returns the phi angle of the center wire position.
        * From the Hit position (= center of the wire) the angle is calculated so that it goes from 0 to 2*pi.
        * With the cases:
        *   x > 0, y > 0: phi in 0, pi/2
        *   x < 0, y > 0: phi in pi/2, pi
        *   x < 0, y < 0: phi in pi, 3/2 pi
        *   x > 0, y < 0: phi in 3/2 pi, 2 pi
        */
      double getPhi() const;

      /** Return curvature sign with respect to a certain point in the conformal plain.*/
      int getCurvatureSignWrt(double xc, double yc) const;

      /** Finds the point of closest approach to a given track candidate adjusts the wire position accordingly.
       * @return Status of the approach. False if no nearest point could be found.
       * UNUSED IN THE MOMENT. DO NOT DELETE.
       * */
      bool approach(const TrackCandidate&);

      /** Finds the point of closest approach to a given track candidate adjusts the wire position accordingly.
       * @return Status of the approach. False if no nearest point could be found.
       * UNUSED IN THE MOMENT. DO NOT DELETE.
       * */
      bool approach2(const TrackCandidate&);

      /** Returns the position of the center(!!!) of the wire. */
      inline TVector3 getWirePosition() const { return m_wirePosition; }

      /** Returns the original wire position (before the finding of the z-position, so z = 0) */
      inline TVector3 getOriginalWirePosition() const
      {
        Vector2D referenceWirePosition = m_underlayingWireHit->getRefPos2D();
        return TVector3(referenceWirePosition.x(), referenceWirePosition.y(), 0);
      }

      /** Check hit drift time; if it greater than distances between wires mark hit as bad*/
      bool checkHitDriftLength();

      /** Assigns the Z coordinate of the hit wire and update XY coordinates*/
      void setZReference(double zReference);

#ifndef __CINT__
      /** Calculate conformal coordinates with respect to choosen point by transforming the wire coordinates. Returns (x',y',driftLength) */
      std::tuple<double, double, double> performConformalTransformWithRespectToPoint(double x0, double y0);
#endif


    private:
      /** Set all parameters from the given wire hit */
      void initializeFromWireHit(const CDCWireHit* wireHit);

      /** Assigns the coordinates of the hit wire (from CDC Geometry database) and sets the wire vector.*/
      void setWirePosition();

      /** Assigns values for conformal coordinates by transforming the wire coordinates. */
      void performConformalTransformation();

      const CDCWireHit*
      m_underlayingWireHit;  /**< The TrackHit is something like an adapter for the CDCWireHit class with some additional information */

      Vector3D m_wirePosition;           /**< Coordinates of the center (!) of the hit wire. */
      Vector2D m_conformalPosition;      /**< Position in the conformal space of the drift center (not the wire!) */
      double m_conformalDriftLength;     /**< Drift time of the hit in the conformal plane (under the assumption that r << x,y*/
      double m_zReference;               /**< Reference z position for wire position determination*/
      HitUsage m_hitUsage;               /**< Indicates whether hit was used */

    }; //end class CDCTrackHit
  } //end namespace TrackFindingCDC
} //end namespace Belle2


