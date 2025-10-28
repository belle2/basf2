/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * Class to store TOP injection veto flag
   */
  class TOPInjectionVeto : public RelationsObject {

  public:

    /**
     * Default constructor
     */
    TOPInjectionVeto()
    {}

    /**
     * Sets the veto flag on first occurence of true if called multiple times (like in TOPUnpacker).
     * @param flag TOP injection veto flag
     */
    void set(bool flag = true) {m_flag = m_flag or flag;}

    /**
     * Resets the veto flag.
     */
    void reset() {m_flag = false;}

    /**
     * Returns the veto flag.
     * @return TOP injection veto flag
     */
    bool get() const {return m_flag;}

  private:

    bool m_flag = false; /**< injection veto flag */

    ClassDef(TOPInjectionVeto, 1); /**< ClassDef */

  };

} // end namespace Belle2


