/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *    Ewan Hill                                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLHADRONTIMECALIBRATIONVALIDATIONMODULE_H
#define ECLHADRONTIMECALIBRATIONVALIDATIONMODULE_H

#include <framework/core/Module.h>
#include <ecl/utility/ECLChannelMapper.h>

#include <calibration/CalibrationCollectorModule.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventT0.h>

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
  class eclHadronTimeCalibrationValidationCollectorModule : public CalibrationCollectorModule {

  public:

    /** Module constructor */
    eclHadronTimeCalibrationValidationCollectorModule() ;

    /**
     * Module destructor.
     */
    virtual ~eclHadronTimeCalibrationValidationCollectorModule() ;

    /** Replacement for defineHisto() in CalibrationCollector modules */
    void inDefineHisto() override;

    /** Define histograms and read payloads from DB */
    void prepare() override;

    /** Select events and crystals and accumulate histograms */
    void collect() override;

  private:

    /** If true, save TTree with more detailed event info */
    bool m_saveTree ;

    /****** Parameters END ******/


    StoreArray<Track> tracks ; /**< Required input array of tracks */
    StoreArray<ECLCluster> m_eclClusterArray ; /**< Required input array of ECLClusters */
    //StoreArray<ECLDigit> m_eclDigitArray; /**< Required input array of ECLDigits */
    StoreArray<ECLCalDigit> m_eclCalDigitArray; /**< Required input array of ECLCalDigits */



    /**
     * StoreObjPtr for T0. The event t0 class has an overall event t0 so use that as presumably some code has been run to determine what the best t0 is to use.
     */
    StoreObjPtr<EventT0> m_eventT0 ;

    /**
     * Output tree with detailed event data.
     */
    TTree* m_dbg_tree_photonClusters ;
    TTree* m_dbg_tree_event ;

    /*** tree branches ***/
    /*** See inDefineHisto method for branches description ***/
    int m_tree_evt_num = -1;    /**< Event number for debug TTree output*/
    int m_tree_run = -1;     /**< Run number for debug TTree output */
    int m_tree_cid = -1;     /**< ECL Cell ID (1..8736) for debug TTree output */
    double m_tree_dt99 = -1;  /**< dt99 for cluster */
    double m_tree_time = -1; /**< Calibrated time */

    double m_tree_time_fromE0 = -1; /**< Calibrated time - highest E cluster*/
    double m_tree_E0 = -1;   /**< Highest E cluster energy */

    double m_tree_t0 = -1;   /**< EventT0 (not from ECL) for debug TTree output */
    double m_tree_t0_unc = -1;   /**< EventT0 uncertainty for debug TTree output */

    int m_NtightTracks = -1;
    int m_NphotonClusters = -1;
    int m_NGoodClusters = -1;

    /*** tree branches END ***/

    std::vector<float> m_EperCrys ; /**< ECL Cal digit energy for each crystal */


    /****** Parameters for cuts ******/
    short m_timeAbsMax ; /**< Events with abs(time) > m_timeAbsMax are excluded, mostly for histogram x-range purposes*/

    double m_looseTrkZ0 ;
    double m_tightTrkZ0 ;
    double m_looseTrkD0 ;
    double m_tightTrkD0 ;

    double m_E_photon_clust = -1;

  } ;
}

#endif /* ECLTIMECALIBRATIONVALIDATIONMODULE_H  */

