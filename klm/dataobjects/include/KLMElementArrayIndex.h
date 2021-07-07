/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

/* C++ headers. */
#include <map>

namespace Belle2 {

  /**
   * KLM element array index.
   */
  class KLMElementArrayIndex {

  public:

    /**
     * Constructor.
     */
    explicit KLMElementArrayIndex(enum KLMChannelIndex::IndexLevel indexLevel);

    /**
     * Destructor.
     */
    virtual ~KLMElementArrayIndex();

    /**
     * Get element index.
     * @param[in] number Element number.
     */
    uint16_t getIndex(uint16_t number) const;

    /**
     * Get element number.
     * @param[in] index Element index.
     */
    uint16_t getNumber(uint16_t index) const;

    /**
     * Get number of elements.
     */
    uint16_t getNElements() const
    {
      return m_NElements;
    }

  private:

    /** Map KLM element number - KLM element index. */
    std::map<uint16_t, uint16_t> m_MapNumberIndex;

    /** Map KLM element index - KLM element number. */
    std::map<uint16_t, uint16_t> m_MapIndexNumber;

    /** Number of elements. */
    uint16_t m_NElements;

  };

}
