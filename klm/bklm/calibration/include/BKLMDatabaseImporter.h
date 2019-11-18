/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui Guan, Vipin GAUR, Z. S. Stottler,                *
 *               Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/bklm/dbobjects/BKLMADCThreshold.h>
#include <klm/bklm/dbobjects/BKLMElectronicsChannel.h>
#include <klm/bklm/dbobjects/BKLMTimeWindow.h>

namespace Belle2 {

  /**
   This module writes BKLM data to database
   */
  class BKLMDatabaseImporter {
  public:

    /**
    * Constructor
    */
    BKLMDatabaseImporter();

    /**
    * Destructor
    */
    virtual ~BKLMDatabaseImporter() {};

    /**
     * Set interval of validity.
     */
    void setIOV(int experimentLow, int runLow, int experimentHigh, int runHigh);

    /**
     * Load default electronics mapping.
     */
    void loadDefaultElectronicMapping();

    /**
     * Set non-default lane.
     * @param[in] section Section.
     * @param[in] sector  Sector.
     * @param[in] layer   Layer.
     * @param[in] lane    Lane.
     */
    void setElectronicMappingLane(
      int section, int sector, int layer, int lane);

    /**
     * Import BKLM electronics mapping in the database.
     */
    void importElectronicMapping();

    /**
     * Import BKLM geometry parameters into the database
     */
    void importGeometryPar();

    /**
     * Import BKLM simulation parameters into the database
     */
    void importSimulationPar();

    /**
     * Import BKLM Alignment parameters into the database
     */
    void importAlignment();

    /**
     * Import BKLM displaced-geometry parameters into the database
     */
    void importDisplacement();

    /**
     * Import BKLM scintillator ADC parameters into the database
     */
    void importADCThreshold(BKLMADCThreshold* inputThreshold);

    /**
     * Import BKLM time window cuts into the database
     */
    void importTimeWindow(BKLMTimeWindow* inputWindow);

  private:

    /** Electronics mapping. */
    std::vector< std::pair<uint16_t, BKLMElectronicsChannel> > m_ElectronicsChannels;

    /** Low experiment. */
    int m_ExperimentLow;

    /** Low run. */
    int m_RunLow;

    /** High experiment. */
    int m_ExperimentHigh;

    /** High run. */
    int m_RunHigh;

  };

}

