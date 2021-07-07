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

//ECL
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  class ECLDigit;
  class ECLCalDigit;
  class Track;
  class ECLCrystalCalib;
  class EventMetaData;
  class TRGSummary;

  namespace ECL {
    class ECLNeighbours;
  }

  /** Calibration collector module that uses muon pairs to do ECL single crystal energy calibration */
  class eclMuMuECollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor: Sets the description, the properties and the parameters of the module */
    eclMuMuECollectorModule();

    /** Define histograms and read payloads from DB */
    void prepare() override;

    /** Select events and crystals and accumulate histograms */
    void collect() override;

  private:

    /** Parameters to control the job */
    double m_minPairMass{9.0}; /**< minimum invariant mass of the muon pair (9 GeV/c^2) */
    double m_minTrackLength{30.0}; /**< minimum extrapolated track length in the crystal (30 cm) */
    double m_MaxNeighbourE{0.010}; /**< maximum signal allowed in a neighbouring crystal (0.010 GeV) */
    double m_thetaLabMinDeg{17.0}; /**< miniumum muon theta in lab (17 degrees) */
    double m_thetaLabMaxDeg{150.0}; /**< maximum muon theta in lab (150 degrees) */
    bool m_measureTrueEnergy{false}; /**< use eclCalDigit to determine MC deposited energy (false) */
    bool m_requireL1{true}; /**< require events to satisfy a level 1 trigger (true) */

    /** Neighbours of each ECL crystal. 4 Neighbours for barrel and outer endcap; ;~8 otherwise */
    int firstcellIDN4 = 1009; /**< first cellID where we only need 4 neighbours */
    int lastcellIDN4 = 7920; /**< last cellID where we only need 4 neighbours */
    ECL::ECLNeighbours* myNeighbours4{nullptr}; /**< class to return 4 nearest neighbours to crystal */
    ECL::ECLNeighbours* myNeighbours8{nullptr}; /**< class to return 8 nearest neighbours to crystal */

    /** Required arrays */
    StoreArray<Track> m_trackArray; /**< Required input array of tracks */
    StoreArray<ECLDigit> m_eclDigitArray; /**< Required input array of eclDigits */
    StoreObjPtr<EventMetaData> m_evtMetaData; /**< DataStore EventMetaData */
    StoreObjPtr<TRGSummary> m_TRGResults; /**< DataStore TRGSummary */
    StoreArray<ECLCalDigit> m_eclCalDigitArray; /**< DataStore TRGSummary */

    /** Some other useful quantities */
    double cotThetaLabMin{0.0};  /**< m_thetaLabMinDeg converted to cotangent */
    double cotThetaLabMax{0.0};  /**< m_thetaLabMaxDeg converted to cotangent */
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
