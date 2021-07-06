/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMElementNumberDefinitions.h>
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
    const KLMChannelNumber* getDetectorChannel(
      KLMElectronicsChannel* electronicsChannel) const;

    /**
     * Get electronics channel by detector channel.
     * @param[in] detectorChannel Detector channel.
     * @return Electronics channel, nullptr if not found.
     */
    const KLMElectronicsChannel* getElectronicsChannel(
      KLMChannelNumber detectorChannel) const;

    /**
     * Add channel.
     * @param[in] detectorChannel Detector channel.
     * @param[in] copper          Copper.
     * @param[in] slot            Slot.
     * @param[in] lane            Lane.
     * @param[in] axis            Axis.
     * @param[in] channel         Channel.
     */
    void addChannel(KLMChannelNumber detectorChannel, int copper, int slot,
                    int lane, int axis, int channel);

  private:

    /** Detector channel - electronics channel map. */
    std::map<KLMChannelNumber, KLMElectronicsChannel> m_MapDetectorElectronics;

    /** Electronics channel - detector channel map. */
    std::map<KLMElectronicsChannel, KLMChannelNumber> m_MapElectronicsDetector;

    /** Class version. */
    ClassDef(Belle2::KLMElectronicsMap, 1);

  };

}
