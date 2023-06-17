/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <arich/dbobjects/ARICHSimulationPar.h>
#include <arich/dbobjects/ARICHModulesInfo.h>
#include <arich/dbobjects/ARICHChannelMask.h>
#include <arich/dbobjects/ARICHChannelMapping.h>
#include <framework/database/DBObjPtr.h>

#include <string>

namespace Belle2 {
  class ARICHDigit;
  class ARICHSimHit;

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
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     *
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     *
     * Convert ARICHSimHits of the event to arichDigits.
     */
    virtual void event() override;

    /**
     * Apply correction to hit position due to non-perpendicular component of magnetic field
     * @param hit local position of simhit
     * @param copyno copy number of hapd
     */
    void magFieldDistorsion(TVector2& hit, int copyno);

  private:

    std::string m_inColName;         /**< Input collection name from simulation */
    std::string m_outColName;        /**< Output collection name: digitized photons hits */

    bool m_bgOverlay = false; /**< True if BG overlay detected */

    DBObjPtr<ARICHModulesInfo> m_modInfo;     /**< information on installed modules from the DB (QEs etc.) */
    DBObjPtr<ARICHSimulationPar> m_simPar;    /**< simulation parameters from the DB */
    DBObjPtr<ARICHGeometryConfig> m_geoPar;   /**< geometry configuration parameters from the DB */
    DBObjPtr<ARICHChannelMask> m_chnMask;     /**< list of dead channels from the DB */
    DBObjPtr<ARICHChannelMapping> m_chnMap;   /**< HAPD (x,y) to asic channel mapping */

    StoreArray<ARICHDigit> m_ARICHDigits;     /**< StoreArray containing the ARICHDigits */
    StoreArray<ARICHSimHit> m_ARICHSimHits;   /**< StoreArray containing the ARICHSimHits */

    /* Other members.*/
    double m_maxQE;                  /**< QE at 400nm (from QE curve applied in SensitveDetector) */
    double m_timeWindow;             /**< Readout time window width in ns */
    double m_timeWindowStart;        /**< Readout time window shift w.r.t. the global time zero in ns */
    double m_bkgLevel;               /**< Number of background hits ped hapd per readout (electronics noise) */
    int m_bdistort;                  /**< apply distorsion due to magnetic field */

  };

} // Belle2 namespace