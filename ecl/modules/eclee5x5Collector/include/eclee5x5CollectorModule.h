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
#include <analysis/utility/PCmsLabTransform.h>

//ECL
#include <ecl/geometry/ECLNeighbours.h>

namespace Belle2 {
  class ECLCrystalCalib;
  class TRGSummary;
  class EventMetaData;
  class ECLCluster;
  class ECLCalDigit;
  class ECLDigit;


  /** Calibration collector module that uses e+e- --> e+e- to do ECL single crystal energy calibration */
  class eclee5x5CollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor: Sets the description, the properties and the parameters of the module */
    eclee5x5CollectorModule();

    /** Define histograms and read payloads from DB */
    void prepare() override;

    /** Select events and crystals and accumulate histograms */
    void collect() override;

  private:
    /** Parameters to control the job */
    double m_thetaLabMinDeg; /**< miniumum ecl cluster theta in lab (17 degrees) */
    double m_thetaLabMaxDeg; /**< maximum ecl cluster theta in lab (150 degrees) */
    double m_minE0; /**< minimum energy of cluster 0: E*0/sqrts (0.45) */
    double m_minE1; /**< minimum energy of cluster 1: E*1/sqrts (0.40) */
    double m_maxdThetaSum; /**< abs(theta0* + theta1* - 180 deg) must be less than less (2 deg) */
    double m_dPhiScale; /**< scale dPhi* cut by this factor (1) */
    double m_maxTime; /**< maximum cluster time diff abs(t1-t0)/dt99 (10) */
    bool m_useCalDigits; /**< use eclCalDigit to determine MC deposited energy (false) */
    bool m_requireL1; /**< require events to satisfy a level 1 trigger (false) */

    /** Required arrays */
    StoreArray<ECLCluster> m_eclClusterArray; /**< Required input array of ECLClusters */
    StoreArray<ECLCalDigit> m_eclCalDigitArray; /**< Required input array of ECLCalDigits */
    StoreArray<ECLDigit> m_eclDigitArray; /**< Required input array of ECLDigits */
    StoreObjPtr<EventMetaData> m_evtMetaData; /**< dataStore EventMetaData */
    StoreObjPtr<TRGSummary> m_TRGResults; /**< dataStore TRGSummary */

    /** Some other useful quantities */
    double m_thetaLabMin = 0.;  /**< m_thetaLabMinDeg converted to radians */
    double m_thetaLabMax = 0.;  /**< m_thetaLabMaxDeg converted to radians */
    std::vector<float> m_dPhiMin; /**< minimum dPhi* as a function of thetaID */
    std::vector<float> m_dPhiMax; /**< maximum dPhi* as a function of thetaID */
    bool storeCalib = true; /**< force the input calibration constants to be saved first event */
    std::vector<float> EperCrys; /**< Energy for each crystal from ECLDigit or ECLCalDigit (GeV) */
    ECL::ECLNeighbours* m_eclNeighbours5x5{nullptr}; /**< Neighbour map of 25 crystals */
    PCmsLabTransform m_boostrotate; /**< boost from COM to lab and visa versa */
    double m_sqrts = 10.58; /**< sqrt s from m_boostrotate */
    std::vector<int> m_thetaID; /**< thetaID of each crystal */

    /** Expected energies from database */
    DBObjPtr<ECLCrystalCalib> m_ECLExpee5x5E;
    std::vector<float> Expee5x5E; /**< vector of energies obtained from DB object */
    std::vector<float> Expee5x5Sigma; /**< vector of sigmaE obtained from DB object */

    /** Electronics calibration from database */
    DBObjPtr<ECLCrystalCalib> m_ElectronicsCalib;
    std::vector<float> ElectronicsCalib; /**< vector obtained from DB object */

    /** Existing single crystal calibration from DB; will be updated by CAF */
    DBObjPtr<ECLCrystalCalib> m_ee5x5Calib;
    std::vector<float> ee5x5Calib; /**< vector obtained from DB object */

    /** dPhi cut */
    DBObjPtr<ECLCrystalCalib> m_selectdPhiData; /**< DB object for data */
    DBObjPtr<ECLCrystalCalib> m_selectdPhiMC; /**< DB object for MC */
    std::vector<float> meandPhi;  /**< mean requirement on dPhi from DB object */
    std::vector<float> widthdPhi;  /**< width of requirement on dPhi from DB object */

  };
}
