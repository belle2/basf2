/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <bitset>

namespace Belle2 {

  /**
   * Class to store unpacker error flags (one per boardstack)
   */
  class TOPUnpackerErrors : public RelationsObject {

  public:

    /**
     * Default constructor
     */
    TOPUnpackerErrors()
    {}

    /**
     * Sets unpacker error flag
     * @parameter BS boardstack number within the TOP detector (0-based)
     */
    void setErrorFlag(unsigned BS)
    {
      if (BS < m_errorFlags.size()) m_errorFlags.set(BS);
    }

    /**
     * Returns unpacker error flags
     * @return unpacker error flags (bit# = boardstack number within the TOP detector)
     */
    const std::bitset<64>& getErrorFlags() const {return m_errorFlags;}

  private:

    std::bitset<64> m_errorFlags;  /**< unpacker error flags (one per boardstack) */

    ClassDef(TOPUnpackerErrors, 1); /**< ClassDef */

  };

} // end namespace Belle2

