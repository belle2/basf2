/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Nils Braun                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>

#include <list>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    class TrackHit;

    /** Class for track candidates after CDC pattern recognition. */
    class TrackCandidate {
    public:

      /** Enum for charge hypotheses of track.*/
      enum ChargeHypotheses {
        charge_positive = 1, /**< Enum value positive charge. */
        charge_negative = -1, /**< Enum value negative charge. */
        charge_two_tracks = 2, /**< Enum value two tracks with the same values of r and theta charge. */
        charge_curler = 3, /**< Enum value curler (d < r(CDC). */
        charge_tracklet = 4
      };

      /** Copy Constructor.
       * Creates a new track from a copy of another track.
       */
      TrackCandidate(TrackCandidate& candidate);

      /** Construct track candidate using information from CDCLegendreQuadTree nodes */
      explicit TrackCandidate(const std::vector<QuadTreeLegendre*>& nodeList);

      /**Construct a Track Candidate from give theta and r value (in conformal plane)
       * Do only use for testing purposes!
       * @param theta theta value of track
       * @param r r value of track
       * @param charge charge assumption of track, defined by enum ChargeHypotheses
       * @param trackHitList List of Hits, which are assigned to the track
       */
      TrackCandidate(double theta, double r, int charge,
                     const std::vector<TrackHit*>& trackHitList);


      /**
       * @brief Get charge assumption based on the provided Hits
       * @param theta theta value of the potential track candidate
       * @param r r value of the potential track candidate
       * @param trackHits list of track hits, which is used to determine the charge assumption
       * @return int, base on the enum ChargeHypotheses
       * Determines the charge assumption of a potential track candidate, using its curvature with respect to all hits. A voting system is used to separate single and two tracks
       */
      static int getChargeAssumption(
        double theta, double r, const std::vector<TrackHit*>& trackHits);


      /** Return vector of assigned hits.*/
      inline std::vector<TrackHit*>& getTrackHits()
      {
        return m_TrackHits;
      }

      /**
       * Return theta value of track.
       * Important: this theta angle is value given by Legendre finding, not an azimuthal (phi) angle; mainly used in estimation of center of track trajectory
       */
      inline double getTheta() const
      {
        return m_theta;
      }

      /**
       * Return r value of track.
       * Important: this r value is signed curvature of track; mainly used in estimation of center of track trajectory
       */
      inline double getR() const
      {
        return m_r;
      }

      /** Return radius of track trajectory */
      inline double getRadius() const
      {
        return fabs(1. / m_r);
      }

      /** convert rho (one of the axis in legendre phase-space) to Pt (in GeV) */
      static double convertRhoToPt(double rho) {return 1.5 * 0.00299792458 / fabs(rho); };

      /** Pt (in GeV)convert  to rho (one of the axis in legendre phase-space) */
      static double convertPtToRho(double pt) {return 1.5 * 0.00299792458 / fabs(pt); };

      /** Return Xc value of track - X projection of track's trajectory center.*/
      inline double getXc() const
      {
        return m_xc;
      }

      /** Return Yc value of track - Y projection of track's trajectory center.*/
      inline double getYc() const
      {
        return m_yc;
      }

      /**
       * Return charge hypotheses of track.
       * Might also be curler or two tracks.
       */
      inline int getCharge() const
      {
        return m_charge;
      }

      /** Reestimate charge sign */
      void reestimateCharge()
      {
        m_charge = getChargeAssumption(getTheta(), getR(), getTrackHits());
      }


      /** Return charge sign of track.
       * Sure to be 1 or -1
       */
      int getChargeSign() const;

      /** Return number of assigned hits.*/
      inline unsigned int getNHits() const
      {
        return m_TrackHits.size();
      }

      /** Return momentum estimation of the track.*/
      Vector2D getMomentumEstimation();

      /** Setter for m_r, adopts m_xc and m_yc.*/
      void setR(double r)
      {
        m_r = r;
        m_xc = cos(m_theta) / r + m_ref_x;
        m_yc = sin(m_theta) / r + m_ref_y;
      }

      /** Setter for m_theta, adopts m_xc and m_yc.*/
      void setTheta(double theta)
      {
        m_theta = theta;
        m_xc = cos(theta) / m_r + m_ref_x;
        m_yc = sin(theta) / m_r + m_ref_y;
      }

      /** Adds a hit to the trackHitVector.*/
      void addHit(TrackHit* hit);

      /**
       * set reference point, with respect to which track was found
       */
      void setReferencePoint(double x0, double y0)
      {
        m_ref_x = x0;
        m_ref_y = y0;
        m_xc = cos(m_theta) / m_r + m_ref_x;
        m_yc = sin(m_theta) / m_r + m_ref_y;
      }

      /**
       * Sort hits by their cylindrical R information
       */
      void sortHitsByR()
      {
        std::sort(m_TrackHits.begin(), m_TrackHits.end(), [](TrackHit * hit1, TrackHit * hit2) {
          return hit1->getWirePosition().Mag2() < hit2->getWirePosition().Mag2();
        });
      }

      /**
       * set reference point, with respect to which track was found
       */
      TVector3 getReferencePoint() const {return TVector3(m_ref_x, m_ref_y, 0);}

    private:
      std::vector<TrackHit*> m_TrackHits; /**< vector to store TrackCandidateHits belonging to this TrackCandidate */

      double m_theta; /**< theta_value of the track candidate, given by Legendre track finding*/
      double m_r; /**< r_value of the track candidate, given by Legendre track finding*/
      double m_xc; /**< xc value in conformal plane*/
      double m_yc; /**< yc value in conformal plane*/
      double m_ref_x; /**< xc value in conformal plane*/
      double m_ref_y; /**< yc value in conformal plane*/
      int m_charge; /**< charge assumption of track*/



      friend class CDCLegendreTestFixture;

      static constexpr double curlerRSplitValue = 56.5;

    }; //end class CDCLegendreTrackCandidate
  }
}//end namespace Belle2
