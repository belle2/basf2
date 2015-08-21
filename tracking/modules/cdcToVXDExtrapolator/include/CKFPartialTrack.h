/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ian J. Watson                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <genfit/Track.h>

namespace genfit { class AbsMeasurement; }

namespace Belle2 {
  /**
   * Container for additional info needed to be kept with a potential
   * new track during a CKF run.
   */
  class CKFPartialTrack : public genfit::Track {
  public:
    /**
     * Construct a CKFPartialTrack (with cleared info fields) from an genfit::Track
     */
    CKFPartialTrack(const genfit::Track& other) :
      genfit::Track(other),
      holes(0),
      current_holes(0),
      hits(0),
      lowest_chi2(1e9)
    {}
    /**
     * Construct a CKFPartialTrack from an exisiting one
     */
    CKFPartialTrack(const CKFPartialTrack& other) :
      genfit::Track(other),
      holes(other.holes),
      current_holes(other.current_holes),
      hits(other.hits),
      lowest_chi2(other.lowest_chi2)
    {}

    /// number of hits added by the CKF
    unsigned addedHits() { return hits; }
    /// Number of "true" holes (holes which have a hit added after them)
    unsigned trueHoles() { return holes; }
    /// total number of holes ("true" holes plus holes at the end of the track not followed by hits (so far))
    unsigned totalHoles() { return holes + current_holes; }
    /// Number of holes between the current step and the last hit
    unsigned currentHoles() { return current_holes; }
    /// Chi2 value of the (CKF added) hit with the lowest chi2
    double lowestChi2() { return lowest_chi2; }

    /// Add the information for a new hit (not the actual hit, i.e. doesn't affect underlying genfit::Track)
    void addHit(double chi2 = 1e9) { holes += current_holes; current_holes = 0; hits++; if (chi2 < lowest_chi2) lowest_chi2 = chi2; }
    /// Add a hole to the information (doesn't change underlying genfit::Track)
    void addHole() { current_holes++; }

  private:
    unsigned holes;
    unsigned current_holes;
    unsigned hits;

    double lowest_chi2;
  };
}
