/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCLEGENDRETRACKCANDIDATE_H
#define CDCLEGENDRETRACKCANDIDATE_H

#include "TVector3.h"
#include "TVector2.h"
#include <list>
#include <tracking/dataobjects/HitPatternCDC.h>

namespace Belle2 {

  class CDCLegendreTrackHit;

  /** Class for track candidates after CDC pattern recognition. */
  class CDCLegendreTrackCandidate {
  public:

    /** Enum for charge hypotheses of track.*/
    enum ChargeHypotheses {
      charge_positive = 1, /**< Enum value positive charge. */
      charge_negative = -1, /**< Enum value negative charge. */
      charge_two_tracks = 2, /**< Enum value two tracks with the same values of r and theta charge. */
      charge_curler = 4 /**< Enum value curler (d < r(CDC). */
    };

    /** Enum for track candidate type */
    enum CandidateType {
      fullTrack = 1, /**< passed through all superlayers */
      curlerTrack = 2, /**< starts at 1st superlayer, but not reach 9th */
      tracklet = 3, /**< some kind of cluster somewhere in CDC, should be merged with other tracklets or tracks */
    };

    /** Copy Constructor.
     * Creates a new track from a copy of another track.
     */
    CDCLegendreTrackCandidate(CDCLegendreTrackCandidate& candidate);

    /** Destructor. */
    ~CDCLegendreTrackCandidate();

    /**Construct a Track Candidate from give theta and r value (in conformal plane)
     * @param theta theta value of track
     * @param r r value of track
     * @param charge charge assumption of track, defined by enum ChargeHypotheses
     * @param trackHitList List of Hits, which are assigned to the track
     */
    CDCLegendreTrackCandidate(double theta, double r, int charge,
                              const std::vector<CDCLegendreTrackHit*>& trackHitList);

    /**Return vector of assigned hits.*/
    inline std::vector<Belle2::CDCLegendreTrackHit*> getTrackHits() {
      return m_TrackHits;
    }

    /**Return pattern of assigned axial hits.*/
    inline HitPatternCDC getHitPatternAxial() {
      return hitPatternAxial;
    }

    /**Return pattern of assigned stereo hits.*/
    inline HitPatternCDC getHitPatternStereo() {
      return hitPatternStereo;
    }

    /**Return pattern of assigned axial and stereo hits.*/
    inline HitPatternCDC getHitPattern() {
      return hitPattern;
    }

    /**
     * Check pattern of hits:
     * in between of innermost and outermost SLayers should be no empty SLayers;
     * argument of function allows define minimal number of axial hits in each SLayer
     * */
    bool checkHitPattern(int minNHitsSLayer = 2);

    /** Return theta value of track.*/
    inline double getTheta() const {
      return m_theta;
    }

    /** Return r value of track.*/
    inline double getR() const {
      return m_r;
    }

    /**Return Xc value of track.*/
    inline double getXc() const {
      return m_xc;
    }

    /**Return Yc value of track.*/
    inline double getYc() const {
      return m_yc;
    }

    /**Return charge hypotheses of track.
     * Might also be curler or two tracks.
     */
    inline int getCharge() const {
      return m_charge;
    }

    /** Return charge sign of track.
     * Sure to be 1 or -1 (1 for curlers)
     */
    int getChargeSign() const;

    /** Return number of assigned hits.*/
    inline int getNHits() const {
      return (m_stereoHits + m_axialHits);
    }

    /** Return number of assigned axial hits.*/
    inline int getNAxialHits() const {
      return m_axialHits;
    }

    /** Return number of assigned stereo hits.*/
    inline int getNStereoHits() const {
      return m_stereoHits;
    }

    /** Return momentum estimation of the track.*/
    TVector3 getMomentumEstimation(bool force_calculation = false) const;

    /** Setter for m_r, adopts m_xc and m_yc.*/
    void setR(double r);

    /** Setter for m_theta, adopts m_xc and m_yc.*/
    void setTheta(double theta);

    /** Adds a hit to the trackHitVector.*/
    void addHit(CDCLegendreTrackHit* hit);

    /** Calculate distance to a given track hit.*/
    double DistanceTo(const CDCLegendreTrackHit&) const;

    /** Calculate distance to a given track hit (before finding of z-position).*/
    double OriginalDistanceTo(const CDCLegendreTrackHit&) const;

    /**
     * @brief Calculate distance of a given track to a given track hit.
     * Static function, so parameters of track needs to provided.
     * Also calculation of original distance (before finding of z-position) is possible
     * @param xc Xc value of track
     * @param yx Yc value of track
     * @param orig flag for calculation of original distance
     */
    static double DistanceTo(double xc, double yc, const CDCLegendreTrackHit&,
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
      double theta, double r, const std::vector<CDCLegendreTrackHit*>& trackHits);

    /**
     * @brief return SLayer ID of the contributing axial hit with the small layer ID.
     * @param forced allows to redefine innermost SLayer ID (especially after changes in hit pattern)
     * @param minNhits allows to change minimal number of hits required in innermost SLayer
     */
    int getInnermostAxialSLayer(bool forced = false, int minNHits = 2);

    /**
     * @brief return SLayer ID of the contributing axial hit with the largest layer ID.
     * @param forced allows to redefine outermost SLayer ID (especially after changes in hit pattern)
     * @param minNhits allows to change minimal number of hits required in outermost SLayer
      */
    int getOutermostAxialSLayer(bool forced = false, int minNHits = 2);

    /**
     * set reference point, with respect to which track was found
     */
    void setReferencePoint(double x0, double y0);

    /**
     * set reference point, with respect to which track was found
     */
    TVector3 getReferencePoint() const {return TVector3(ref_x, ref_y, 0);}

    /**
     *  Returns a weight, calculated from the contributing hits.
     *  Quite useless right now, but is an example how quality cuts based on information from contributing hits could look like.
     */
    double getLayerWaight();

    /**
     * Checks the contributing hits and removes stereo hits, that do not fit to the rest of the track
     */
    void CheckStereoHits();

    /**
     * Returns type of candidate (see enum CandidateType)
     */
    int getCandidateType();


  private:

    /** Empty constructor. */
    CDCLegendreTrackCandidate() : m_charge(0) {};

    std::vector<CDCLegendreTrackHit*> m_TrackHits; /**< vector to store TrackCandidateHits belonging to this TrackCandidate */

    double m_theta; /**< theta_value of the track candidate, given by Legendre track finding*/
    double m_r; /**< r_value of the track candidate, given by Legendre track finding*/
    double m_xc; /**< xc value in conformal plane*/
    double m_yc; /**< yc value in conformal plane*/
    double ref_x; /**< xc value in conformal plane*/
    double ref_y; /**< yc value in conformal plane*/
    const int m_charge; /**< charge assumption of track*/

    int m_axialHits; /**< Number of axial hits, belonging to the track*/
    int m_stereoHits; /**< Number of stereo hits, belonging to the track*/

    bool m_calcedMomentum; /**< Is the momentum estimation already calculated?*/
    TVector3 m_momEstimation; /**< Momentum estimation*/

    HitPatternCDC hitPatternAxial; /**< Efficient hit pattern builder; see HitPatternCDC description  */
    HitPatternCDC hitPatternStereo; /**< Efficient hit pattern builder; see HitPatternCDC description  */
    HitPatternCDC hitPattern; /**< Efficient hit pattern builder; see HitPatternCDC description  */

    int innermostAxialSLayer; //Innermost axial superlayer;
    int outermostAxialSLayer; //Outermost axial superlayer;
    int innermostAxialLayer; //Innermost axial layer;
    int outermostAxialLayer; //Outermost axial layer;
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

    /**
     * Updates m_axialHits, m_stereoHits, and m_allHits
     */
    void DetermineHitNumbers();

    /**Return pattern of assigned axial and stereo hits.*/
    void makeHitPattern();

  };
//end class CDCLegendreTrackCandidate
}//end namespace Belle2

#endif //CDCLEGENDRETRACKCANDIDATE
