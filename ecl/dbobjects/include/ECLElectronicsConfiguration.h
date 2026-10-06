/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * DB object to store which ECL readout electronics is used for the given
   * run/exp interval: the current or the upgraded one.
   */
  class ECLElectronicsConfiguration: public TObject {

  public:

    /** Constructor. */
    ECLElectronicsConfiguration() {}

    /** Destructor. */
    ~ECLElectronicsConfiguration() {}

    /** Set whether the upgraded electronics is used. */
    void setUpgrade(bool upgrade)
    {
      m_upgrade = upgrade;
    }

    /** Get whether the upgraded electronics is used. */
    bool getUpgrade() const
    {
      return m_upgrade;
    }

  private:

    /** Upgraded electronics is used. */
    bool m_upgrade = false;

    /** Class version. */
    ClassDef(ECLElectronicsConfiguration, 1);

  };

}
