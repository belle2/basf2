/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/mapper/ECLChannelMapper.h>

/* Basf2 headers. */
#include <calibration/CalibrationCollectorModule.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>

class TTree ;

namespace Belle2 {
  class ECLDigit ;
  class ECLCalDigit ;
  class Track ;
  class ECLCrystalCalib ;
  class ECLCluster ;
  class ECLChannelMapper;


  /**
   * This module generates 'TimevsCrys' histogram to later
   * (in eclBhabhaTAlgorithm) find time offset from bhabha events.
   */
  class eclBhabhaTimeCalibrationValidationCollectorModule : public CalibrationCollectorModule {

  public:

    /** Module constructor */
    eclBhabhaTimeCalibrationValidationCollectorModule() ;

    /**
     * Module destructor.
     */
    virtual ~eclBhabhaTimeCalibrationValidationCollectorModule() ;

    /** Replacement for defineHisto() in CalibrationCollector modules */
    void inDefineHisto() override;

    /** Define histograms and read payloads from DB */
    void prepare() override;

    /** Select events and crystals and accumulate histograms */
    void collect() override;

  private:

    /** If true, save TTree with more detailed event info */
    bool m_saveTree ;

    StoreArray<Track> tracks ; /**< Required input array of tracks */
    StoreArray<ECLCluster> m_eclClusterArray ; /**< Required input array of ECLClusters */
    StoreArray<ECLCalDigit> m_eclCalDigitArray; /**< Required input array of ECLCalDigits */

    /**
     * ECL object for keeping track of mapping between crystals
     * and crates etc.
     */
    std::unique_ptr< Belle2::ECL::ECLChannelMapper> m_crystalMapper =
      std::make_unique<Belle2::ECL::ECLChannelMapper>();

    /** Event metadata. */
    StoreObjPtr<EventMetaData> m_EventMetaData;

    StoreObjPtr<SoftwareTriggerResult> m_TrgResult; /**< Store array for Trigger selection */
    /**
     * StoreObjPtr for T0. The event t0 class has an overall event t0 so use that as presumably some code has been run to determine what the best t0 is to use.
     */
    StoreObjPtr<EventT0> m_eventT0 ;

    TTree* m_dbg_tree_electronClusters ;    /**< debug output tree for per electron cluster*/
    TTree* m_dbg_tree_event ;    /**< debug output tree for per event*/
    TTree* m_dbg_tree_run ;    /**< debug output tree for per run*/

    /*** tree branches ***/
    /*** See inDefineHisto method for branches description ***/
    int m_tree_evt_num = -1;    /**< Event number for debug TTree output*/
    int m_tree_run = -1;     /**< Run number for debug TTree output */
    int m_tree_cid = -1;     /**< ECL Cell ID (1..ECLElementNumbers::c_NCrystals) for debug TTree output */
    double m_tree_dt99 = -1;  /**< dt99 for cluster */
    double m_tree_time = -1; /**< Calibrated time */

    double m_tree_time_fromE0 = -1; /**< Calibrated time - highest E cluster*/
    double m_tree_time_fromE1 = -1; /**< Calibrated time - second highest E cluster*/
    double m_tree_E0 = -1;   /**< Highest E cluster energy */
    double m_tree_E1 = -1;   /**< second highest E cluster energy */

    double m_tree_t0 = -1;   /**< EventT0 (not from ECL) for debug TTree output */
    double m_tree_t0_unc = -1;   /**< EventT0 uncertainty for debug TTree output */

    int m_NtightTracks = -1;   /**< Number of tight tracks */


    // for crate time plotting
    DBObjPtr<ECLCrystalCalib> m_CrateTimeDB; /**< database object */
    std::vector<float> m_CrateTime; /**< vector obtained from DB object */
    std::vector<float> m_CrateTimeUnc; /**< uncertainty vector obtained from DB object */

    /** Mapper of ecl channels to various other objects, like crates */
    DBObjPtr<Belle2::ECLChannelMap> m_channelMapDB; /**< database object */

    int     m_tree_crateid = -1;            /**< Crate ID for debug TTree output */
    double  m_tree_tcrate = -1;             /**< Crate time for debug TTree output */
    double  m_tree_tcrate_unc = -1;         /**< Crate time uncertainty for debug TTree output */
    int     m_tree_PreviousRun = -1;   /**< Run number for the previous run for debug TTree output */

    /*** tree branches END ***/

    std::vector<float> m_EperCrys ; /**< ECL Cal digit energy for each crystal */

    double m_E_electron_clust = -1 ;   /**< Electron cluster energy*/


    /****** Parameters for cuts ******/
    short m_timeAbsMax ; /**< Events with abs(time) > m_timeAbsMax are excluded, mostly for histogram x-range purposes*/

    double m_looseTrkZ0 ;   /**< Loose track z0 minimum cut*/
    double m_tightTrkZ0 ;   /**< Tight track z0 minimum cut*/
    double m_looseTrkD0 ;   /**< Loose track d0 minimum cut*/
    double m_tightTrkD0 ;   /**< Tight track d0 minimum cut*/

    bool skipTrgSel; /**< flag to skip the trigger skim selection in the module */

  } ;
}
