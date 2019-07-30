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
   * KLM sector array index.
   */
  class KLMSectorArrayIndex {

  public:

    /**
     * Instantiation.
     */
    static const KLMSectorArrayIndex& Instance();

    /**
     * Get sector index.
     * @param[in] number Sector number.
     */
    uint16_t getIndex(uint16_t number) const;

    /**
     * Get sector number.
     * @param[in] index Sector index.
     */
    uint16_t getNumber(uint16_t index) const;

    /**
     * Get number of sectors.
     */
    uint16_t getNSectors() const
    {
      return m_NSectors;
    }

  private:

    /**
     * Constructor.
     */
    KLMSectorArrayIndex();

    /**
     * Destructor.
     */
    ~KLMSectorArrayIndex();

    /** Map KLM sector number - KLM sector index. */
    std::map<uint16_t, uint16_t> m_MapNumberIndex;

    /** Map KLM sector index - KLM sector number. */
    std::map<uint16_t, uint16_t> m_MapIndexNumber;

    /** Number of sectors. */
    uint16_t m_NSectors;

  };

}
