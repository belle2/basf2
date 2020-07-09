/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kota Nakagiri, Yuma Uematsu                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {

  namespace Variable {

    /**
     * returns number of hits associated to this track for given tracking detector
     *
     * If given particle is not created out of the Track, the return value is 0.
     */
    double v0DaughterTrackNHits(const Particle* part, const std::vector<double>& daughterID, const Const::EDetector& det);

    /**
     * returns the number of CDC hits associated to this track
     */
    double v0DaughterTrackNCDCHits(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the number of SVD hits associated to this track
     */
    double v0DaughterTrackNSVDHits(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the number of PXD hits associated to this track
     */
    double v0DaughterTrackNPXDHits(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the number of PXD and SVD hits associated to this track
     */
    double v0DaughterTrackNVXDHits(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the first activated SVD layer associated to the track
     */
    double v0DaughterTrackFirstSVDLayer(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the first activated PXD layer associated to the track
     */
    double v0DaughterTrackFirstPXDLayer(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the first activated CDC layer associated to the track
     */
    double v0DaughterTrackFirstCDCLayer(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the last CDC layer associated to the track
     */
    double v0DaughterTrackLastCDCLayer(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the fit P-value of the track
     */
    double v0DaughterTrackPValue(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the D0 impact parameter of the V0 daughter track
     */
    double v0DaughterTrackD0(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the tranverse momentum angle of the V0 daughter track
     */
    double v0DaughterTrackPhi0(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the curvature of the V0 daughter track
     */
    double v0DaughterTrackOmega(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the Z0 impact parameter of the V0 daughter track
     */
    double v0DaughterTrackZ0(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the slope of the V0 daughter track
     */
    double v0DaughterTrackTanLambda(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the d0 error of the track
     */
    double v0DaughterTrackD0Error(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the phi0 error of the track
     */
    double v0DaughterTrackPhi0Error(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the omega error of the track
     */
    double v0DaughterTrackOmegaError(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the z0 error of the track
     */
    double v0DaughterTrackZ0Error(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the tan(lambda) error of the track
     */
    double v0DaughterTrackTanLambdaError(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the track parameter Tau  0:d0, 1:phi0, 2:omega, 3:z0, 4:tanLambda
     */
    double v0DaughterTrackParam5AtIPPerigee(const Particle* part, const std::vector<double>& params);

    /**
     * returns an element of the 15 covariance matrix elements
     * (0,0), (0,1) ... (1,1), (1,2) ... (2,2) ...
     */
    double v0DaughterTrackParamCov5x5AtIPPerigee(const Particle* part, const std::vector<double>& params);

    /**
     * returns the pull of the helix parameter d0 with the V0 vertex as the track pivot.
     */
    double v0DaughterHelixWithVertexAsPivotD0Pull(const Particle* part, const std::vector<double>& params);

    /**
     * returns the pull of the helix parameter phi0 with the V0 vertex as the track pivot.
     */
    double v0DaughterHelixWithVertexAsPivotPhi0Pull(const Particle* part, const std::vector<double>& params);

    /**
     * returns the pull of the helix parameter omega with the V0 vertex as the track pivot.
     */
    double v0DaughterHelixWithVertexAsPivotOmegaPull(const Particle* part, const std::vector<double>& params);

    /**
     * returns the pull of the helix parameter z0 with the V0 vertex as the track pivot.
     */
    double v0DaughterHelixWithVertexAsPivotZ0Pull(const Particle* part, const std::vector<double>& params);

    /**
     * returns the pull of the helix parameter tan(lambda) with the V0 vertex as the track pivot.
     */
    double v0DaughterHelixWithVertexAsPivotTanLambdaPull(const Particle* part, const std::vector<double>& params);


    /** helper function to get track fit result from V0 object */
    TrackFitResult const* getTrackFitResultFromV0DaughterParticle(Particle const* particle, const double daughterID);

    /** helper function to get pull of the helix parameters of the V0 daughter tracks*/
    double getHelixParameterPullOfV0DaughterWithVertexAsPivotAtIndex(const Particle* particle, const double daughterID,
        const int tauIndex);


  }
} // Belle2 namespace

