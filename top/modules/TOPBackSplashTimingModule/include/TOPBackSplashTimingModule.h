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
#include <mdst/dataobjects/ECLCluster.h>


namespace Belle2 {
  /**
   * Extract timing of an ECL cluster from TOP signal
   */
  class TOPBackSplashTimingModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TOPBackSplashTimingModule();

    /**  */
    ~TOPBackSplashTimingModule() override;

    /** Register input and output data */
    void initialize() override;
    // Need to fetch DB payloads and store locally in private variable, to avoid repeated calls to DB
    // Create the PDF object, adjust parameters stead of creatin geach event
    // & initialise mdst obj (?)
    /**  */
    void beginRun() override;

    /**  */
    void event() override;
    // Loop over clusters, run fit

    /**  */
    void endRun() override;

    /**  */
    void terminate() override;
    // Write out mdst


  private:
    // private method, write the fitter
    float m_minClusterE;  /**< Minimum energy of the cluster to be used */
    bool m_getPDFfromDB;  /**< Get the PDF parameters from DB, otherwise floating */
    StoreArray<ECLCluster> m_eclCluster; /**<  */
    StoreArray<TOPClusterTime> m_topClusterTime; /**< Fitted time */
  };
}
