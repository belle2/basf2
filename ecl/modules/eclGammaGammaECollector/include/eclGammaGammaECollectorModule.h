/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>
#include <ecl/geometry/ECLNeighbours.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>

namespace Belle2 {

  /** Calibration collector module that uses e+e- --> gamma gamma to do ECL single crystal energy calibration */
  class eclGammaGammaECollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor: Sets the description, the properties and the parameters of the module */
    eclGammaGammaECollectorModule();

    /** Define histograms and read payloads from DB */
    void prepare();

    /** Select events and crystals and accumulate histograms */
    void collect();

  private:

    /** Parameters to control the job */
    double m_thetaLabMinDeg; /**< miniumum photon theta in lab (0 degrees) */
    double m_thetaLabMaxDeg; /**< maximum photon theta in lab (180 degrees) */
    bool m_measureTrueEnergy; /**< use eclCalDigit to determine MC deposited energy (false) */
    bool m_requireL1; /**< require events to satisfy a level 1 trigger (true) */

    /** Required arrays */
    StoreArray<Track> TrackArray; /**< Required input array of Tracks */
    StoreArray<ECLCluster> eclClusterArray; /**< Required input array of ECLClusters */
    StoreArray<ECLCalDigit> eclCalDigitArray; /**< Required input array of ECLCalDigits */
    StoreArray<ECLDigit> eclDigitArray; /**< Required input array of ECLDigits */


    /** Some other useful quantities */
    double degPerRad; /**< convert rad to degrees */
    double thetaLabMin;  /**< m_thetaLabMinDeg converted to radians */
    double thetaLabMax;  /**< m_thetaLabMaxDeg converted to radians */
    int iEvent = 0; /**< event counter */
    std::vector<float> EperCrys; /**< ECL digit energy for each crystal */

    double minTrkpt = 0.3; /**< (GeV/c) minimum pt of a good track */
    double maxZ0 = 4.; /**< (cm) maximum abs(Z0) of a good track */
    double maxD0 = 2.; /**< (cm) maximum abs(D0) of a good track */
    double minpValue = 0.001; /**< minimum p value of a good track */
    int minCDChits = 5; /**< minimum CDC hits for a good track */

    double minPairMass = 9.; /**< (GeV/c^2) minimum invariant mass of the pair of photons */
    double mindPhi = 179.; /**< (deg) minimum delta phi between clusters */

    /** Expected energies from database */
    DBObjPtr<ECLCrystalCalib> m_ECLExpGammaGammaE;
    std::vector<float> ExpGammaGammaE; /**< vector obtained from DB object */

    /** Electronics calibration from database */
    DBObjPtr<ECLCrystalCalib> m_ElectronicsCalib;
    std::vector<float> ElectronicsCalib; /**< vector obtained from DB object */

    /** Existing single crystal calibration from DB; will be updated by CAF */
    DBObjPtr<ECLCrystalCalib> m_GammaGammaECalib;
    std::vector<float> GammaGammaECalib; /**< vector obtained from DB object */

  };
}

