/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <string>
namespace Belle2 {
  /**
   * Database object to keep wirehit filter name and parameters
  */

  class WireHitFilterSettings: public TObject {
  public:
    /**
     * Default constructor
     */
    WireHitFilterSettings() = default;
    /**
     * Destructor
     */
    ~WireHitFilterSettings() = default;

    /** Set from which SuperLayer to start using the MVA filter. The previous SuperLayers will use the DB cuts */
    void setMVASwitchSuperLayer(int superLayer)
    {
      m_CombinedFilterMVASwitchSuperLayer = superLayer;
    }

    /** Get from which SuperLayer to start using the MVA filter. The previous SuperLayers will use the DB cuts */
    int getMVASwitchSuperLayer() const
    {
      return m_CombinedFilterMVASwitchSuperLayer;
    }

  private:
    /** For the combined filter, super layer number from which to start using MVA */
    int m_CombinedFilterMVASwitchSuperLayer = 2;

    ClassDef(WireHitFilterSettings, 1); /**< ClassDef, necessary for ROOT */
  };
}
