/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Mikhail Remnev                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLBHABHATCOLLECTORMODULE_H
#define ECLBHABHATCOLLECTORMODULE_H

#include <framework/core/Module.h>
#include <ecl/utility/ECLChannelMapper.h>

#include <calibration/CalibrationCollectorModule.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventT0.h>

class TTree;

namespace Belle2 {
  class ECLDigit;
  class ECLCalDigit;
  class Track;
  class ECLCrystalCalib;

  /**
   * This module generates 'TimevsCrys' histogram to later
   * (in eclBhabhaTAlgorithm) find time offset from bhabha events.
   */
  class ECLBhabhaTCollectorModule : public CalibrationCollectorModule {

  public:

    /**  */
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
    /****** Parameters ******/

    double m_minEn; /**< Low energy cut (GeV) */
    double m_maxEn; /**< High energy cut (GeV) */
    double m_maxTotalEn; /**< High energy cut for total energy in event (GeV) */
    short m_timeAbsMax; /**< Events with abs(time) > m_timeAbsMax are excluded */
    /**
     * Events with ECLDigits.getEntries() > m_nentriesMax are excluded.
     */
    int m_nentriesMax;

    bool m_saveTree; /** If true, save TTree with more detailed event info */
    /**
     * If true, time difference histogram is filled with weight
     * min(energy*energy, 1 GeV)
     */
    bool m_weightedHist;

    int m_minCrystal; /**< First CellId to handle */
    int m_maxCrystal; /**< Last CellId to handle */

    /****** Parameters END ******/

    StoreArray<ECLDigit> ecl_digits; /**< StoreArray for input ECL events */
    StoreArray<ECLCalDigit> ecl_cal_digits; /**< StoreArray for calibrated input ECL events */
    StoreArray<Track> tracks; /**< StoreArray for tracks */
    /**
     * StoreObjPtr for T0. Only eventT0 that comes from CDC is selected.
     */
    StoreObjPtr<EventT0> event_t0;

    /** Time offset from electronics calibration from database */
    DBObjPtr<ECLCrystalCalib> m_ElectronicsTime; /**< database object */
    std::vector<float> ElectronicsTime; /**< vector obtained from DB object */

    /** Time offset from flight time b/w IP and crystal from database */
    DBObjPtr<ECLCrystalCalib> m_FlightTime; /**< database object */
    std::vector<float> FlightTime; /**< vector obtained from DB object */

    int m_tracks;            /**< Number of good tracks */
    double m_track_chi2[30]; /**< chi2 of good tracks */
    double m_track_ndf[30];  /**< ndf of good tracks */
    double en_sum;           /**< Total energy in event, GeV */

    /**
     * Output tree with detailed event data.
     */
    TTree* m_dbg_tree;

    /*** tree branches ***/
    /*** See inDefineHisto method for branches description ***/
    int m_tree_evtn;    /**< */
    int m_tree_exp;     /**< Experiment number */
    int m_tree_run;     /**< Run number */
    int m_tree_cid;     /**< ECL Cell ID (1..8736) */
    int m_tree_amp;     /**< */
    double m_tree_en;     /**< Energy, GeV */
    int m_tree_quality; /**< */
    double m_tree_time; /**< */
    double m_tree_t0;   /**< */
    /*** tree branches END ***/

  };
}

#endif /* ECLBHABHATCOLLECTORMODULE_H */

