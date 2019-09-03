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
#include <klm/bklm/dbobjects/BKLMAlignment.h>
#include <klm/dbobjects/KLMChannelStatus.h>
#include <klm/dbobjects/KLMScintillatorDigitizationParameters.h>
#include <klm/dbobjects/KLMStripEfficiency.h>
#include <klm/dbobjects/KLMTimeConversion.h>
#include <klm/eklm/dbobjects/EKLMAlignment.h>
#include <klm/eklm/dbobjects/EKLMSegmentAlignment.h>

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
     * Load strip efficiencies.
     *
     * @param[out] stripEfficiency
     * Strip efficiency.
     *
     * @param[in] fileName
     * Name of the ROOT file containing the efficiencies.
     */
    void loadStripEfficiency(KLMStripEfficiency* stripEfficiency,
                             std::string fileName);

    /**
     * Import strip efficiencies.
     * @param[in] stripEfficiency Strip efficiency.
     */
    void importStripEfficiency(const KLMStripEfficiency* stripEfficiency);

    /**
     * Import alignment.
     * @param[in] bklmAlignment        BKLM alignment.
     * @param[in] eklmAlignment        EKLM alignment.
     * @param[in] eklmSegmentAlignment EKLM segment alignment.
     */
    void importAlignment(const BKLMAlignment* bklmAlignment,
                         const EKLMAlignment* eklmAlignment,
                         const EKLMSegmentAlignment* eklmSegmentAlignment);

  private:

    /**
     * Import BKLM alignment.
     * @param[in] bklmAlignment BKLM alignment.
     */
    void importBKLMAlignment(const BKLMAlignment* bklmAlignment);

    /**
     * Import EKLM alignment.
     * @param[in] eklmAlignment EKLM alignment.
     */
    void importEKLMAlignment(const EKLMAlignment* eklmAlignment);

    /**
     * Import EKLM segment alignment.
     * @param[in] eklmSegmentAlignment EKLM segment alignment.
     */
    void importEKLMSegmentAlignment(
      const EKLMSegmentAlignment* eklmSegmentAlignment);

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
