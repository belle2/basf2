/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Ferstl                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <tracking/dataobjects/CollectorTFInfo.h>
//#include <gtest/gtest.h>

// #include <fstream>
//#include <string>


namespace Belle2 {

  /** CollectorTestModule
   *
   * simply tests the Collector (CollectorTFInfo and its ingredients) of the TrackFinder
   *
   */
  class CollectorTestModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    CollectorTestModule();

    /** Destructor of the module. */
    virtual ~CollectorTestModule();

    /** Initializes the Module.
     */
    virtual void initialize();

    /**
     * Prints a header for each new run.
     */
    virtual void beginRun();

    /** Prints the full information about the event, run and experiment number. */
    virtual void event();

    /**
     * Prints a footer for each run which ended.
     */
    virtual void endRun();

    /**
     * Termination action.
     */
    virtual void terminate();


    void import_sectors_loop();
    void import_sectors_standard();

    void import_clusters_loop();
    void import_clusters_standard();

    void import_hit_loop();
    void import_hit_standard();

    void import_cell_loop();
    void import_cell_standard();

    void import_tfc_loop();
    void import_tfc_standard();

    void getAllCells();
    void getAllHits();
    void getAllClusters();
    void getAllTC();
    void getAllSectors();

    void testSilentKill();
    void testOverlapped();
    void testAllInformationLoop();
    void testAllInformationStandard();


  protected:
//    CollectorTFInfo m_collector = CollectorTFInfo();
    CollectorTFInfo m_collector; /**< carries complete information about hits, cells and the whole rest */
    // Pass Ids
    std::vector<int> pass_sector_ids; /**< carries the IDs of the passes for fast access */
    int pass_sector_id_single; /**< @ Stefan: please explain ;) */

    int m_eventCounter; /**< knows current event number */
//   int m_stepSize; /**< Informes the user that  event: (eventCounter-modulo(stepSize)) is currently executed */
    int m_pxdClusterCounter; /**< counts total number of pxd clusters occured */
    int m_svdClusterCounter; /**< counts total number of pxd clusters occured */

  private:

  };
}
