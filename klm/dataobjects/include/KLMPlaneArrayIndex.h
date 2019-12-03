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
   * KLM plane array index.
   */
  class KLMPlaneArrayIndex {

  public:

    /**
     * Instantiation.
     */
    static const KLMPlaneArrayIndex& Instance();

    /**
     * Get plane index.
     * @param[in] number Plane number.
     */
    uint16_t getIndex(uint16_t number) const;

    /**
     * Get plane number.
     * @param[in] index Plane index.
     */
    uint16_t getNumber(uint16_t index) const;

    /**
     * Get number of planes.
     */
    uint16_t getNPlanes() const
    {
      return m_NPlanes;
    }

  private:

    /**
     * Constructor.
     */
    KLMPlaneArrayIndex();

    /**
     * Destructor.
     */
    ~KLMPlaneArrayIndex();

    /** Map KLM plane number - KLM plane index. */
    std::map<uint16_t, uint16_t> m_MapNumberIndex;

    /** Map KLM plane index - KLM plane number. */
    std::map<uint16_t, uint16_t> m_MapIndexNumber;

    /** Number of planes. */
    uint16_t m_NPlanes;

  };

}
