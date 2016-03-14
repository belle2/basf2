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

#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>
#include <mdst/dataobjects/HitPatternCDC.h>

#include <TVector3.h>
#include <TVector2.h>

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

      /** Enum for track candidate type */
      enum CandidateType {
        goodTrack = 1, /**< passed through all superlayers */
        tracklet = 2, /**< some kind of cluster somewhere in CDC, should be merged with other tracklets or tracks */
        curlerTrack = 3, /**< starts at 1st superlayer, but not reach 9th */
      };

      /** Copy Constructor.
       * Creates a new track from a copy of another track.
       */
      TrackCandidate(TrackCandidate& candidate);

      /** Construct track candidate using information from CDCLegendreQuadTree nodes */
      TrackCandidate(const std::vector<QuadTreeLegendre*>& nodeList);

      /** Destructor. */
      virtual ~TrackCandidate();

      /**Construct a Track Candidate from give theta and r value (in conformal plane)
       * @param theta theta value of track
       * @param r r value of track
       * @param charge charge assumption of track, defined by enum ChargeHypotheses
       * @param trackHitList List of Hits, which are assigned to the track
       */
      TrackCandidate(double theta, double r, int charge,
                     const std::vector<TrackHit*>& trackHitList);

      /** Return vector of assigned hits.*/
      inline std::vector<TrackHit*>& getTrackHits()
      {
        return m_TrackHits;
      }

      /** Return pattern of assigned axial and stereo hits.*/
      inline HitPatternCDC getHitPattern() const
      {
        return m_hitPattern;
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

      /** Return pT of the track candidate */
      inline double getPt() const
      {
        return fabs(1 / m_r) * 1.5 * 0.00299792458;
      }

      /** Return azimuthal angle of the track (phi) */
      double getPhi() const
      {
        return (m_r > 0.) ? (m_theta + m_PI * getChargeSign() / 2.) : (m_theta + m_PI * getChargeSign() / 2. + m_PI);
      }

      /** convert rho (one of the axis in legendre phase-space) to Pt (in GeV) */
      static double convertRhoToPt(double rho) {return 1.5 * 0.00299792458 / rho; };

      /** convert rho (one of the axis in legendre phase-space) to Pt (in GeV) */
      static double convertPtToRho(double Pt) {return 1.5 * 0.00299792458 / Pt; };

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
        m_charge = getChargeAssumption(m_theta, m_r, m_TrackHits);
      }


      /** Return charge sign of track.
       * Sure to be 1 or -1
       */
      int getChargeSign() const;

      /** Return number of assigned hits.*/
      inline int getNHits() const
      {
        return static_cast<int>(m_TrackHits.size());
      }

      /** Return number of assigned axial hits.*/
      inline int getNAxialHits() const
      {
        return m_axialHits;
      }

      /** Return number of assigned stereo hits.*/
      inline int getNStereoHits() const
      {
        return m_stereoHits;
      }

      /** Return momentum estimation of the track.*/
      TVector3 getMomentumEstimation(bool force_calculation = false) const;

      /** Setter for m_r, adopts m_xc and m_yc.*/
      void setR(double r);

      /** Setter for m_theta, adopts m_xc and m_yc.*/
      void setTheta(double theta);

      /** Adds a hit to the trackHitVector.*/
      void addHit(TrackHit* hit);

      /** Adds a hit to the trackHitVector.*/
      void removeHit(TrackHit* hit);

      /** Calculate distance to a given track hit.*/
      double DistanceTo(const TrackHit&) const;

      /** Calculate distance to a given track hit (before finding of z-position).*/
      double OriginalDistanceTo(const TrackHit&) const;

      /**
       * @brief Calculate distance of a given track to a given track hit.
       * Static function, so parameters of track needs to provided.
       * Also calculation of original distance (before finding of z-position) is possible
       * @param xc Xc value of track
       * @param yx Yc value of track
       * @param orig flag for calculation of original distance
       */
      static double DistanceTo(double xc, double yc, const TrackHit&,
                               bool orig = false);

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

      /**
       * @brief return SLayer ID of the contributing axial hit with the small layer ID.
       * @param forced allows to redefine innermost SLayer ID (especially after changes in hit pattern)
       * @param minNhits allows to change minimal number of hits required in innermost SLayer
       */
      int getInnermostSLayer(bool forced = false, int minNHits = 2);

      /**
       * @brief return SLayer ID of the contributing axial hit with the largest layer ID.
       * @param forced allows to redefine outermost SLayer ID (especially after changes in hit pattern)
       * @param minNhits allows to change minimal number of hits required in outermost SLayer
        */
      int getOutermostSLayer(bool forced = false, int minNHits = 2);

      /**
       * set reference point, with respect to which track was found
       */
      void setReferencePoint(double x0, double y0);

      /**
       * set reference point, with respect to which track was found
       */
      TVector3 getReferencePoint() const {return TVector3(m_ref_x, m_ref_y, 0);}

      /**
       *  Returns a weight, calculated from the contributing hits.
       *  Quite useless right now, but is an example how quality cuts based on information from contributing hits could look like.
       */
      double getLayerWaight();

      /**  Checks the contributing hits and removes stereo hits, that do not fit to the rest of the track */
      void CheckStereoHits();

      /** Remove "bad" hits */
      void clearBadHits();

      /** set chi2 after fitting */
      void setChi2(double chi2)
      {
        m_chi2 = chi2;
      }

      /** Get chi2 square of circular fit */
      double getChi2() const
      {
        return m_chi2;
      }

      /** set type of the track */
      void setCandidateType(int type) {m_type = type;};

      /** get type of the track (see enum CandidateType) */
      inline int getCandidateType() const {return m_type;};


    private:


      std::vector<TrackHit*> m_TrackHits; /**< vector to store TrackCandidateHits belonging to this TrackCandidate */
      std::vector<QuadTreeLegendre*> m_nodes; /**< vector to store nodes containing hits which belong to the candidate */

      double m_theta; /**< theta_value of the track candidate, given by Legendre track finding*/
      double m_r; /**< r_value of the track candidate, given by Legendre track finding*/
      double m_xc; /**< xc value in conformal plane*/
      double m_yc; /**< yc value in conformal plane*/
      double m_ref_x; /**< xc value in conformal plane*/
      double m_ref_y; /**< yc value in conformal plane*/
      int m_charge; /**< charge assumption of track*/
      int m_type; /**< type of the track*/

      int m_axialHits; /**< Number of axial hits, belonging to the track*/
      int m_stereoHits; /**< Number of stereo hits, belonging to the track*/

      bool m_calcedMomentum; /**< Is the momentum estimation already calculated?*/
      TVector3 m_momEstimation; /**< Momentum estimation*/

      HitPatternCDC m_hitPattern; /**< Efficient hit pattern builder; see HitPatternCDC description  */

      int m_innermostAxialSLayer; /**< Innermost axial superlayer */
      int m_outermostAxialSLayer; /**< Outermost axial superlayer */
//    int m_innermostAxialLayer; //Innermost axial layer;
//    int m_outermostAxialLayer; //Outermost axial layer;
      static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/

      double m_chi2; /**< Chi2 of track fitting */
      /**
       * Calculates the momentum estimation from parameters of the track (for x and y)
       * and from contributing hits (for z)
       */
      void calculateMomentumEstimation();

      /**
       * @brief Delivers an estimation of the z momentum of the track
       * @param mom x and y momentum, used to determine the absolute of the z momentum
       * Determines an estimate for the z momentum of the track, based on the position of the stereo hits. Only the first two stereo superlayers are taken into account. The actual value is based on the median of all values of the single track hits.
       */
      double getZMomentumEstimation(TVector2 mom2) const;

      /** Updates m_axialHits, m_stereoHits, and m_allHits */
      void determineHitNumbers();

      /** Return pattern of assigned axial and stereo hits. */
      void makeHitPattern();

    }; //end class CDCLegendreTrackCandidate
  }
}//end namespace Belle2
