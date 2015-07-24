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

#include<vector>
#include<map>

namespace genfit { class Track; class AbsMeasurement; }

namespace Belle2 {

  /**
   * Extrapolate a track, collect compatible hits, update, filter, refit
   *
   * Given a track and a method to ask for compatible hits, the CKF
   * extrapolates the track to each of the hits, runs a Kalman update
   * to give a new track for each hit, filters out bad updates, and
   * repeats until no more hits are available. After this, it filters
   * out a best track (or tracks?) and does a full refit.
   *
   */
  class CKF {

  public:

    /**
     * Pass in an existing track to be fit, and a way to ask for new
     * hits. findHits should take in a track, and an integer
     * representing the number of steps already processed. data will
     * be passed unmolested to each invocation of findHits
     */
    CKF(genfit::Track* track, bool (*findHits)(genfit::Track*, unsigned, std::vector<genfit::AbsMeasurement*>&, void*), void* data,
        double _maxChi2Increment = 20, int _maxHoles = 3);

    /// find hits, run extrapolations, trim outputs, find a best track candidate
    genfit::Track* processTrack();

    double getChi2Inc(int hitId) {return chi2Map[hitId];}

  private:
    genfit::Track* seedTrack;
    bool (*findHits)(genfit::Track*, unsigned, std::vector<genfit::AbsMeasurement*>&, void*);

    /// refit track, assumes a TrackRep for the track already exists
    bool refitTrack(genfit::Track* track);

    /// finds the best track of all the candidates
    genfit::Track* bestTrack(std::vector<genfit::Track*>*);

    /// after adding an additional hit but before updating, check that the track passes some selections (e.g. check for holes)
    bool passPreUpdateTrim(genfit::Track*, unsigned step);
    /// after adding an additional hit and updating, check that the track passes some selections (e.g. check the updates chi2 increment)
    bool passPostUpdateTrim(genfit::Track*, unsigned step);

    /// data needed by the findHits functions
    void* data;

    /// how many findHits steps we've already taken
    unsigned step;

    /// maximum number of holes we allow in a track
    int maxHoles;
    /// maximum chi2 increment per added hit
    double maxChi2Increment;

    /// stores the chi2 increment of the added hit when it was added (i.e. not after refit)
    std::map<int, double> chi2Map;
  };
}
