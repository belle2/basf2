/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/**************************************************************************
 * Description:                                                           *
 * A code to collect just the calibration constants to be then plotted    *
 * by the algorithm to see how much they jump from run to run.  This      *
 * collector does not need to look at individual events as the            *
 * calibration constants are constant over all the events in a single     *
 * file (run or bucket of data).                                          *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <ecl/utility/ECLChannelMapper.h>
#include <calibration/CalibrationCollectorModule.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>


class TTree;

namespace Belle2 {
  class ECLCrystalCalib;
  class ECLReferenceCrystalPerCrateCalib;
  class ECLChannelMapper;


  /**
   * This modules looks up crystal and crate time offset constants from the database
   * and saves them to a tree for the algorithm to plot as a function of run number
   */
  class eclTimeShiftsPlottingCollectorModule : public CalibrationCollectorModule {

  public:

    /** Module constructor */
    eclTimeShiftsPlottingCollectorModule();

    /**
     * Module destructor.
     */
    virtual ~eclTimeShiftsPlottingCollectorModule();

    /** Replacement for defineHisto() in CalibrationCollector modules */
    void inDefineHisto() override;

    /** Define histograms and read payloads from DB */
    void prepare() override;

    /** Select events and crystals and accumulate histograms */
    void collect() override;

  private:

    /** Time offset from previous crystal time calibration (this calibration) from database */
    DBObjPtr<ECLCrystalCalib> m_CrystalTimeDB; /**< database object */
    std::vector<float> m_CrystalTime; /**< vector obtained from DB object */
    std::vector<float> m_CrystalTimeUnc; /**< vector obtained from DB object */

    /** Time offset from crate time calibration (also this calibration) from database */
    DBObjPtr<ECLCrystalCalib> m_CrateTimeDB; /**< database object */
    std::vector<float> m_CrateTime; /**< vector obtained from DB object */
    std::vector<float> m_CrateTimeUnc; /**< uncertainty vector obtained from DB object */

    /** Crystal IDs of the one reference crystal per crate from database */
    DBObjPtr<ECLReferenceCrystalPerCrateCalib> m_RefCrystalsCalibDB; /**< database object */
    std::vector<short> m_RefCrystalsCalib; /**< vector obtained from DB object */

    StoreObjPtr<EventMetaData> m_evtMetaData;    /**< Event meta data */

    /*** tree branches ***/
    double m_crateTimeConst = -1;         /**< crate time constant in ticks */
    double m_crystalTimeConst = -1;       /**< crystal time constants in ticks */
    double m_crateTimeConstUnc = -1;      /**< crate time uncertainty in ticks */
    double m_crystalTimeConstUnc = -1;    /**< crystal time uncertainty in ticks */
    int m_crystalID = -1;                 /**< crystal identification number*/
    int m_crateID = -1;                   /**< crate identification number*/
    int m_refCrystalID = -1;              /**< reference crystal identification number*/
    int m_run = -1;                       /**< Current run number*/
    int m_exp = -1;                       /**< Current experiment number*/

    /*** tree branches END ***/

    int m_previousExp = -2;          /**< Previous experiment number, in case we
                                          run over several runs.  Different initial
                                          value from the current experiment number. */
    int m_previousRun = -2;          /**< Previous run number, in case we run over
                                          several runs.  Different initial value
                                          from the current run number variable. */

    const int NUM_CRYSTALS = 8736;    /**< Number of crystals in the ECL */

  };
}

