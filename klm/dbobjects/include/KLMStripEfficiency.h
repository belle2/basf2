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
#include <klm/dataobjects/KLMElementNumbers.h>
#include <framework/logging/Logger.h>

// ROOT includes
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
     * Set efficiency and relative error for a single BKLM strip using directly the stripId
     * @param stripId BKLM strip identifier
     * @param efficiency efficiency of the strip
     * @param efficiencyError error on the efficiency of the strip
     */
    void setBarrelEfficiency(int stripId, float efficiency, float efficiencyError = 0.)
    {
      m_barrelEfficiency.insert(std::pair<int, float>(stripId, efficiency));
      m_barrelEfficiencyError.insert(std::pair<int, float>(stripId, efficiencyError));
    }

    /**
     * Set efficiency and relative error for a single BKLM strip using the geometrical infos
     * @param isForward 1 for BF sectors, 0 for BB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     * @param efficiency efficiency of the strip
     * @param efficiencyError error on the efficiency of the strip
     */
    void setBarrelEfficiency(int isForward, int sector, int layer, int plane, int strip, float efficiency, float efficiencyError = 0.)
    {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      int stripId = elementNumbers->channelNumberBKLM(isForward, sector, layer, plane, strip);
      setBarrelEfficiency(stripId, efficiency, efficiencyError);
    }

    /**
     * Returns efficiency of a given BKLM strip using directly the stripId
     * @param stripId BKLM strip identifier
     */
    float getBarrelEfficiency(int stripId) const
    {
      auto search = m_barrelEfficiency.find(stripId);
      if (search == m_barrelEfficiency.end())
        return std::numeric_limits<float>::quiet_NaN();
      return search->second;
    }

    /**
     * Returns efficiency of a given BKLM strip using the geometrical infos
     * @param isForward 1 for BF sectors, 0 for BB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     */
    float getBarrelEfficiency(int isForward, int sector, int layer, int plane, int strip) const
    {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      int stripId = elementNumbers->channelNumberBKLM(isForward, sector, layer, plane, strip);
      return getBarrelEfficiency(stripId);
    }

    /**
     * Returns error on efficiency of a given BKLM strip using directly the stripId
     * @param stripId BKLM strip identifier
     */
    float getBarrelEfficiencyError(int stripId) const
    {
      auto search = m_barrelEfficiencyError.find(stripId);
      if (search == m_barrelEfficiencyError.end())
        return std::numeric_limits<float>::quiet_NaN();
      return search->second;
    }

    /**
     * Returns error on efficiency of a given BKLM strip using the geometrical infos
     * @param isForward 1 for BF sectors, 0 for BB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     */
    float getBarrelEfficiencyError(int isForward, int sector, int layer, int plane, int strip) const
    {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      int stripId = elementNumbers->channelNumberBKLM(isForward, sector, layer, plane, strip);
      return getBarrelEfficiencyError(stripId);
    }

    /**
     * Set efficiency and relative error for a single EKLM strip using directly the stripId
     * @param stripId EKLM strip identifier
     * @param efficiency efficiency of the strip
     * @param efficiencyError error on the efficiency of the strip
     */
    void setEndcapEfficiency(int stripId, float efficiency, float efficiencyError = 0.)
    {
      m_endcapEfficiency.insert(std::pair<int, float>(stripId, efficiency));
      m_endcapEfficiencyError.insert(std::pair<int, float>(stripId, efficiencyError));
    }

    /**
     * Set efficiency and relative error for a single EKLM strip using the geometrical infos
     * @param isForward 1 for EF sectors, 0 for EB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     * @param efficiency efficiency of the strip
     * @param efficiencyError error on the efficiency of the strip
     */
    void setEndcapEfficiency(int isForward, int sector, int layer, int plane, int strip, float efficiency, float efficiencyError = 0.)
    {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      int stripId = elementNumbers->channelNumberEKLM(isForward, layer, sector, plane, strip);
      setEndcapEfficiency(stripId, efficiency, efficiencyError);
    }

    /**
     * Returns efficiency of a given EKLM strip using directly the stripId
     * @param stripId EKLM strip identifier
     */
    float getEndcapEfficiency(int stripId) const
    {
      auto search = m_endcapEfficiency.find(stripId);
      if (search == m_endcapEfficiency.end())
        return std::numeric_limits<float>::quiet_NaN();
      return search->second;
    }

    /**
     * Returns efficiency of a given EKLM strip using the geometrical infos
     * @param isForward 1 for EF sectors, 0 for EB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     */
    float getEndcapEfficiency(int isForward, int sector, int layer, int plane, int strip) const
    {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      int stripId = elementNumbers->channelNumberEKLM(isForward, layer, sector, plane, strip);
      return getEndcapEfficiency(stripId);
    }

    /**
     * Returns error on efficiency of a given EKLM strip using directly the stripId
     * @param stripId EKLM strip identifier
     */
    float getEndcapEfficiencyError(int stripId) const
    {
      auto search = m_endcapEfficiencyError.find(stripId);
      if (search == m_endcapEfficiencyError.end())
        return std::numeric_limits<float>::quiet_NaN();
      return search->second;
    }

    /**
     * Returns error on efficiency of a given EKLM strip using the geometrical infos
     * @param isForward 1 for EF sectors, 0 for EB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     */
    float getEndcapEfficiencyError(int isForward, int sector, int layer, int plane, int strip) const
    {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      int stripId = elementNumbers->channelNumberEKLM(isForward, layer, sector, plane, strip);
      return getEndcapEfficiencyError(stripId);
    }

  private:

    std::map<int, float> m_barrelEfficiency; /** BKLM strip efficiency */
    std::map<int, float> m_barrelEfficiencyError; /** BKLM strip efficiency error */
    std::map<int, float> m_endcapEfficiency; /** EKLM strip efficiency */
    std::map<int, float> m_endcapEfficiencyError; /** EKLM strip efficiency error */

    ClassDef(KLMStripEfficiency, 1); /**< ClassDef */
  };

} // end namespace Belle2
