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

/* KLM headers. */
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dbobjects/KLMElectronicsChannel.h>

namespace Belle2 {

  /**
   * KLM database importer.
   */
  class KLMElectronicsMapImporter {

  public:

    /**
     * Constructor.
     */
    KLMElectronicsMapImporter();

    /**
     * Destructor.
     */
    ~KLMElectronicsMapImporter();

    /**
     * Set interval of validity.
     */
    void setIOV(int experimentLow, int runLow, int experimentHigh, int runHigh);

    /**
     * Clear electronics map (to be able to import its multiple versions).
     */
    void clearElectronicsMap();

    /**
     * Load BKLM electronics map.
     *
     * @param[in] version
     * map version:
     * 1) Before experiment 10.
     * 2) Experiment 10 and later (mapping in chimney sector has changed)
     */
    void loadBKLMElectronicsMap(int version);

    /**
     * Load EKLM electronics map.
     *
     * @param[in] version
     * Map version:
     * 1) Phase 2, experiment 3. Connection of cables was wrong
     *    for backward sectors 2 and 3.
     * 2) Phase 3, starting from experiment 4.
     *
     * @param[in] mc
     * MC or data (MC does not have occasional cable switches that exist
     * in the data).
     */
    void loadEKLMElectronicsMap(int version, bool mc);

    /**
     * Set non-default lane for all channels in a module.
     * @param[in] subdetector Subdetector.
     * @param[in] section     Section.
     * @param[in] sector      Sector.
     * @param[in] layer       Layer.
     * @param[in] lane        Lane.
     */
    void setLane(
      int subdetector, int section, int sector, int layer, int lane);

    /**
     * Set non-default lane for all channels in a plane.
     * @param[in] subdetector Subdetector.
     * @param[in] section     Section.
     * @param[in] sector      Sector.
     * @param[in] layer       Layer.
     * @param[in] plane       Plane.
     * @param[in] lane        Lane.
     */
    void setLane(
      int subdetector, int section, int sector, int layer, int plane, int lane);

    /**
     * Import electronics map.
     */
    void importElectronicsMap();

  private:

    /**
     * Get EKLM firmware strip number by software strip number.
     * @param[in] stripFirmware Firmware strip number.
     */
    int getEKLMStripFirmwareBySoftware(int stripSoftware) const;

    /**
     * Add EKLM electronics map lane.
     * @param[in] section Section.
     * @param[in] sector  Sector.
     * @param[in] layer   Layer.
     * @param[in] copper  Copper.
     * @param[in] slot    Slot.
     * @param[in] lane    Lane.
     */
    void addEKLMLane(
      int section, int sector, int layer, int copper, int slot, int axis);

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** Data for creation of the electronics map. */
    std::vector< std::pair<uint16_t, KLMElectronicsChannel> > m_ElectronicsChannels;

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
