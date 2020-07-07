/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dbobjects/KLMElectronicsChannel.h>

/* C++ headers. */
#include <map>

namespace Belle2 {

  /**
   * BKLM electronics map.
   */
  class KLMElectronicsMap: public TObject {

  public:

    /**
     * Constructor.
     */
    KLMElectronicsMap();

    /**
     * Destructor.
     */
    ~KLMElectronicsMap();

    /**
     * Get detector channel by electronics channel.
     * @param[in] electronicsChannel Electronics channel.
     * @return Detector channel, nullptr if not found.
     */
    const uint16_t* getDetectorChannel(
      KLMElectronicsChannel* electronicsChannel) const;

    /**
     * Get electronics channel by detector channel.
     * @param[in] detectorChannel Detector channel.
     * @return Electronics channel, nullptr if not found.
     */
    const KLMElectronicsChannel* getElectronicsChannel(
      uint16_t detectorChannel) const;

    /**
     * Add channel.
     * @param[in] detectorChannel Detector channel.
     * @param[in] copper          Copper.
     * @param[in] slot            Slot.
     * @param[in] lane            Lane.
     * @param[in] axis            Axis.
     * @param[in] channel         Channel.
     */
    void addChannel(uint16_t detectorChannel, int copper, int slot, int lane,
                    int axis, int channel);

  private:

    /** Detector channel - electronics channel map. */
    std::map<uint16_t, KLMElectronicsChannel> m_MapDetectorElectronics;

    /** Electronics channel - detector channel map. */
    std::map<KLMElectronicsChannel, uint16_t> m_MapElectronicsDetector;

    /** Class version. */
    ClassDef(Belle2::KLMElectronicsMap, 1);

  };

}
