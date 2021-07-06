/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/dataobjects/BackgroundMetaData.h>
#include <string>
#include <map>

namespace Belle2 {
  namespace background {

    /**
     * Class to define BG types and to convert between BG types and tags or v.v.
     * Designed primarely for BeamBkgTagSetter and BeamBkgMixer modules
     * Suitable also to convert backgroundTag in SimHit classes to name (BG type)
     */
    class BeamBGTypes {

    public:
      /**
       * Constructor
       */
      BeamBGTypes()
      {
        m_tags["Coulomb_LER"] = BackgroundMetaData::bg_Coulomb_LER;
        m_tags["Coulomb_HER"] = BackgroundMetaData::bg_Coulomb_HER;
        m_tags["RBB_LER"] = BackgroundMetaData::bg_RBB_LER;
        m_tags["RBB_HER"] = BackgroundMetaData::bg_RBB_HER;
        m_tags["Touschek_LER"] = BackgroundMetaData::bg_Touschek_LER;
        m_tags["Touschek_HER"] = BackgroundMetaData::bg_Touschek_HER;
        m_tags["twoPhoton"] = BackgroundMetaData::bg_twoPhoton;
        m_tags["RBB_gamma"] = BackgroundMetaData::bg_RBB_gamma;
        m_tags["RBB_LER_far"] = BackgroundMetaData::bg_RBB_LER_far;
        m_tags["RBB_HER_far"] = BackgroundMetaData::bg_RBB_HER_far;
        m_tags["Touschek_LER_far"] = BackgroundMetaData::bg_Touschek_LER_far;
        m_tags["Touschek_HER_far"] = BackgroundMetaData::bg_Touschek_HER_far;
        m_tags["SynchRad_LER"] = BackgroundMetaData::bg_SynchRad_LER;
        m_tags["SynchRad_HER"] = BackgroundMetaData::bg_SynchRad_HER;
        m_tags["BHWide_LER"] = BackgroundMetaData::bg_BHWide_LER;
        m_tags["BHWide_HER"] = BackgroundMetaData::bg_BHWide_HER;
        m_tags["RBB"] = BackgroundMetaData::bg_RBB;
        m_tags["BHWide"] = BackgroundMetaData::bg_BHWide;
        m_tags["BHWideLargeAngle"] = BackgroundMetaData::bg_BHWideLargeAngle;
        m_tags["InjectionLER"] = BackgroundMetaData::bg_InjectionLER;
        m_tags["InjectionHER"] = BackgroundMetaData::bg_InjectionHER;
        m_tags["Brems_LER"] = BackgroundMetaData::bg_Brems_LER;
        m_tags["Brems_HER"] = BackgroundMetaData::bg_Brems_HER;
        m_tags["other"] = BackgroundMetaData::bg_other;
      }

      /**
       * Return BG tag for a given BG type
       * @param bgType BG type name
       * @return BG tag value
       */
      BackgroundMetaData::BG_TAG getTag(const std::string& bgType)
      {
        return m_tags[bgType];
      }

      /**
       * Return BG type for a given BG tag
       * @param bgTag BG tag value
       * @return BG type name or empty string
       */
      std::string getType(BackgroundMetaData::BG_TAG bgTag) const
      {
        if (bgTag == 0) return "";
        for (auto tag : m_tags) {
          if (bgTag == tag.second) return tag.first;
        }
        return "";
      }

      /**
       * Return BG type for a given number
       * @param bgTag BG tag value
       * @return BG type name or empty string
       */
      std::string getType(unsigned int bgTag) const
      {
        if (bgTag == 0) return "";
        for (auto tag : m_tags) {
          if (bgTag == tag.second) return tag.first;
        }
        return "";
      }

      /**
       * Return all defined BG types as a string
       * @return BG type names separated by comma
       */
      std::string getBGTypes() const
      {
        std::string types;
        for (auto tag : m_tags) {
          if (tag.first != "other" && tag.second != 0) types += tag.first + ", ";
        }
        types += "other";
        return types;
      }

    private:

      std::map<std::string, BackgroundMetaData::BG_TAG> m_tags; /**< a map of BG tags */

    };

  } // background
} // Belle2

