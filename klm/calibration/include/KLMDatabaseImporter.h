/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Giacomo De Pietro                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <klm/dbobjects/KLMChannelStatus.h>
#include <klm/dbobjects/KLMScintillatorDigitizationParameters.h>
#include <klm/dbobjects/KLMStripEfficiency.h>
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
     * Import channel status.
     */
    void importChannelStatus(const KLMChannelStatus* channelStatus);

    /**
     * Import scintillator simulation parameters.
     */
    void importScintillatorDigitizationParameters(
      const KLMScintillatorDigitizationParameters* digitizationParameters);

    /**
     * Import time conversion parameters.
     * @param[in] timeConversion Time conversion parameters.
     */
    void importTimeConversion(const KLMTimeConversion* timeConversion);

    /**
     * Import strip efficiencies.
     * @param[in] fileName Name of the ROOT file containing the efficiencies.
     */
    void importStripEfficiency(std::string fileName);

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
