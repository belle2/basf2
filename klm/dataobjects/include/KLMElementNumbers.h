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
#include <eklm/dataobjects/ElementNumbersSingleton.h>

namespace Belle2 {

  /**
   * KLM element numbers.
   */
  class KLMElementNumbers {

  public:

    /**
     * Instantiation.
     */
    static const KLMElementNumbers& Instance();

    /**
     * Get channel number for BKLM.
     * @param[in] forward Forward (1) or backward (0) BKLM.
     * @param[in] sector  Sector (1-based).
     * @param[in] layer   Layer (1-based).
     * @param[in] plane   Plane (0-based).
     * @param[in] strip   Strip (1-based).
     */
    uint16_t channelNumberBKLM(int forward, int sector, int layer, int plane,
                               int strip) const;

    /**
     * Get channel number for BKLM.
     * @param[in] bklmChannel BKLM channel.
     */
    uint16_t channelNumberBKLM(int bklmChannel) const;

    /**
     * Get channel number for EKLM.
     * @param[in] endcap Endcap number.
     * @param[in] sector Sector number.
     * @param[in] layer  Layer number.
     * @param[in] plane  Plane number.
     * @param[in] strip  Strip number.
     */
    uint16_t channelNumberEKLM(int endcap, int sector, int layer, int plane,
                               int strip) const;

    /**
     * Get channel number for EKLM.
     * @param[in] eklmStrip EKLM strip number.
     */
    uint16_t channelNumberEKLM(int eklmStrip) const;

    /**
     * Determine whether a given channel is in BKLM.
     * @param[in] channel KLM channel number.
     */
    bool isBKLMChannel(uint16_t channel) const;

    /**
     * Determine whether a given channel is in EKLM.
     * @param[in] channel KLM channel number.
     */
    bool isEKLMChannel(uint16_t channel) const;

    /**
     * Get local BKLM channel number.
     * @param[in] channel KLM channel number.
     */
    int localChannelNumberBKLM(uint16_t channel) const;

    /**
     * Get local EKLM channel number.
     * @param[in] channel KLM channel number.
     */
    int localChannelNumberEKLM(uint16_t channel) const;

    /**
     * Get module number for BKLM.
     * @param[in] forward Forward (1) or backward (0) BKLM.
     * @param[in] sector  Sector (1-based).
     * @param[in] layer   Layer (1-based).
     */
    uint16_t moduleNumberBKLM(int forward, int sector, int layer) const;

    /**
     * Get module number for EKLM.
     * @param[in] endcap Endcap number.
     * @param[in] sector Sector number.
     * @param[in] layer  Layer number.
     */
    uint16_t moduleNumberEKLM(int endcap, int sector, int layer) const;

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
