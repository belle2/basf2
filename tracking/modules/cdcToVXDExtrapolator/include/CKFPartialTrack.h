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
  class CKFPartialTrack : public genfit::Track {
  public:
    CKFPartialTrack(const genfit::Track& other) :
      genfit::Track(other),
      holes(0),
      current_holes(0),
      hits(0),
      lowest_chi2(1e9)
    {}
    CKFPartialTrack(const CKFPartialTrack& other) :
      genfit::Track(other),
      holes(other.holes),
      current_holes(other.current_holes),
      hits(other.hits),
      lowest_chi2(other.lowest_chi2)
    {}

    unsigned addedHits() { return hits; }
    unsigned trueHoles() { return holes; }
    unsigned totalHoles() { return holes + current_holes; }
    unsigned currentHoles() { return current_holes; }
    double lowestChi2() { return lowest_chi2; }

    void addHit(double chi2 = 1e9) { holes += current_holes; current_holes = 0; hits++; if (chi2 < lowest_chi2) lowest_chi2 = chi2; }
    void addHole() { current_holes++; }

  private:
    unsigned holes;
    unsigned current_holes;
    unsigned hits;

    double lowest_chi2;
  };
}
