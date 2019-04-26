/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// basf2 includes
#include <bklm/dataobjects/BKLMStatus.h>
#include <framework/logging/Logger.h>

// ROOT includes
#include <TObject.h>

namespace Belle2 {

  /**
   * DBObject used to store the efficiencies of BKLM strips.
   * Needed for the Muid calculation.
   */
  class BKLMStripEfficiency: public TObject {
  public:

    /**
     * Default constructor
     */
    BKLMStripEfficiency()
    {}

    /**
     * Set efficiency and relative error for a single strip using the geometrical infos
     * @param isForward 1 for BF sectors, 0 for BB sectors
     * @param sector sector number
     * @param layer layer number
     * @param isPhi 1 for phi plane, 0 for z plane
     * @param strip strip number
     * @param efficiency efficiency of the strip
     * @param efficiencyError error on the efficiency of the strip
     */
    void setEfficiency(int isForward, int sector, int layer, int isPhi, int strip, float efficiency, float efficiencyError = 0.)
    {
      m_efficiency[isForward][sector][layer][isPhi][strip] = efficiency;
      m_efficiencyError[isForward][sector][layer][isPhi][strip] = efficiencyError;
    }

    /**
     * Returns efficiency of a given strip using the geometrical infos
     * @param isForward 1 for BF sectors, 0 for BB sectors
     * @param sector sector number
     * @param layer layer number
     * @param isPhi 1 for phi plane, 0 for z plane
     * @param strip strip number
     */
    float getEfficiency(int isForward, int sector, int layer, int isPhi, int strip) const
    {
      return m_efficiency[isForward][sector][layer][isPhi][strip];
    }

    /**
     * Returns error on efficiency of a given strip using the geometrical infos
     * @param isForward 1 for BF sectors, 0 for BB sectors
     * @param sector sector number
     * @param layer layer number
     * @param isPhi 1 for phi plane, 0 for z plane
     * @param strip strip number
     */
    float getEfficiencyError(int isForward, int sector, int layer, int isPhi, int strip) const
    {
      return m_efficiencyError[isForward][sector][layer][isPhi][strip];
    }

  private:

    float m_efficiency[BKLM_END_BIT + 1][BKLM_SECTOR_BIT + 1][BKLM_LAYER_BIT + 1][BKLM_PLANE_BIT + 1][BKLM_STRIP_BIT + 1] = {1}; /**< strip efficiency */
    float m_efficiencyError[BKLM_END_BIT + 1][BKLM_SECTOR_BIT + 1][BKLM_LAYER_BIT + 1][BKLM_PLANE_BIT + 1][BKLM_STRIP_BIT + 1] = {0}; /**< strip efficiency */

    ClassDef(BKLMStripEfficiency, 1); /**< ClassDef */
  };

} // end namespace Belle2

