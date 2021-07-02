/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dbobjects/bklm/BKLMADCThreshold.h>

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
    ~BKLMDatabaseImporter()
    {
    }

    /**
     * Set interval of validity.
     */
    void setIOV(int experimentLow, int runLow, int experimentHigh, int runHigh);

    /**
     * Import BKLM geometry parameters into the database
     */
    void importGeometryPar();

    /**
     * Import BKLM simulation parameters into the database
     */
    void importSimulationPar();

    /**
     * Import BKLM scintillator ADC parameters into the database
     */
    void importADCThreshold(BKLMADCThreshold* inputThreshold);

  private:

    /** Low experiment. */
    int m_ExperimentLow = 0;

    /** Low run. */
    int m_RunLow = 0;

    /** High experiment. */
    int m_ExperimentHigh = -1;

    /** High run. */
    int m_RunHigh = -1;

  };

}

