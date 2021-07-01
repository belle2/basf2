/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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
  class ECLReferenceCrystalPerCrateCalib;
  class ECLCluster;
  class ECLChannelMapper;

  const static double realNaN = std::numeric_limits<double>::quiet_NaN();  /**< constant for double NaN */
  const static int    intNaN  = std::numeric_limits<int>::quiet_NaN();     /**< constant for integer NaN */

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
    void inDefineHisto() override;

    /** Define histograms and read payloads from DB */
    void prepare() override;

    /** Select events and crystals and accumulate histograms */
    void collect() override;

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

    /** Crystal IDs of the one reference crystal per crate from database */
    DBObjPtr<ECLReferenceCrystalPerCrateCalib> m_RefCrystalsCalibDB; /**< database object */
    std::vector<short> m_RefCrystalsCalib; /**< vector obtained from DB object */

    /**
     * Output tree with detailed event data.
     */
    TTree* m_dbgTree_electrons = nullptr;       /**< Debug TTree output per electron */
    TTree* m_dbgTree_tracks = nullptr;          /**< Debug TTree output per track */
    TTree* m_dbgTree_crystals = nullptr;        /**< Debug TTree output per crystal */
    TTree* m_dbgTree_event = nullptr;           /**< Debug TTree output per event */
    TTree* m_dbgTree_allCuts = nullptr;         /**< Debug TTree output after all cuts */
    TTree* m_dbgTree_evt_allCuts = nullptr;     /**< Debug TTree output per event after all cuts */
    TTree* m_dbgTree_crys_allCuts = nullptr;    /**< Debug TTree output per crystal after all cuts */

    /*** tree branches ***/
    /*** See inDefineHisto method for branches description ***/
    int m_tree_evtNum;    /**< Event number for debug TTree output*/
    int m_tree_cid;     /**< ECL Cell ID (1..8736) for debug TTree output */
    int m_tree_amp;     /**< Fitting amplitude from ECL for debug TTree output */
    double m_tree_en;     /**< Energy of crystal with maximum energy within ECL cluster, GeV for debug TTree output */
    double m_tree_E1Etot;     /**< Energy of crystal with maximum energy within
                                    ECL cluster divided by total cluster energy,
                                    unitless for debug TTree output */
    double m_tree_E1E2 = realNaN;     /**< Energy of crystal with maximum energy within ECL
                                  cluster divided by second most energetic crystal
                                  in the cluster, unitless for debug TTree output */
    double m_tree_E1p = realNaN;     /**< Energy of crystal with maximum energy within ECL
                                 cluster divided by total cluster energy divided by
                                 the track momentum, unitless for debug TTree output */
    int m_tree_quality = intNaN; /**< ECL fit quality for debug TTree output */
    double m_tree_timeF = realNaN; /**< ECL fitting time for debug TTree output */
    double m_tree_time = realNaN; /**< Time for Ts distribution for debug TTree output */
    double m_tree_timetsPreviousTimeCalibs = realNaN; /**< Time for Ts distribution after
                                                  application of previous time calibrations
                                                  for debug TTree output */
    double m_tree_t0 = realNaN;   /**< EventT0 (not from ECL) for debug TTree output */
    double m_tree_t0_unc = realNaN;   /**< EventT0 uncertainty for debug TTree output */
    double m_tree_t0_ECLclosestCDC = realNaN;   /**< EventT0 (from ECL) closest to CDC for debug TTree output */
    double m_tree_t0_ECL_minChi2 = realNaN;   /**< EventT0 (from ECL) min chi2 for debug TTree output */
    double m_tree_d0 = realNaN;    /**< Track d0 for debug TTree output */
    double m_tree_z0 = realNaN;    /**< Track z0 for debug TTree output */
    double m_tree_p = realNaN;    /**< Track momentum for debug TTree output */
    double m_tree_nCDChits = realNaN;    /**< Number of CDC hits along the track for debug TTree output */
    double m_tree_clustCrysE_DIV_maxEcrys = realNaN;    /**< ratio of crystal energy to energy of the crystal that
                                                   has the maximum energy, only for the crystals that
                                                   meet all the selection criteria for debug TTree output */
    double m_tree_clustCrysE = realNaN;    /**< crystal energy, only for the crystals that meet all the selection
                                      criteria for debug TTree output */


    double m_tree_enPlus = realNaN;     /**< Energy of cluster associated to positively charged track, GeV for debug TTree output */
    double m_tree_enNeg = realNaN;     /**< Energy of cluster associated to negatively charged track, GeV for debug TTree output */
    double m_tree_tClustPos =
      realNaN;     /**< Cluster time of cluster associated to positively charged track, ns for debug TTree output */
    double m_tree_tClustNeg =
      realNaN;     /**< Cluster time of cluster associated to negatively charged track, ns for debug TTree output */
    double m_tree_maxEcrystPosClust = realNaN;     /**< Time of the highest energy crystal in the cluster
                                               associated to positively charged track, ns for debug TTree output */
    double m_tree_maxEcrystNegClust = realNaN;     /**< Time of the highest energy crystal in the cluster associated
                                              to negatively charged track, ns for debug TTree output */

    double m_tree_tClust = realNaN;     /**< Cluster time of a cluster, ns for debug TTree output */

    double m_tree_ECLCalDigitTime = realNaN;   /**< Time of an ECLCalDigit within a cluster, ns for debug TTree output */
    double m_tree_ECLCalDigitE = realNaN;   /**< Energy of an ECLCalDigit within a cluster, GeV for debug TTree output */
    double m_tree_ECLDigitAmplitude = realNaN;   /**< Amplitude (used to calculate energy) of an ECLDigit within
                                            a cluster, for debug TTree output */


    int m_charge = intNaN;        /**< particle charge, for debug TTree output */
    double m_E_DIV_p = realNaN;    /**< Energy divided by momentum, for debug TTree output */
    double m_massInvTracks = realNaN;    /**< invariant mass of the two tracks, for debug TTree output */


    /*** tree branches END ***/


    StoreArray<ECLDigit> m_eclDigitArray; /**< Required input array of ECLDigits */
    StoreArray<ECLCalDigit> m_eclCalDigitArray; /**< Required input array of ECLCalDigits */
    StoreArray<ECLCluster> m_eclClusterArray; /**< Required input array of ECLClusters */

    std::vector<float> m_EperCrys; /**< ECL cal digit energy for each crystal */
    std::vector<int> m_eclCalDigitID; /**< ECL cal digit id sorter */
    std::vector<int> m_eclDigitID; /**< ECL digit id sorter */




    /****** Parameters for cuts ******/
    short m_timeAbsMax; /**< Events with abs(time) > m_timeAbsMax are excluded, mostly for histogram x-range purposes*/

    int m_minCrystal = intNaN; /**< First CellId to handle */
    int m_maxCrystal = intNaN; /**< Last CellId to handle */

    /* d0 and z0 values of the loose and tight tracks*/
    double m_looseTrkZ0 = realNaN;   /**< Loose track z0 minimum cut*/
    double m_tightTrkZ0 = realNaN;   /**< Tight track z0 minimum cut*/
    double m_looseTrkD0 = realNaN;   /**< Loose track d0 minimum cut*/
    double m_tightTrkD0 = realNaN;   /**< Tight track d0 minimum cut*/

    int m_crystalCrate = intNaN;    /**< Crate id for the crystal */
    int m_runNum = intNaN;   /**< run number */

    bool m_storeCalib = true;   /**< Boolean for whether or not to store the previous
                                    calibration calibration constants*/

    // For the energy dependence correction to the time
    // t-t0 = p1 + pow( (p3/(amplitude+p2)), p4 ) + p5*exp(-amplitude/p6)      ("Energy dependence equation")

    std::unique_ptr< Belle2::ECL::ECLTimingUtilities > m_ECLTimeUtil =
      std::make_unique<Belle2::ECL::ECLTimingUtilities>(); /**< ECL timing tools */

    /**  correction to apply to CDC event t0 values in bhabha events to correct for CDC event t0
         bias compared to CDC event t0 in hadronic events in ns*/
    double m_hadronEventT0_TO_bhabhaEventT0_correction;


  };
}
