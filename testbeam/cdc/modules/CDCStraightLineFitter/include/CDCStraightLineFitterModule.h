/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCSTRAIGHTLINEFITMODULE_H
#define CDCSTRAIGHTLINEFITMODULE_H

#include <framework/core/Module.h>
#include <string>

#include <genfit/TrackCand.h>
#include <mdst/dataobjects/MCParticle.h>
#include <reconstruction/dataobjects/DedxTrack.h>
#include <TVector3.h>
#include <TRotation.h>

namespace Belle2 {

  /**
   * Simple straight line fit (based on an old fortran code from HERA-B)
   * Input: track candidates from a track finder (genfit::TrackCand)
   * Output: fitted tracks (Track, TrackFitResult), dE/dx (dedxTracks),
   * extrapolation to TOP (ExtHit)
   */
  class CDCStraightLineFitterModule : public Module {

  public:

    /**
     * Constructor
     */
    CDCStraightLineFitterModule();

    /**
     * Destructor
     */
    virtual ~CDCStraightLineFitterModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    /**
     * Performs a fit of track candidate
     * @param trackCand track candidate
     * @return true on success
     */
    bool fitTrackCandidate(const genfit::TrackCand& trackCand);

    /**
     * Determines dE/dx (mean, truncated mean and error) and stores results to DedxTrack
     * @param trackCand track candidate
     * @param momFit fitted track momentum
     * @param posFit fitted track position
     * @param dedxTrack object to store dE/dx
     */
    bool setDedx(const genfit::TrackCand& trackCand,
                 const TVector3& momFit,
                 const TVector3& posFit,
                 DedxTrack* dedxTrack);

    /**
     * Returns Chi^2 of the fit
     * @return Chi^2
     */
    double getChi2();

    /**
     * Returns number of degrees of freedom of the fit
     * @return number of degrees of freedom
     */
    int getNDF();

    /**
     * Returns fitted momentum vector
     * @return momentum vector
     */
    TVector3 getMomentum();

    /**
     * Returns fitted track start point
     * @return start point
     */
    TVector3 getStartPoint();

    /**
     * Returns fitted track end point
     * @return end point
     */
    TVector3 getEndPoint();

    /**
     * Returns rotation matrix to rotate back to Belle II frame
     * @return rotation matrix
     */
    TRotation m_rotateBack;

    /**
     * Sets start time using trigger information
     * (currently using simHits of two trigger scintillators)
     */
    void setStartTime();

    /**
     * Returns MC truth of the track, using relation weights of CDCHitsToMCParticles
     * @param hitIDs a vector of CDCHits array indices (from track candidate)
     * @param fraction minimal fraction of hits of MCParticle to assign it to track
     * @return pointer to MCParticle which contributes majority of hits or NULL
     */
    const MCParticle* getMCTruth(const std::vector<int>& hitIDs, double fraction = 0.7);

    /**
     * Checks whether two points are on the same side of the trigger counters
     * @param point1 first point
     * @param point2 second point
     * @return true if points are on the same side
     */
    bool onTheSameSide(const TVector3& point1, const TVector3& point2);

    double m_alpha;     /**< rotation angle of trigger counters (around z-axis) */
    double m_momentumEstimate;  /**< momentum estimate */
    double m_smearDriftLength; /**< sigma for additional drift length smearing */
    double m_removeLowest;  /**< portion of lowest dE/dx measurements to discard */
    double m_removeHighest; /**< portion of highest dE/dx measurements to discard */
    double m_smearDedx;  /**< relative sigma to smear individual dEdx measurements */

    double m_startTime; /**< start time given by trigger scintillators */

  };

} // Belle2 namespace

#endif
