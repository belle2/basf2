/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHDIGITIZERMODULE_H
#define ARICHDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <arich/dbobjects/ARICHSimulationPar.h>
#include <arich/dbobjects/ARICHModulesInfo.h>
#include <arich/dbobjects/ARICHChannelMask.h>
#include <arich/dbobjects/ARICHChannelMapping.h>
#include <framework/database/DBObjPtr.h>

#include <string>

namespace Belle2 {
  /** ARICH digitizer module.
    *
    * This module takes the hits form GEANT4 simulation (ARICHSimHit),
    * applies q.e. of HAPDs, calculates and saves hit channel numbers (ARICHDigit).
    * If the channel has multiple hits, only one is saved.
    */
  class ARICHDigitizerModule : public Module {

  public:

    /**
     * Constructor.
     */
    ARICHDigitizerModule();

    /**
     *   Destructor.
     */
    virtual ~ARICHDigitizerModule();

    /**
     * Initialize the Module.
     *
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     *
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     *
     * Convert ARICHSimHits of the event to arichDigits.
     */
    virtual void event();

    /**
     * End-of-run action.
     *
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     *
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

    /**
     *Prints module parameters.
     */
    void printModuleParams() const {};

  private:

    std::string m_inColName;         /**< Input collection name from simulation */
    std::string m_outColName;        /**< Output collection name: digitized photons hits */

    bool m_bgOverlay = false; /**< True if BG overlay detected */

    DBObjPtr<ARICHModulesInfo> m_modInfo;     /**< information on installed modules from the DB (QEs etc.) */
    DBObjPtr<ARICHSimulationPar> m_simPar;    /**< simulation parameters from the DB */
    DBObjPtr<ARICHGeometryConfig> m_geoPar;   /**< geometry configuration parameters from the DB */
    DBObjPtr<ARICHChannelMask> m_chnMask;     /**< list of dead channels from the DB */
    DBObjPtr<ARICHChannelMapping> m_chnMap;   /**< HAPD (x,y) to asic channel mapping */

    /* Other members.*/
    double m_maxQE;                  /**< QE at 400nm (from QE curve applied in SensitveDetector) */
    double m_timeWindow;             /**< Readout time window width */
    double m_bkgLevel;               /**< Number of background hits ped hapd per readout (electronics noise) */

  };

} // Belle2 namespace

#endif // ARICHDIGITIZERMODULE_H
