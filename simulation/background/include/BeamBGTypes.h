/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <simulation/dataobjects/SimHitBase.h>
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
        m_tags["Coulomb_LER"] = SimHitBase::bg_Coulomb_LER;
        m_tags["Coulomb_HER"] = SimHitBase::bg_Coulomb_HER;
        m_tags["RBB_LER"] = SimHitBase::bg_RBB_LER;
        m_tags["RBB_HER"] = SimHitBase::bg_RBB_HER;
        m_tags["Touschek_LER"] = SimHitBase::bg_Touschek_LER;
        m_tags["Touschek_HER"] = SimHitBase::bg_Touschek_HER;
        m_tags["twoPhoton"] = SimHitBase::bg_twoPhoton;
        m_tags["RBB_gamma"] = SimHitBase::bg_RBB_gamma;
        m_tags["RBB_LER_far"] = SimHitBase::bg_RBB_LER_far;
        m_tags["RBB_HER_far"] = SimHitBase::bg_RBB_HER_far;
        m_tags["Touschek_LER_far"] = SimHitBase::bg_Touschek_LER_far;
        m_tags["Touschek_HER_far"] = SimHitBase::bg_Touschek_HER_far;
        m_tags["SynchRad_LER"] = SimHitBase::bg_SynchRad_LER;
        m_tags["SynchRad_HER"] = SimHitBase::bg_SynchRad_HER;
        m_tags["BHWide_LER"] = SimHitBase::bg_BHWide_LER;
        m_tags["BHWide_HER"] = SimHitBase::bg_BHWide_HER;
        m_tags["RBB"] = SimHitBase::bg_RBB;
        m_tags["BHWide"] = SimHitBase::bg_BHWide;
        m_tags["BHWideLargeAngle"] = SimHitBase::bg_BHWideLargeAngle;
        m_tags["InjectionLER"] = SimHitBase::bg_InjectionLER;
        m_tags["InjectionHER"] = SimHitBase::bg_InjectionHER;
        m_tags["other"] = SimHitBase::bg_other;
      }

      /**
       * Return BG tag for a given BG type
       * @param bgType BG type name
       * @return BG tag value
       */
      SimHitBase::BG_TAG getTag(const std::string& bgType)
      {
        return m_tags[bgType];
      }

      /**
       * Return BG type for a given BG tag
       * @param bgTag BG tag value
       * @return BG type name or empty string
       */
      std::string getType(SimHitBase::BG_TAG bgTag) const
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

      std::map<std::string, SimHitBase::BG_TAG> m_tags; /**< a map of BG tags */

    };

  } // background
} // Belle2

