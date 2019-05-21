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
#include <bklm/dataobjects/BKLMElementNumbers.h>
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
     * Set efficiency and relative error for a single strip using directly the stripId
     * @param stripId strip identifier
     * @param efficiency efficiency of the strip
     * @param efficiencyError error on the efficiency of the strip
     */
    void setEfficiency(int stripId, float efficiency, float efficiencyError = 0.)
    {
      m_efficiency.insert(std::pair<int, float>(stripId, efficiency));
      m_efficiencyError.insert(std::pair<int, float>(stripId, efficiencyError));
    }

    /**
     * Set efficiency and relative error for a single strip using the geometrical infos
     * @param isForward 1 for BF sectors, 0 for BB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     * @param efficiency efficiency of the strip
     * @param efficiencyError error on the efficiency of the strip
     */
    void setEfficiency(int isForward, int sector, int layer, int plane, int strip, float efficiency, float efficiencyError = 0.)
    {
      int stripId = BKLMElementNumbers::channelNumber(isForward, sector, layer, plane, strip);
      setEfficiency(stripId, efficiency, efficiencyError);
    }

    /**
     * Returns efficiency of a given strip using directly the stripId
     * @param stripId strip identifier
     */
    float getEfficiency(int stripId) const
    {
      auto search = m_efficiency.find(stripId);
      if (search == m_efficiency.end())
        return std::numeric_limits<float>::quiet_NaN();
      return search->second;
    }

    /**
     * Returns efficiency of a given strip using the geometrical infos
     * @param isForward 1 for BF sectors, 0 for BB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     */
    float getEfficiency(int isForward, int sector, int layer, int plane, int strip) const
    {
      int stripId = BKLMElementNumbers::channelNumber(isForward, sector, layer, plane, strip);
      return getEfficiency(stripId);
    }

    /**
     * Returns error on efficiency of a given strip using directly the stripId
     * @param stripId strip identifier
     */
    float getEfficiencyError(int stripId) const
    {
      auto search = m_efficiencyError.find(stripId);
      if (search == m_efficiencyError.end())
        return std::numeric_limits<float>::quiet_NaN();
      return search->second;
    }

    /**
     * Returns error on efficiency of a given strip using the geometrical infos
     * @param isForward 1 for BF sectors, 0 for BB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     */
    float getEfficiencyError(int isForward, int sector, int layer, int plane, int strip) const
    {
      int stripId = BKLMElementNumbers::channelNumber(isForward, sector, layer, plane, strip);
      return getEfficiencyError(stripId);
    }

  private:

    std::map<int, float> m_efficiency; /**< strip efficiency */
    std::map<int, float> m_efficiencyError; /**< strip efficiency error */

    ClassDef(BKLMStripEfficiency, 1); /**< ClassDef */
  };

} // end namespace Belle2
