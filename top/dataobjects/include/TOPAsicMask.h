/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <vector>

namespace Belle2 {

  /**
   * Class to store bit fields of masked ASICs, as reported in raw data.
   */
  class TOPAsicMask: public RelationsObject {

  public:

    /**
     * Default constructor
     */
    TOPAsicMask()
    {}

    /**
     * Sets bit fields of masked ASIC's. Bit is set if ASIC is masked.
     * @param masks vector of 64 16-bit-long bit fields (one bit field per boardstack)
     */
    void set(const std::vector<unsigned short>& masks) {m_masks = masks;}

    /**
     * Returns bit fields of masked ASIC's. Bit is set if ASIC is masked.
     * @return vector of 64 16-bit-long bit fields (one bit field per boardstack)
     */
    const std::vector<unsigned short>& get() const {return m_masks;}

    /**
     * Returns true if channel is not explicitely labeled as masked.
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     */
    bool isActive(int moduleID, unsigned channel) const;

  private:

    std::vector<unsigned short> m_masks; /**< bit fields of masked ASIC's (one per BS) */

    ClassDef(TOPAsicMask, 1); /**< ClassDef */

  };


} // end namespace Belle2
