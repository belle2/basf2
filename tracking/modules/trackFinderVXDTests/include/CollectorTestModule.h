/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Ferstl                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/* Additional Info:
* This Module is in an early stage of developement. The comments are mainly for temporal purposes
* and will be changed and corrected in later stages of developement. So please ignore them.
*/

#ifndef CollectorTestModule_H_
#define CollectorTestModule_H_

#include <framework/core/Module.h>
#include <tracking/dataobjects/CollectorTFInfo.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <gtest/gtest.h>

// #include <fstream>
#include <string>


namespace Belle2 {

  /** The event counter module
   *
   * this module simply counts the number of events (and prints every x-th event onto the screen, where x is user-defined). Useful when executing modules which do not provide this feature themselves
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
    CollectorTFInfo m_collector = CollectorTFInfo();
    // Pass Ids
    std::vector<int> pass_sector_ids = {0, 1, 2};
    int pass_sector_id_single = 0;


    int m_eventCounter; /**< knows current event number */
    int m_stepSize; /**< Informes the user that  event: (eventCounter-modulo(stepSize)) is currently executed */
    int m_pxdClusterCounter; /**< counts total number of pxd clusters occured */
    int m_svdClusterCounter; /**< counts total number of pxd clusters occured */

  private:

  };
}

#endif /* CollectorTestModule_H_ */
