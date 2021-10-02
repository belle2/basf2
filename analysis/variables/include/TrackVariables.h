/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    double trackNHits(const Particle* part, const Const::EDetector& det);

    /**
     * returns the number of CDC hits associated to this track
     */
    double trackNCDCHits(const Particle* part);

    /**
     * returns the number of SVD hits associated to this track
     */
    double trackNSVDHits(const Particle* part);

    /**
     * returns the number of PXD hits associated to this track
     */
    double trackNPXDHits(const Particle* part);

    /**
     * returns the number of PXD and SVD hits associated to this track
     */
    double trackNVXDHits(const Particle* part);

    /**
     * return NDF of the track fit which is equal to number of hits used in the fit (after outlier rejection) minus 5
     */
    double trackNDF(const Particle* part);

    /**
     * return chi2 of the track fit, which is computed based on the stored p-value and NDF
     */
    double trackChi2(const Particle* part);

    /**
     * returns the first activated SVD layer associated to the track
     */
    double trackFirstSVDLayer(const Particle* part);

    /**
     * returns the first activated PXD layer associated to the track
     */
    double trackFirstPXDLayer(const Particle* part);

    /**
     * returns the first activated CDC layer associated to the track
     */
    double trackFirstCDCLayer(const Particle* part);

    /**
     * returns the last CDC layer associated to the track
     */
    double trackLastCDCLayer(const Particle* part);

    /**
     * returns the track's D0 impact parameter
     */
    double trackD0(const Particle* part);

    /**
     * returns the track's transverse momentum angle
     */
    double trackPhi0(const Particle* part);

    /**
     * returns the track's curvature
     */
    double trackOmega(const Particle* part);

    /**
     * returns the track's Z0 impact parameter
     */
    double trackZ0(const Particle* part);

    /**
     * returns the track's slope
     */
    double trackTanLambda(const Particle* part);

    /**
     * returns the track's D0 impact parameter error
     */
    double trackD0Error(const Particle* part);

    /**
     * returns the track's transverse momentum angle error
     */
    double trackPhi0Error(const Particle* part);

    /**
     * returns the track's curvature error
     */
    double trackOmegaError(const Particle* part);

    /**
     * returns the track's Z0 impact parameter error
     */
    double trackZ0Error(const Particle* part);

    /**
     * returns the track's slope error
     */
    double trackTanLambdaError(const Particle* part);

    /**
     * returns the pValue of the track's fit
     */
    double trackPValue(const Particle* part);

    /**
     * returns the number of ECL clusters associated to the track
     * (should be 0 or 1 but in old ECL versions of reconstruction it can be > 1)
     */
    double trackNECLClusters(const Particle* part);

    /**
     * returns the number of track hits removed in V0Finder
     */
    double trackNRemovedHits(const Particle* part);

    /**
     * returns the number of CDC hits in the event not assigned to any track
     */
    double nExtraCDCHits(const Particle*);

    /*
     * returns the number of CDC hits in the event not assigned to any track
     * nor very likely beam background (i.e. hits that survive cleanup selection)
     */
    double nExtraCDCHitsPostCleaning(const Particle*);

    /*
     * checks for the presence of a non-assigned hit in the specified CDC layer
     */
    double hasExtraCDCHitsInLayer(const Particle*, const std::vector<double>& layer);

    /**
     * checks for the presence of a non-assigned hit in the specified CDC SuperLayer
     */
    double hasExtraCDCHitsInSuperLayer(const Particle*, const std::vector<double>& layer);

    /**
     * returns the number of segments that couldn't be assigned to any track
     */
    double nExtraCDCSegments(const Particle*);

    /**
     * returns the number of VXD hits not assigned to any track in the specified layer
     */
    double nExtraVXDHitsInLayer(const Particle*, const std::vector<double>& layer);

    /**
     * returns the number of VXD hits not assigned to any track
     */
    double nExtraVXDHits(const Particle*);

    /**
     * returns time of first SVD sample relative to event T0
     */
    double svdFirstSampleTime(const Particle*);

    /**
     * returns a flag set by the tracking if there is reason to assume there was
     * a track in the event missed by the tracking
     */
    double trackFindingFailureFlag(const Particle*);

    /**
    * returns extrapolated theta position based on helix parameters
    * parameters are the radius and z values to stop extrapolation
    */
    double trackHelixExtTheta(const Particle* part, const std::vector<double>& pars);

    /**
    * returns extrapolated phi position based on helix parameters
    * parameters are the radius and z values to stop extrapolation
    */
    double trackHelixExtPhi(const Particle* part, const std::vector<double>& pars);

    /**
     * returns the PDG code of the track fit hypothesis actually used for the particle
     */
    double trackFitHypothesisPDG(const Particle* part);

    /** mc-meas/err_meas for the respective helix parameter for the given particle */
    double getHelixD0Pull(const Particle* part);

    /** mc-meas/err_meas for the respective helix parameter for the given particle */
    double getHelixPhi0Pull(const Particle* part);

    /** mc-meas/err_meas for the respective helix parameter for the given particle */
    double getHelixOmegaPull(const Particle* part);

    /** mc-meas/err_meas for the respective helix parameter for the given particle */
    double getHelixZ0Pull(const Particle* part);

    /** mc-meas/err_meas for the respective helix parameter for the given particle */
    double getHelixTanLambdaPull(const Particle* part);

    /** helper function to get the position on the Helix */
    TVector3 getPositionOnHelix(const Particle* part, const std::vector<double>& pars);
  }
} // Belle2 namespace

