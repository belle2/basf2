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
     * returns the number of VTX hits associated to this track
     */
    double v0DaughterTrackNVTXHits(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the number of PXD, SVD and VTX hits associated to this track
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
     * returns the first activated VTX layer associated to the track
     */
    double v0DaughterTrackFirstVTXLayer(const Particle* part, const std::vector<double>& daughterID);

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
     * return the d0 impact parameter of a V0's daughter with daughterID index with the V0 vertex point as a pivot for the track
     */
    double v0DaughterD0(const Particle* particle, const std::vector<double>& daughterID);

    /**
     * return the difference between d0 impact parameters of V0's daughters with the V0 vertex point as a pivot for the track
     */
    double v0DaughterD0Diff(const Particle* particle);

    /**
     * return the z0 impact parameter of a V0's daughter with daughterID index with the V0 vertex point as a pivot for the track
     */
    double v0DaughterZ0(const Particle* particle, const std::vector<double>& daughterID);

    /**
     * return the difference between Z0 impact parameters of V0's daughters with the V0 vertex point as a pivot for the track
     */
    double v0DaughterZ0Diff(const Particle* particle);

    /**
     * returns the pull of the helix parameter d0 with the true V0 vertex as the track pivot.
     */
    double v0DaughterHelixWithTrueVertexAsPivotD0Pull(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the pull of the helix parameter phi0 with the true V0 vertex as the track pivot.
     */
    double v0DaughterHelixWithTrueVertexAsPivotPhi0Pull(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the pull of the helix parameter omega with the true V0 vertex as the track pivot.
     */
    double v0DaughterHelixWithTrueVertexAsPivotOmegaPull(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the pull of the helix parameter z0 with the true V0 vertex as the track pivot.
     */
    double v0DaughterHelixWithTrueVertexAsPivotZ0Pull(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the pull of the helix parameter tan(lambda) with the true V0 vertex as the track pivot.
     */
    double v0DaughterHelixWithTrueVertexAsPivotTanLambdaPull(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the pull of the helix parameter d0 with the origin as the track pivot.
     */
    double v0DaughterHelixWithOriginAsPivotD0Pull(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the pull of the helix parameter phi0 with the origin as the track pivot.
     */
    double v0DaughterHelixWithOriginAsPivotPhi0Pull(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the pull of the helix parameter omega with the origin as the track pivot.
     */
    double v0DaughterHelixWithOriginAsPivotOmegaPull(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the pull of the helix parameter z0 with the origin as the track pivot.
     */
    double v0DaughterHelixWithOriginAsPivotZ0Pull(const Particle* part, const std::vector<double>& daughterID);

    /**
     * returns the pull of the helix parameter tan(lambda) with the origin as the track pivot.
     */
    double v0DaughterHelixWithOriginAsPivotTanLambdaPull(const Particle* part, const std::vector<double>& daughterID);

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
     * returns the flag for V0 daughters sharing the innermost VXD hit.
     * 0x1(0x2) bit represents V/z(U/r-phi)-hit share.
     */
    int v0DaughtersShareInnermostHit(const Particle* part);

    /**
     * returns the flag for V0 daughters sharing the innermost VXD U-side hit.
     */
    bool v0DaughtersShareInnermostUHit(const Particle* part);

    /**
     * returns the flag for V0 daughters sharing the innermost VXD V-side hit.
     */
    bool v0DaughtersShareInnermostVHit(const Particle* part);

    /**
     * helper function to get the number of hits associated to a track for given tracking detector
     * If given particle is not created out of the Track, the return value is 0.
     * Not registered in variable manager
     */
    double getV0DaughterTrackDetNHits(const Particle* particle, const double daughterID, const Const::EDetector& det);

    /** helper function to get pull of the helix parameters of the V0 daughter tracks with the true V0 vertex as the pivot */
    double getHelixParameterPullOfV0DaughterWithTrueVertexAsPivotAtIndex(const Particle* particle, const double daughterID,
        const int tauIndex);

    /** helper function to do some basic sanity checks before calculating converted photon variables */
    int convertedPhotonErrorChecks(const Particle* gamma, const std::vector<double>& daughterIndices);

    /** helper function to load helix parameters for calculating converted photon variables */
    int convertedPhotonLoadHelixParams(const Particle* gamma, int daughterIndex1, int daughterIndex2, double& Phi01, double& D01,
                                       double& Omega1, double& Z01, double& TanLambda1, double& Phi02, double& D02, double& Omega2, double& Z02,
                                       double& TanLambda2);

    /**
     * returns the invariant-mass of the two-track system assuming it's a converted photon
     */
    double convertedPhotonInvariantMass(const Particle* gamma, const std::vector<double>& daughterIndices);

    /**
     * returns the discriminating variable Delta-TanLambda of the two-track system assuming it's a converted photon
     */
    double convertedPhotonDelTanLambda(const Particle* gamma, const std::vector<double>& daughterIndices);

    /**
     * returns the discriminating variable Delta-R of the two-track system assuming it's a converted photon
     */
    double convertedPhotonDelR(const Particle* gamma, const std::vector<double>& daughterIndices);

    /** helper function that returns the estimated Z-coordinates of the two helices at the vertex */
    std::pair<double, double> convertedPhotonZ1Z2(const Particle* gamma, const std::vector<double>& daughterIndices);

    /**
     * returns the discriminating variable Delta-Z of the two-track system assuming it's a converted photon
     */
    double convertedPhotonDelZ(const Particle* gamma, const std::vector<double>& daughterIndices);

    /**
     * returns the estimated Z-coordinate of the two-track system assuming it's a converted photon
     */
    double convertedPhotonZ(const Particle* gamma, const std::vector<double>& daughterIndices);

    /** helper function that returns the estimated vertex in the transverse plane */
    TVector2 convertedPhotonXY(const Particle* gamma, const std::vector<double>& daughterIndices);

    /**
     * returns the estimated X-coordinate of the two-track system assuming it's a converted photon
     */
    double convertedPhotonX(const Particle* gamma, const std::vector<double>& daughterIndices);

    /**
     * returns the estimated Y-coordinate of the two-track system assuming it's a converted photon
     */
    double convertedPhotonY(const Particle* gamma, const std::vector<double>& daughterIndices);

    /**
     * returns the estimated Rho of the two-track system assuming it's a converted photon
     */
    double convertedPhotonRho(const Particle* gamma, const std::vector<double>& daughterIndices);

    /**
     * helper function that returns the estimated 3-momentum of the two-track system assuming it's a converted photon
     */
    TVector3 convertedPhoton3Momentum(const Particle* gamma, const std::vector<double>& daughterIndices);

    /**
     * returns the estimated x-component of momentum of the two-track system assuming it's a converted photon
     */
    double convertedPhotonPx(const Particle* gamma, const std::vector<double>& daughterIndices);

    /**
     * returns the estimated y-component of momentum of the two-track system assuming it's a converted photon
     */
    double convertedPhotonPy(const Particle* gamma, const std::vector<double>& daughterIndices);

    /**
     * returns the estimated z-component of momentum of the two-track system assuming it's a converted photon
     */
    double convertedPhotonPz(const Particle* gamma, const std::vector<double>& daughterIndices);


  }
} // Belle2 namespace
