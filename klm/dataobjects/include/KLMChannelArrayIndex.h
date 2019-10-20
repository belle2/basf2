/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* C++ headers. */
#include <map>

namespace Belle2 {

  /**
   * KLM channel array index.
   */
  class KLMChannelArrayIndex {

  public:

    /**
     * Instantiation.
     */
    static const KLMChannelArrayIndex& Instance();

    /**
     * Get channel index.
     * @param[in] number Channel number.
     */
    uint16_t getIndex(uint16_t number) const;

    /**
     * Get channel number.
     * @param[in] index Channel index.
     */
    uint16_t getNumber(uint16_t index) const;

    /**
     * Get number of channels.
     */
    uint16_t getNChannels() const
    {
      return m_NChannels;
    }

  private:

    /**
     * Constructor.
     */
    KLMChannelArrayIndex();

    /**
     * Destructor.
     */
    ~KLMChannelArrayIndex();

    /** Map KLM channel number - KLM channel index. */
    std::map<uint16_t, uint16_t> m_MapNumberIndex;

    /** Map KLM channel index - KLM channel number. */
    std::map<uint16_t, uint16_t> m_MapIndexNumber;

    /** Number of channels. */
    uint16_t m_NChannels;

  };

}
