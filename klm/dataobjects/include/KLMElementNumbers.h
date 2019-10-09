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

/* KLM headers. */
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>

namespace Belle2 {

  /**
   * KLM element numbers.
   */
  class KLMElementNumbers {

  public:

    /**
     * Subdetector constants.
     */
    enum Subdetector {

      /** BKLM. */
      c_BKLM = 1,

      /** EKLM. */
      c_EKLM = 2,

    };

    /**
     * Instantiation.
     */
    static const KLMElementNumbers& Instance();

    /**
     * Get channel number for BKLM.
     * @param[in] section Forward (1) or backward (0) BKLM.
     * @param[in] sector  Sector (1-based).
     * @param[in] layer   Layer (1-based).
     * @param[in] plane   Plane (0-based).
     * @param[in] strip   Strip (1-based).
     */
    uint16_t channelNumberBKLM(int section, int sector, int layer, int plane,
                               int strip) const;

    /**
     * Get channel number for BKLM.
     * @param[in] bklmChannel BKLM channel.
     */
    uint16_t channelNumberBKLM(int bklmChannel) const;

    /**
     * Get channel number for EKLM.
     * @param[in] section Section number.
     * @param[in] sector  Sector number.
     * @param[in] layer   Layer number.
     * @param[in] plane   Plane number.
     * @param[in] strip   Strip number.
     */
    uint16_t channelNumberEKLM(int section, int sector, int layer, int plane,
                               int strip) const;

    /**
     * Get channel number for EKLM.
     * @param[in] eklmStrip EKLM strip number.
     */
    uint16_t channelNumberEKLM(int eklmStrip) const;

    /**
     * Determine whether a given channel is in BKLM.
     * This function also works for modules amd sectors.
     * @param[in] channel KLM channel number.
     */
    bool isBKLMChannel(uint16_t channel) const;

    /**
     * Determine whether a given channel is in EKLM.
     * This function also works for modules amd sectors.
     * @param[in] channel KLM channel number.
     */
    bool isEKLMChannel(uint16_t channel) const;

    /**
     * Get local BKLM channel number.
     * This function also works for modules amd sectors.
     * @param[in] channel KLM channel number.
     */
    int localChannelNumberBKLM(uint16_t channel) const;

    /**
     * Get local EKLM channel number.
     * This function also works for modules amd sectors.
     * @param[in] channel KLM channel number.
     */
    int localChannelNumberEKLM(uint16_t channel) const;

    /**
     * Get element numbers by channel number.
     * @param[in]  channel     KLM channel number.
     * @param[out] subdetector Subdetector.
     * @param[out] section     Section.
     * @param[out] sector      Sector.
     * @param[out] layer       Layer.
     * @param[out] plane       Plane.
     * @param[out] strip       Strip.
     */
    void channelNumberToElementNumbers(
      uint16_t channel, int* subdetector, int* section, int* sector,
      int* layer, int* plane, int* strip) const;

    /**
     * Get module number.
     * @param[in] subdetector Subdetector.
     * @param[in] section     Section.
     * @param[in] sector      Sector (1-based).
     * @param[in] layer       Layer (1-based).
     */
    uint16_t moduleNumber(int subdetector, int section, int sector,
                          int layer) const;

    /**
     * Get module number for BKLM.
     * @param[in] section Forward (1) or backward (0) BKLM.
     * @param[in] sector  Sector (1-based).
     * @param[in] layer   Layer (1-based).
     */
    uint16_t moduleNumberBKLM(int section, int sector, int layer) const;

    /**
     * Get module number for EKLM.
     * @param[in] section Section number.
     * @param[in] sector  Sector number.
     * @param[in] layer   Layer number.
     */
    uint16_t moduleNumberEKLM(int section, int sector, int layer) const;

    /**
     * Get element numbers by module number.
     * @param[in]  module      KLM module number.
     * @param[out] subdetector Subdetector.
     * @param[out] section     Section.
     * @param[out] sector      Sector.
     * @param[out] layer       Layer.
     */
    void moduleNumberToElementNumbers(
      uint16_t module, int* subdetector, int* section, int* sector,
      int* layer) const;

    /**
     * Get number of channels in module.
     * @param[in] module KLM module number.
     */
    unsigned int getNChannelsModule(uint16_t module) const;

    /**
     * Get sector number for BKLM.
     * @param[in] section Forward (1) or backward (0) BKLM.
     * @param[in] sector  Sector (1-based).
     */
    uint16_t sectorNumberBKLM(int section, int sector) const;

    /**
     * Get sector number for EKLM.
     * @param[in] section Section number.
     * @param[in] sector  Sector number.
     */
    uint16_t sectorNumberEKLM(int section, int sector) const;

  private:

    /**
     * Constructor.
     */
    KLMElementNumbers();

    /**
     * Destructor.
     */
    ~KLMElementNumbers();

    /** BKLM offset. */
    static constexpr uint16_t m_BKLMOffset = 0x8000;

    /** EKLM element numbers. */
    const EKLM::ElementNumbersSingleton* m_ElementNumbersEKLM;

  };

}
