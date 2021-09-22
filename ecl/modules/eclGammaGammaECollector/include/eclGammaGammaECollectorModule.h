/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//Calibration
#include <calibration/CalibrationCollectorModule.h>

//Framework
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  class ECLCrystalCalib;
  class TRGSummary;
  class EventMetaData;
  class Track;
  class ECLCluster;
  class ECLDigit;

  /** Calibration collector module that uses e+e- --> gamma gamma to do ECL single crystal energy calibration */
  class eclGammaGammaECollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor: Sets the description, the properties and the parameters of the module */
    eclGammaGammaECollectorModule();

    /** Define histograms and read payloads from DB */
    void prepare() override;

    /** Select events and crystals and accumulate histograms */
    void collect() override;

  private:
    /** Parameters to control the job */
    double m_thetaLabMinDeg{0.}; /**< miniumum photon theta in lab (0 degrees) */
    double m_thetaLabMaxDeg{180.}; /**< maximum photon theta in lab (180 degrees) */
    double m_minPairMass; /**< minimum invariant mass of the pair of photons (9 GeV/c^2) */
    double m_mindPhi; /**< minimum delta phi between clusters (179 deg) */
    double m_maxTime; /**< maximum photon (time - <t>)/dt99 (1) */
    bool m_measureTrueEnergy; /**< use eclCalDigit to determine MC deposited energy (false) */
    bool m_requireL1; /**< require events to satisfy a level 1 trigger (true) */

    /** Required arrays */
    StoreArray<Track> m_trackArray; /**< Required input array of Tracks */
    StoreArray<ECLCluster> m_eclClusterArray; /**< Required input array of ECLClusters */
    StoreArray<ECLDigit> m_eclDigitArray; /**< Required input array of ECLDigits */
    StoreObjPtr<EventMetaData> m_evtMetaData; /**< dataStore EventMetaData */
    StoreObjPtr<TRGSummary> m_TRGResults; /**< dataStore TRGSummary */


    /** Some other useful quantities */
    double thetaLabMin{ -1.0};  /**< m_thetaLabMinDeg converted to radians (conversion in Module::init) */
    double thetaLabMax{ -1.0};  /**< m_thetaLabMaxDeg converted to radians (coneversion in Module::init) */
    bool storeCalib = true; /**< force the input calibration constants to be saved first event */
    std::vector<float> EperCrys; /**< ECL digit energy for each crystal */

    double minTrkpt = 0.3; /**< (GeV/c) minimum pt of a good track */
    double maxZ0 = 4.; /**< (cm) maximum abs(Z0) of a good track */
    double maxD0 = 2.; /**< (cm) maximum abs(D0) of a good track */
    double minpValue = 0.001; /**< minimum p value of a good track */
    int minCDChits = 5; /**< minimum CDC hits for a good track */


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
