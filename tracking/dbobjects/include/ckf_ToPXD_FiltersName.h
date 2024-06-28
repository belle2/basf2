/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>

namespace Belle2 {

  /** The payload containing the toPXD CKF
   *  hitFilter and seedFilter names
   */
  class ckf_ToPXD_FiltersName: public TObject {

  public:

    /** Default constructor */
    ckf_ToPXD_FiltersName() {}

    /** Destructor */
    ~ckf_ToPXD_FiltersName() {}

    /** Set the hitFilterName
     * @param hitFilterName name of the hitFilter
     */
    void setHitFilterName(const std::string hitFilterName)
    {
      m_hitFilterName = hitFilterName;
    }
    /** Set the seedFilterName
     * @param seedFilterName name of the seedFilter
     */
    void setSeedFilterName(const std::string seedFilterName)
    {
      m_seedFilterName = seedFilterName;
    }


    /** Get the hitFilterName
     */
    std::string getHitFilterName() const
    {
      return m_hitFilterName;
    }

    /** Get the seedFilterName
     */
    std::string getSeedFilterName() const
    {
      return m_seedFilterName;
    }

  private:

    /** The hitFilterName */
    std::string m_hitFilterName = "not set";

    /** The seedFilterName */
    std::string m_seedFilterName = "not set";

    ClassDef(ckf_ToPXD_FiltersName, 1);  /**< ClassDef, necessary for ROOT */
  };
}
