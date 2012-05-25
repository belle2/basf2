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

#include "TObject.h"
#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>

namespace Belle2 {

  class CDCLegendreTrackHit;

  /** Class for track candidates after CDC pattern recognition. */
  class CDCLegendreTrackCandidate: public TObject {
  public:

    /** Enum for charge hypotheses of track.*/
    enum ChargeHypotheses {
      charge_positive = 1,    /**< Enum value positive charge. */
      charge_negative = -1,   /**< Enum value negative charge. */
      charge_two_tracks = 2,  /**< Enum value two tracks with the same values of r and theta charge. */
      charge_curler = 4       /**< Enum value curler (d < r(CDC). */
    };

    /** Empty constructor. */
    CDCLegendreTrackCandidate();

    /** Copy Constructor.
     * Creates a new track from a copy of another track.
     */
    CDCLegendreTrackCandidate(CDCLegendreTrackCandidate& candidate);

    /** Destructor. */
    ~CDCLegendreTrackCandidate();

    /**Construct a Track Candidate from give theta and r value (in conformal plane)
     * @param id unique id of the track
     * @param theta theta value of track
     * @param r r value of track
     * @param charge charge assumption of track, defined by enum ChargeHypotheses
     * @param resolutionAxial maximal allowed distance of an axial hit
     * @param resolutionStereo maximal allowed distance of a stereo hit
     * @param assignHits controls, whether hits are assigned to track or not
     */
    CDCLegendreTrackCandidate(int id, double theta, double r, int charge,
                              std::string cdcHitsDatastoreName, double resolutionAxial, double resolutionStereo,
                              bool assignHits = true);

    /**Return list of assigned hits.*/
    std::vector<Belle2::CDCLegendreTrackHit*> getTrackHits() {
      return m_TrackHits;
    }

    /** Return (unique) trackID.*/
    int getID() const {
      return m_ID;
    }

    /** Return theta value of track.*/
    double getTheta() const {
      return m_theta;
    }

    /** Return r value of track.*/
    double getR() const {
      return m_r;
    }

    /** Return y0 value of track.*/
    double getY0() const {
      return m_y0;
    }

    /** Return slope value of track.*/
    double getSlope() const {
      return m_slope;
    }

    /**Return Xc value of track.*/
    double getXc() const {
      return 1 / (-1 * m_y0 / m_slope);
    }

    /**Return Yc value of track.*/
    double getYc() const {
      return 1 / m_y0;
    }

    /**Return charge hypotheses of track.
     * Might also be curler or two tracks.
     */
    int getCharge() const {
      return m_charge;
    }

    /** Return charge sign of track.
     * Sure to be 1 or -1 (1 for curler and two tracks).
     */
    int getChargeSign() const;

    /** Return number of unique hits, when the track was constructed.*/
    int getStartingUniqueHits() const {
      return m_uniqueHits;
    }

    /** Return momentum estimation of the track.*/
    TVector3 getMomentumEstimation() const;

    /** Calculate distance to a given track hit.*/
    double DistanceTo(const CDCLegendreTrackHit&) const;

    /** Calculate distance to a given track hit (before finding of z-position).*/
    double OriginalDistanceTo(const CDCLegendreTrackHit&) const;

    /** Calculate distance of a given track to a given track hit.
     * Static function, so parameters of track needs to provided.
     * Also calculation of original distance (before finding of z-position) is possible
     * @param xc Xc value of track
     * @param yx Yc value of track
     * @param orig flag for calculation of original distance
     */
    static double DistanceTo(double xc, double yc, const CDCLegendreTrackHit&, bool orig = false);

    /** Return charge estimation of track with given parameters.
     * @param cdcHitsDatastoreName Name of DataStore containing LegendreTrackHits
     * @param theta theta value of track
     * @param r r value of track
     * @return charge assumption, defined by enum ChargeHypotheses
     */
    static int getChargeAssumption(std::string cdcHitsDatastoreName,
                                   double theta, double r, double resolution);

  private:

    std::vector<CDCLegendreTrackHit*> m_TrackHits; /**< vector to store TrackCandidateHits belonging to this TrackCandidate */

    const double m_theta; /**< theta_value of the track candidate, given by Legendre track finding*/
    const double m_r; /**< r_value of the track candidate, given by Legendre track finding*/
    const double m_y0; /**< y-axis intercept in the conformal plane, calculated from theta and r*/
    const double m_slope; /**< slope in the conformal plane, calculated from theta and r*/
    const int m_charge; /**< charge assumption of track*/

    const int m_ID; /** track ID, unique for the event*/
    const bool m_assignHits; /**< flag for hit assignment (wheter they are assigned to the track or not).*/
    int m_uniqueHits; /**< Number of unique hits, when the track is contstructed.*/

    /**
     * Function to add axial hits to track.
     * @param cdcHitsDatastoreName name of DataStore containing LegendreTrackHits
     * @param resolution maximal allowed distance of a hit to the track
     */
    void AddAxialHits(std::string cdcHitsDatastoreName, double resolution);

    /**
     * Function to add stereo hits to track.
     * @param cdcHitsDatastoreName name of DataStore containing LegendreTrackHits
     * @param resolution maximal allowed distance of a hit to the track
     */
    void AddStereoHits(std::string cdcHitsDatastoreName, double resolution);

    double getZMomentumEstimation(TVector2 mom2) const;
    int getNUniqueHits() const;

    ClassDef(CDCLegendreTrackCandidate, 1) /** ROOT ClassDef macro to make this class a ROOT class.*/

  };
//end class CDCLegendreTrackCandidate
}//end namespace Belle2

#endif //CDCLEGENDRETRACKCANDIDATE
