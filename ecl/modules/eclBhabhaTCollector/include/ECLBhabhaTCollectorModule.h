/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *    Ewan Hill                                                           *
 *    Mikhail Remnev                                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLBHABHATCOLLECTORMODULE_H
#define ECLBHABHATCOLLECTORMODULE_H

#include <framework/core/Module.h>
#include <ecl/utility/ECLChannelMapper.h>
#include <ecl/utility/ECLTimingUtilities.h>

#include <calibration/CalibrationCollectorModule.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/EventT0.h>

class TTree;

namespace Belle2 {
  class ECLDigit;
  class ECLCalDigit;
  class Track;
  class ECLCrystalCalib;
  class ECLCluster;
  class ECLChannelMapper;


  /**
   * This module generates time vs crystal 2D histograms to later
   * (in eclBhabhaTAlgorithm) find time crystal/crate offsets
   * from bhabha events.
   */
  class ECLBhabhaTCollectorModule : public CalibrationCollectorModule {

  public:

    /** Module constructor */
    ECLBhabhaTCollectorModule();

    /**
     * Module destructor.
     */
    virtual ~ECLBhabhaTCollectorModule();

    /** Replacement for defineHisto() in CalibrationCollector modules */
    void inDefineHisto();

    /** Define histograms and read payloads from DB */
    void prepare();

    /** Select events and crystals and accumulate histograms */
    void collect();

  private:

    /** If true, save TTree with more detailed event info */
    bool m_saveTree;

    /****** Parameters END ******/


    StoreArray<Track> tracks; /**< StoreArray for tracks */
    /**
     * StoreObjPtr for T0. The event t0 class has an overall event t0
     */
    StoreObjPtr<EventT0> m_eventT0;

    /** Event metadata. */
    StoreObjPtr<EventMetaData> m_EventMetaData;

    /** electronics amplitude calibration from database
        Scale amplitudefor each crystal and for dead pre-amps*/
    DBObjPtr<ECLCrystalCalib> m_ElectronicsDB; /**< database object */
    std::vector<float> m_Electronics; /**< vector obtained from DB object */

    /** Time offset from electronics calibration from database */
    DBObjPtr<ECLCrystalCalib> m_ElectronicsTimeDB; /**< database object */
    std::vector<float> m_ElectronicsTime; /**< vector obtained from DB object */

    /** Time offset from flight time b/w IP and crystal from database */
    DBObjPtr<ECLCrystalCalib> m_FlightTimeDB; /**< database object */
    std::vector<float> m_FlightTime; /**< vector obtained from DB object */

    /** Time offset from previous crystal time calibration (this calibration) from database */
    DBObjPtr<ECLCrystalCalib> m_PreviousCrystalTimeDB; /**< database object */
    std::vector<float> m_PreviousCrystalTime; /**< vector obtained from DB object */
    std::vector<float> m_PreviousCrystalTimeUnc; /**< vector obtained from DB object */

    /** Time offset from crate time calibration (also this calibration) from database */
    DBObjPtr<ECLCrystalCalib> m_CrateTimeDB; /**< database object */
    std::vector<float> m_CrateTime; /**< vector obtained from DB object */
    std::vector<float> m_CrateTimeUnc; /**< uncertainty vector obtained from DB object */

    /**
     * Output tree with detailed event data.
     */
    TTree* m_dbgTree_electrons;
    TTree* m_dbgTree_tracks;
    TTree* m_dbgTree_crystals;
    TTree* m_dbgTree_event;
    TTree* m_dbgTree_allCuts;
    TTree* m_dbgTree_evt_allCuts;
    TTree* m_dbgTree_crys_allCuts;

    /*** tree branches ***/
    /*** See inDefineHisto method for branches description ***/
    int m_tree_evtNum;    /**< Event number for debug TTree output*/
    int m_tree_cid;     /**< ECL Cell ID (1..8736) for debug TTree output */
    double m_tree_phi;     /**< phi position for debug TTree output */
    double m_tree_theta;     /**< theta position for debug TTree output */
    int m_tree_amp;     /**< Fitting amplitude from ECL for debug TTree output */
    double m_tree_en;     /**< Energy of crystal with maximum energy within ECL cluster, GeV for debug TTree output */
    double m_tree_E1Etot;     /**< Energy of crystal with maximum energy within
                                    ECL cluster divided by total cluster energy,
                                    unitless for debug TTree output */
    double m_tree_E1E2;     /**< Energy of crystal with maximum energy within ECL
                                  cluster divided by second most energetic crystal
                                  in the cluster, unitless for debug TTree output */
    double m_tree_E1p;     /**< Energy of crystal with maximum energy within ECL
                                 cluster divided by total cluster energy divided by
                                 the track momentum, unitless for debug TTree output */
    int m_tree_quality; /**< ECL fit quality for debug TTree output */
    double m_tree_timeF; /**< ECL fitting time for debug TTree output */
    double m_tree_time; /**< Time for Ts distribution for debug TTree output */
    double m_tree_timetsPreviousTimeCalibs; /**< Time for Ts distribution after
                                                  application of previous time calibrations
                                                  for debug TTree output */
    double m_tree_t0;   /**< EventT0 (not from ECL) for debug TTree output */
    double m_tree_t0_unc;   /**< EventT0 uncertainty for debug TTree output */
    double m_tree_t0_ECLclosestCDC;   /**< EventT0 (from ECL) closest to CDC for debug TTree output */
    double m_tree_t0_ECL_minChi2;   /**< EventT0 (from ECL) min chi2 for debug TTree output */
    double m_tree_d0;    /** Track d0 for debug TTree output */
    double m_tree_z0;    /** Track z0 for debug TTree output */
    double m_tree_p;    /** Track momentum for debug TTree output */
    double m_tree_nCDChits;    /** Number of CDC hits along the track for debug TTree output */
    double m_tree_clustCrysE_DIV_maxEcrys;    /** ratio of crystal energy to energy of the crystal that
                                                   has the maximum energy, only for the crystals that
                                                   meet all the selection criteria for debug TTree output */
    double m_tree_clustCrysE;    /** crystal energy, only for the crystals that meet all the selection
                                      criteria for debug TTree output */


    double m_tree_enPlus;     /**< Energy of cluster associated to positively charged track, GeV for debug TTree output */
    double m_tree_enNeg;     /**< Energy of cluster associated to negatively charged track, GeV for debug TTree output */
    double m_tree_tClustPos;     /**< Cluster time of cluster associated to positively charged track, ns for debug TTree output */
    double m_tree_tClustNeg;     /**< Cluster time of cluster associated to negatively charged track, ns for debug TTree output */
    double m_tree_maxEcrystPosClust;     /**< Time of the highest energy crystal in the cluster
                                               associated to positively charged track, ns for debug TTree output */
    double m_tree_maxEcrystNegClust;     /**< Time of the highest energy crystal in the cluster associated
                                              to negatively charged track, ns for debug TTree output */

    double m_tree_tClust;     /**< Cluster time of a cluster, ns for debug TTree output */

    double m_tree_ECLCalDigitTime;   /**< Time of an ECLCalDigit within a cluster, ns for debug TTree output */
    double m_tree_ECLCalDigitE;   /**< Energy of an ECLCalDigit within a cluster, GeV for debug TTree output */
    double m_tree_ECLDigitAmplitude;   /**< Amplitude (used to calculate energy) of an ECLDigit within
                                            a cluster, for debug TTree output */


    int m_charge;        /**< particle charge, for debug TTree output */
    double m_E_DIV_p;    /**< Energy divided by momentum, for debug TTree output */
    double m_massInvTracks;    /**< invariant mass of the two tracks, for debug TTree output */


    /*** tree branches END ***/


    StoreArray<ECLDigit> m_eclDigitArray; /**< Required input array of ECLDigits */
    StoreArray<ECLCalDigit> m_eclCalDigitArray; /**< Required input array of ECLCalDigits */
    StoreArray<ECLCluster> m_eclClusterArray; /**< Required input array of ECLClusters */

    std::vector<float> m_EperCrys; /**< ECL digit energy for each crystal */
    std::vector<int> m_eclCalDigitID; /**< ECL cal digit id sorter */
    std::vector<int> m_eclDigitID; /**< ECL digit id sorter */




    /****** Parameters for cuts ******/
    short m_timeAbsMax; /**< Events with abs(time) > m_timeAbsMax are excluded, mostly for histogram x-range purposes*/

    int m_minCrystal; /**< First CellId to handle */
    int m_maxCrystal; /**< Last CellId to handle */

    /* d0 and z0 values of the loose and tight tracks*/
    double m_looseTrkZ0;
    double m_tightTrkZ0;
    double m_looseTrkD0;
    double m_tightTrkD0;

    int m_crystalCrate;    /**< Crate id for the crystal */
    int m_runNum;   /**< run number */

    bool m_storeCalib = true;   /**< Boolean for whether or not to store the previous
                                    calibration calibration constants*/

    // For the energy dependence correction to the time
    // t-t0 = p1 + pow( (p3/(amplitude+p2)), p4 ) + p5*exp(-amplitude/p6)      ("Energy dependence equation")

    std::unique_ptr< Belle2::ECL::ECLTimingUtilities > m_ECLTimeUtil =
      std::make_unique<Belle2::ECL::ECLTimingUtilities>(); /**< ECL timing tools */


    double m_energyDependenceTimeOffsetFitParam_p1 = 0;                /**< p1 in "energy dependence equation" */
    double m_energyDependenceTimeOffsetFitParam_p2 = 88449.;           /**< p2 in "energy dependence equation" */
    double m_energyDependenceTimeOffsetFitParam_p3 = 0.20867E+06;      /**< p3 in "energy dependence equation" */
    double m_energyDependenceTimeOffsetFitParam_p4 = 3.1482;           /**< p4 in "energy dependence equation" */
    double m_energyDependenceTimeOffsetFitParam_p5 = 7.4747;           /**< p5 in "energy dependence equation" */
    double m_energyDependenceTimeOffsetFitParam_p6 = 1279.3;           /**< p6 in "energy dependence equation" */


  };
}

#endif /* ECLBHABHATCOLLECTORMODULE_H */

