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

/* C++ headers. */
#include <map>

/* Belle2 headers. */
#include <klm/bklm/dbobjects/BKLMElectronicsChannel.h>

namespace Belle2 {

  /**
   * BKLM electronics map.
   */
  class BKLMElectronicsMap: public TObject {

  public:

    /**
     * Constructor.
     */
    BKLMElectronicsMap();

    /**
     * Destructor.
     */
    ~BKLMElectronicsMap();

    /**
     * Get detector channel by electronics channel.
     * @param[in] electronicsChannel Electronics channel.
     * @return Detector channel, nullptr if not found.
     */
    const uint16_t* getDetectorChannel(
      BKLMElectronicsChannel* electronicsChannel) const;

    /**
     * Get electronics channel by detector channel.
     * @param[in] detectorChannel Detector channel.
     * @return Electronics channel, nullptr if not found.
     */
    const BKLMElectronicsChannel* getElectronicsChannel(
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
    std::map<uint16_t, BKLMElectronicsChannel> m_MapDetectorElectronics;

    /** Electronics channel - detector channel map. */
    std::map<BKLMElectronicsChannel, uint16_t> m_MapElectronicsDetector;

    /** Class version. */
    ClassDef(Belle2::BKLMElectronicsMap, 1);

  };

}
