/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Sam Cunliffe, Martin Heck                   *
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
     * returns the first activated SVD layer associated to the track
     */
    double trackFirstSVDLayer(const Particle* part);

    /**
     * returns the first activated PXD layer associated to the track
     */
    double trackFirstPXDLayer(const Particle* part);

    /**
     * returns the track's D0 impact parameter
     */
    double trackD0(const Particle* part);

    /**
     * returns the track's Z0 impact parameter
     */
    double trackZ0(const Particle* part);

    /**
     * returns the track's D0 impact parameter error
     */
    double trackD0Error(const Particle* part);

    /**
     * returns the track's Z0 impact parameter error
     */
    double trackZ0Error(const Particle* part);

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
     * returns time of first SVD sample relatvie to event T0
     */
    double svdFirstSampleTime(const Particle*);

    /**
     * returns a flag set by the tracking if there is reason to assume there was
     * a track in the event missed by the tracking
     */
    double trackFindingFailureFlag(const Particle*);

  }
} // Belle2 namespace

