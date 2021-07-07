/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {

  /**
   * EKLM time calibration algorithm.
   */
  class EKLMDatabaseImporter {

  public:

    /**
     * Constructor.
     */
    EKLMDatabaseImporter();

    /**
     * Destructor.
     */
    ~EKLMDatabaseImporter();

    /**
     * Set interval of validity.
     */
    void setIOV(int experimentLow, int runLow, int experimentHigh, int runHigh);

    /**
     * Import simulation parameters.
     */
    void importSimulationParameters();

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
