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

// basf2 headers
#include <klm/dataobjects/KLMElementNumbers.h>

/* C++ headers. */
#include <cstdint>
#include <map>

// ROOT headers
#include <TObject.h>

namespace Belle2 {

  /**
   * DBObject used to store the efficiencies of KLM strips.
   * Needed for the Muid calculation.
   */
  class KLMStripEfficiency: public TObject {
  public:

    /**
     * Default constructor
     */
    KLMStripEfficiency()
    {}

    /**
     * Set efficiency and relative error for a single KLM strip using directly the stripId
     * @param stripId KLM strip identifier
     * @param efficiency efficiency of the strip
     * @param efficiencyError error on the efficiency of the strip
     */
    void setEfficiency(uint16_t stripId, float efficiency, float efficiencyError = 0.)
    {
      m_efficiency.insert(std::pair<uint16_t, float>(stripId, efficiency));
      m_efficiencyError.insert(std::pair<uint16_t, float>(stripId, efficiencyError));
    }

    /**
     * Set efficiency and relative error for a single BKLM strip using the geometrical infos
     * @param forward 1 for BF sectors, 0 for BB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     * @param efficiency efficiency of the strip
     * @param efficiencyError error on the efficiency of the strip
     */
    void setBarrelEfficiency(int forward, int sector, int layer, int plane, int strip, float efficiency, float efficiencyError = 0.)
    {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      uint16_t stripId = elementNumbers->channelNumberBKLM(forward, sector, layer, plane, strip);
      setEfficiency(stripId, efficiency, efficiencyError);
    }

    /**
     * Set efficiency and relative error for a single EKLM strip using the geometrical infos
     * @param section 2 for EF sectors, 1 for EB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     * @param efficiency efficiency of the strip
     * @param efficiencyError error on the efficiency of the strip
     */
    void setEndcapEfficiency(int section, int sector, int layer, int plane, int strip, float efficiency, float efficiencyError = 0.)
    {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      uint16_t stripId = elementNumbers->channelNumberEKLM(section, sector, layer, plane, strip);
      setEfficiency(stripId, efficiency, efficiencyError);
    }

    /**
     * Returns efficiency of a given KLM strip using directly the stripId
     * @param stripId KLM strip identifier
     */
    float getEfficiency(uint16_t stripId) const
    {
      auto search = m_efficiency.find(stripId);
      if (search == m_efficiency.end())
        return std::numeric_limits<float>::quiet_NaN();
      return search->second;
    }

    /**
     * Returns efficiency of a given BKLM strip using the geometrical infos
     * @param forward 1 for BF sectors, 0 for BB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     */
    float getBarrelEfficiency(int forward, int sector, int layer, int plane, int strip) const
    {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      uint16_t stripId = elementNumbers->channelNumberBKLM(forward, sector, layer, plane, strip);
      return getEfficiency(stripId);
    }

    /**
     * Returns efficiency of a given EKLM strip using the geometrical infos
     * @param section 2 for EF sectors, 1 for EB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     */
    float getEndcapEfficiency(int section, int sector, int layer, int plane, int strip) const
    {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      uint16_t stripId = elementNumbers->channelNumberEKLM(section, sector, layer, plane, strip);
      return getEfficiency(stripId);
    }

    /**
     * Returns error on efficiency of a given KLM strip using directly the stripId
     * @param stripId KLM strip identifier
     */
    float getEfficiencyError(uint16_t stripId) const
    {
      auto search = m_efficiencyError.find(stripId);
      if (search == m_efficiencyError.end())
        return std::numeric_limits<float>::quiet_NaN();
      return search->second;
    }

    /**
     * Returns error on efficiency of a given BKLM strip using the geometrical infos
     * @param forward 1 for BF sectors, 0 for BB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     */
    float getBarrelEfficiencyError(int forward, int sector, int layer, int plane, int strip) const
    {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      uint16_t stripId = elementNumbers->channelNumberBKLM(forward, sector, layer, plane, strip);
      return getEfficiencyError(stripId);
    }

    /**
     * Returns error on efficiency of a given EKLM strip using the geometrical infos
     * @param section 2 for EF sectors, 1 for EB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     */
    float getEndcapEfficiencyError(int section, int sector, int layer, int plane, int strip) const
    {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      uint16_t stripId = elementNumbers->channelNumberEKLM(section, sector, layer, plane, strip);
      return getEfficiencyError(stripId);
    }

  private:

    /** KLM strip efficiency */
    std::map<uint16_t, float> m_efficiency;

    /** KLM strip efficiency error */
    std::map<uint16_t, float> m_efficiencyError;

    /** Class version. */
    ClassDef(KLMStripEfficiency, 1);

  };

}
