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
#include <klm/eklm/dbobjects/EKLMAlignment.h>
#include <klm/eklm/dbobjects/EKLMElectronicsMap.h>
#include <framework/database/DBImportObjPtr.h>

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
     * Import digitization parameters.
     */
    void importDigitizationParameters();

    /**
     * Import reconstruction parameters.
     */
    void importReconstructionParameters();

    /**
     * Import simulation parameters.
     */
    void importSimulationParameters();

    /**
     * Load default displacement data (zeros).
     */
    void loadDefaultDisplacement();

    /**
     * Set sector displacement.
     * @param[in] section    Section number.
     * @param[in] layer      Layer number.
     * @param[in] sector     Sector number.
     * @param[in] deltaU     Shift in U.
     * @param[in] deltaV     Shift in V.
     * @param[in] deltaGamma Rotation in Gamma.
     */
    void setSectorDisplacement(int section, int layer, int sector,
                               float deltaU, float deltaV, float deltaGamma);

    /**
     * Set segment displacement.
     * @param[in] section    Section number.
     * @param[in] layer      Layer number.
     * @param[in] sector     Sector number.
     * @param[in] plane      Plane number.
     * @param[in] segment    Segment number.
     * @param[in] deltaU     Shift in U.
     * @param[in] deltaV     Shift in V.
     * @param[in] deltaGamma Rotation in Gamma.
     */
    void setSegmentDisplacement(
      int section, int layer, int sector, int plane, int segment,
      float deltaU, float deltaV, float deltaGamma);

    /**
     * Import displacement data.
     */
    void importDisplacement();

    /**
     * Import electronics map.
     * @param[in] electronicsMap EKLM electronics map.
     */
    void importElectronicsMap(const EKLMElectronicsMap* electronicsMap);

  private:

    /** Displacement. */
    DBImportObjPtr<EKLMAlignment> m_Displacement;

    /** Electronics map. */
    DBImportObjPtr<EKLMElectronicsMap> m_ElectronicsMap;

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
