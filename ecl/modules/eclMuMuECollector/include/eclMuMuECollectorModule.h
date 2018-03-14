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
#include <ecl/dataobjects/ECLDigit.h>

namespace Belle2 {

  /** Calibration collector module that uses muon pairs to do ECL single crystal energy calibration */
  class eclMuMuECollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor: Sets the description, the properties and the parameters of the module */
    eclMuMuECollectorModule();

    /** Define histograms and read payloads from DB */
    void prepare();

    /** Select events and crystals and accumulate histograms */
    void collect();

  private:

    /** Parameters to control the job */
    double m_minPairMass; /**< minimum invariant mass of the muon pair (9 GeV/c^2) */
    double m_minTrackLength; /**< minimum extrapolated track length in the crystal (30 cm) */
    double m_MaxNeighbourE; /**< maximum signal allowed in a neighbouring crystal (0.010 GeV) */
    double m_thetaLabMinDeg; /**< miniumum muon theta in lab (17 degrees) */
    double m_thetaLabMaxDeg; /**< maximum muon theta in lab (150 degrees) */
    bool m_measureTrueEnergy; /**< use eclCalDigit to determine MC deposited energy (false) */
    bool m_requireL1; /**< require events to satisfy a level 1 trigger (true) */

    /** Neighbours of each ECL crystal. 4 Neighbours for barrel and outer endcap; ;~8 otherwise */
    int firstcellIDN4 = 1009; /**< first cellID where we only need 4 neighbours */
    int lastcellIDN4 = 7920; /**< last cellID where we only need 4 neighbours */
    ECL::ECLNeighbours* myNeighbours4; /**< class to return 4 nearest neighbours to crystal */
    ECL::ECLNeighbours* myNeighbours8; /**< class to return 8 nearest neighbours to crystal */

    /** Required arrays */
    StoreArray<Track> TrackArray; /**< Required input array of tracks */
    StoreArray<ECLDigit> eclDigitArray; /**< Required input array of eclDigits */


    /** Some other useful quantities */
    double cotThetaLabMin;  /**< m_thetaLabMinDeg converted to cotangent */
    double cotThetaLabMax;  /**< m_thetaLabMaxDeg converted to cotangent */
    int iEvent = 0; /**< event counter */
    std::vector<float> EperCrys; /**< ECL digit energy for each crystal */

    /** Expected energies from database */
    DBObjPtr<ECLCrystalCalib> m_ECLExpMuMuE;
    std::vector<float> ExpMuMuE; /**< vector obtained from DB object */

    /** Electronics calibration from database */
    DBObjPtr<ECLCrystalCalib> m_ElectronicsCalib;
    std::vector<float> ElectronicsCalib; /**< vector obtained from DB object */

    /** Existing single crystal calibration from DB; will be updated by CAF */
    DBObjPtr<ECLCrystalCalib> m_MuMuECalib;
    std::vector<float> MuMuECalib; /**< vector obtained from DB object */

  };
}

