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
    virtual ~BKLMDatabaseImporter()
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

