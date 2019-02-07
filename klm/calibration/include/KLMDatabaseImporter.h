/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <framework/database/DBImportObjPtr.h>
#include <klm/dbobjects/KLMTimeConversion.h>

namespace Belle2 {

  /**
   * KLM database importer.
   */
  class KLMDatabaseImporter {

  public:

    /**
     * Constructor.
     */
    KLMDatabaseImporter();

    /**
     * Destructor.
     */
    ~KLMDatabaseImporter();

    /**
     * Set interval of validity.
     */
    void setIOV(int experimentLow, int runLow, int experimentHigh, int runHigh);

    /**
     * Import scintillator simulation parameters.
     */
    void importScintillatorSimulationParameters();

    /**
     * Import time conversion parameters.
     * @param[in] timeConversion Time conversion parameters.
     */
    void importTimeConversion(const KLMTimeConversion* timeConversion);

  private:

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
