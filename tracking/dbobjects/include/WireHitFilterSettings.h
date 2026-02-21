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

    /** Set filter */
    void setFilterName(const std::string& name)
    {
      m_FilterName = name;
    }

    /** Set swich layer */
    void setMVASwitchSuperLayer(int superLayer)
    {
      m_CombinedFilterMVASwitchSuperLayer = superLayer;
    }

    /** Get filter name */
    std::string getFilterName() const
    {
      return m_FilterName;
    }

    /** Get filter name */
    int getMVASwitchSuperLayer() const
    {
      return m_CombinedFilterMVASwitchSuperLayer;
    }

  private:
    /** Filter name */
    std::string m_FilterName = "combined";
    /** For the combined filter, super layer number to switch to MVA */
    int m_CombinedFilterMVASwitchSuperLayer = 2;

    ClassDef(WireHitFilterSettings, 1);
  };
}
