/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMElementNumbers.h>

/* ROOT headers. */
#include <TObject.h>

/* C++ headers. */
#include <map>
#include <limits>

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
    {
    }

    /**
     * Default destructor
     */
    ~KLMStripEfficiency()
    {
    }

    /**
     * Set efficiency and relative error for a single KLM strip using directly the stripId
     * @param stripId KLM strip identifier
     * @param efficiency efficiency of the strip
     * @param efficiencyError error on the efficiency of the strip
     */
    void setEfficiency(KLMChannelNumber stripId, float efficiency, float efficiencyError = 0.)
    {
      m_efficiency.insert(std::pair<KLMChannelNumber, float>(stripId, efficiency));
      m_efficiencyError.insert(std::pair<KLMChannelNumber, float>(stripId, efficiencyError));
    }

    /**
     * Set efficiency and relative error for a single BKLM strip using the geometrical infos
     * @param section 1 for BF sectors, 0 for BB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     * @param efficiency efficiency of the strip
     * @param efficiencyError error on the efficiency of the strip
     */
    void setBarrelEfficiency(int section, int sector, int layer, int plane, int strip, float efficiency, float efficiencyError = 0.)
    {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      KLMChannelNumber stripId = elementNumbers->channelNumberBKLM(section, sector, layer, plane, strip);
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
      KLMChannelNumber stripId = elementNumbers->channelNumberEKLM(section, sector, layer, plane, strip);
      setEfficiency(stripId, efficiency, efficiencyError);
    }

    /**
     * Returns efficiency of a given KLM strip using directly the stripId
     * @param stripId KLM strip identifier
     */
    float getEfficiency(KLMChannelNumber stripId) const
    {
      auto search = m_efficiency.find(stripId);
      if (search == m_efficiency.end())
        return std::numeric_limits<float>::quiet_NaN();
      return search->second;
    }

    /**
     * Returns efficiency of a given BKLM strip using the geometrical infos
     * @param section 1 for BF sectors, 0 for BB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     */
    float getBarrelEfficiency(int section, int sector, int layer, int plane, int strip) const
    {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      KLMChannelNumber stripId = elementNumbers->channelNumberBKLM(section, sector, layer, plane, strip);
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
      KLMChannelNumber stripId = elementNumbers->channelNumberEKLM(section, sector, layer, plane, strip);
      return getEfficiency(stripId);
    }

    /**
     * Returns error on efficiency of a given KLM strip using directly the stripId
     * @param stripId KLM strip identifier
     */
    float getEfficiencyError(KLMChannelNumber stripId) const
    {
      auto search = m_efficiencyError.find(stripId);
      if (search == m_efficiencyError.end())
        return std::numeric_limits<float>::quiet_NaN();
      return search->second;
    }

    /**
     * Returns error on efficiency of a given BKLM strip using the geometrical infos
     * @param section 1 for BF sectors, 0 for BB sectors
     * @param sector sector number
     * @param layer layer number
     * @param plane plane number
     * @param strip strip number
     */
    float getBarrelEfficiencyError(int section, int sector, int layer, int plane, int strip) const
    {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      KLMChannelNumber stripId = elementNumbers->channelNumberBKLM(section, sector, layer, plane, strip);
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
      KLMChannelNumber stripId = elementNumbers->channelNumberEKLM(section, sector, layer, plane, strip);
      return getEfficiencyError(stripId);
    }

  private:

    /** KLM strip efficiency */
    std::map<KLMChannelNumber, float> m_efficiency;

    /** KLM strip efficiency error */
    std::map<KLMChannelNumber, float> m_efficiencyError;

    /** Class version. */
    ClassDef(KLMStripEfficiency, 1);

  };

}
