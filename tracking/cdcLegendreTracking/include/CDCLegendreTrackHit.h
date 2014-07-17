/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include "TVector3.h"
#include <cdc/dataobjects/CDCHit.h>

namespace Belle2 {
  namespace TrackFinderCDCLegendre {

    class TrackCandidate;

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


      /** Constructor to create a CDCTrackHit from a CDCHit object.
       * Some member variables of CDCHit are copied and other to CDCTrackHit specific variables are initialized
       * (e.g. the position of the hit wire in normal space and in the conformal plane).
       */
      TrackHit(CDCHit* hit, int iHit);

      /** Copy constructor.
       *
       */
      TrackHit(const TrackHit& rhs);

      /* Destructor. */
      ~TrackHit();

      /** Check hit drift time; if it greater than distances between wires mark hit as bad*/
      bool checkHitDriftLength();

      /** Assigns the coordinates of the hit wire (from CDC Geometry database) and sets the wire vector.*/
      void setWirePosition();

      /** Assigns the Z coordinate of the hit wire and update XY coordinates*/
      void setZReference(double zReference);

      /** Assigns values for conformal coordinates by transforming the wire coordinates. */
      void performConformalTransformation();

      /** Return the index in the store array of the original CDCHit*/
      inline int getStoreIndex() const {return m_cdcHitIndex;}

      /** Returns the layerId (0 - 55). */
      inline int getLayerId() const {return m_layerId;};

      /** Returns the wireId (0 - 383, depends on layer). */
      inline int getWireId() const {return m_wireId;};

      /** Returns the superlayerId (0-8). */
      inline int getSuperlayerId() const {return m_superlayerId;};

      /** Returns the DriftTime.*/
      inline double getDriftLength() const {return m_driftLength;}

      /** Returns the Uncertainty of the Drift Time.*/
      inline double getSigmaDriftLength() const {return m_sigmaDriftLength;}

      /** Returns true for a Hit in an axial layer, false for a Hit in a stereo layer. */
      inline bool getIsAxial() const {return m_isAxial;};

      /** Returns the position of the center(!!!) of the wire. */
      inline TVector3 getWirePosition() const {return m_wirePosition ;}

      /** Returns the original wire position (before the finding of the z-position.*/
      inline TVector3 getOriginalWirePosition() const {return m_wirePositionOrig ;}

      /** Returns the Hit position (X coordinate) in the conformal plane.*/
      inline double getConformalX() const {return m_conformalX;};

      /** Returns the Hit position (Y coordinate) in the conformal plane.*/
      inline double getConformalY() const {return m_conformalY;};

      /** Returns the drift time in the conformal plane (with r << x,y).*/
      inline double getConformalDriftLength() const {return m_conformalDriftLength;};

      /** Returns the phi angle of the center wire position.
        * From the Hit position (= center of the wire) the angle is calculated so that it goes from 0 to 2*pi.
        */
      double getPhi() const;

      /** Return curvature sign with respect to a certain point in the conformal plain.*/
      int getCurvatureSignWrt(double xc, double yc) const;

      /** Finds the point of closest approach to a given track candidate adjusts the wire position accordingly.
       * @return Status of the approach. False if no nearest point could be found.
       * */
      bool approach(const TrackCandidate&);
      bool approach2(const TrackCandidate&);

      /** Sets the position of the hit to given point in 3D space.*/
      void setPosition(const TVector3 position) {
        m_wirePosition = position;
        performConformalTransformation();
      }

      /** Shows how hit was used */
      inline int getHitUsage() const {return m_hitUsage;};

      /** Sets how hit was used */
      void setHitUsage(int hitUsage) {m_hitUsage = hitUsage;};


    private:

      int m_cdcHitIndex;                 /**< ID of the original CDCHit in the store array*/
      int m_layerId;                     /**< ID of the layer of the hit (0-55)*/
      int m_wireId;                      /**< ID of the wire of the hit */
      int m_superlayerId;                /**< ID of the superlayer of the hit (0-8) (superlayer = group of 6/8 layers with the same orientation)*/
      double m_driftLength;              /**< Drift Length of the CDCHit */
      double m_sigmaDriftLength;         /**< Resolution of the drift Length*/
      unsigned short m_charge;                   /**< Charge deposit the CDCHit */

      bool m_isAxial;                    /**< Boolean to mark a Hit as belonging to an axial superlayer */

      TVector3 m_wirePosition;           /**< Coordinates of the center (!) of the hit wire. */
      double m_conformalX;               /**< X coordinate of the hit in the conformal plane*/
      double m_conformalY;               /**< Y coordinate of the hit in the conformal plane*/
      double m_conformalDriftLength;       /**< Drift time of the hit in the conformal plane (under the assumption that r << x,y*/

      TVector3 m_wirePositionOrig;           /**< Original Coordinates of the center (!) of the hit wire. */

      //    static constexpr double m_zReference = 25.852;  /**< Reference z position for wire position determination*/
      double m_zReference;  /**< Reference z position for wire position determination*/

      int m_hitUsage; /**< Indicates whether hit was used */

    }; //end class CDCTrackHit
  } //end namespace TrackFinderCDCLegendre
} //end namespace Belle2


