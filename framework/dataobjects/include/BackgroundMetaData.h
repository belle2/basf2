/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/pcore/Mergeable.h>
#include <framework/core/FrameworkExceptions.h>
#include <string>

namespace Belle2 {

  /**
   * Metadata information about the beam background file
   */
  class BackgroundMetaData: public Mergeable {

  public:

    /**
     * Enum for background tags.
     */
    enum BG_TAG { bg_none             = 0,  /**< No background */
                  bg_Coulomb_LER      = 1,  /**< Coulomb LER */
                  bg_Coulomb_HER      = 2,  /**< Coulomb HER */
                  bg_RBB_LER          = 3,  /**< Radiative Bhabha LER */
                  bg_RBB_HER          = 4,  /**< Radiative Bhabha HER */
                  bg_Touschek_LER     = 5,  /**< Touschek LER */
                  bg_Touschek_HER     = 6,  /**< Touschek HER */
                  bg_twoPhoton        = 7,  /**< 2-photon */
                  bg_RBB_gamma        = 8,  /**< Gammas from radiative Bhabha */
                  bg_RBB_LER_far      = 9,  /**< Radiative Bhabha far LER */
                  bg_RBB_HER_far      = 10, /**< Radiative Bhabha far HER */
                  bg_Touschek_LER_far = 11, /**< Touschek far LER */
                  bg_Touschek_HER_far = 12, /**< Touschek far HER */
                  bg_SynchRad_LER     = 13, /**< Synchrotron radiation LER */
                  bg_SynchRad_HER     = 14, /**< Synchrotron radiation HER */
                  bg_BHWide_LER       = 15, /**< Wide angle radiative Bhabha LER */
                  bg_BHWide_HER       = 16, /**< Wide angle radiative Bhabha HER */
                  bg_RBB              = 17, /**< Radiative Bhabha */
                  bg_BHWide           = 18, /**< Wide angle radiative Bhabha */
                  bg_BHWideLargeAngle = 19, /**< Large angle radiative Bhabha */
                  bg_InjectionLER     = 21, /**< injection background LER */
                  bg_InjectionHER     = 22, /**< injection background HER */
                  bg_Brems_LER        = 23, /**< bremsstrahlung LER */
                  bg_Brems_HER        = 24, /**< bremsstrahlung HER */
                  bg_other            = 99  /**< Other type of background */
                };

    /**
     * Exception definition
     */
    BELLE2_DEFINE_EXCEPTION(BackgroundMetaDataNotMergeable,
                            "BackgroundMetaData: objects cannot be merged");

    /**
     * Enum for BG file types
     */
    enum EFileType { c_Usual = 0,   /**< usual BG file */
                     c_ECL   = 1,   /**< additional for ECL */
                     c_PXD   = 2    /**< additional for PXD */
                   };

    /**
     * Constructor
     */
    BackgroundMetaData()
    {}

    /**
     * Sets background type
     * @param type background type
     */
    void setBackgroundType(const std::string& type) {m_backgroundType = type;}

    /**
     * Sets background tag value that corresponds to background type
     * @param tag a tag value
     */
    void setBackgroundTag(BG_TAG tag) {m_backgroundTag = tag;}

    /**
     * Sets real time that corresponds to this background sample
     * @param time real time
     */
    void setRealTime(float time) {m_realTime = time;}

    /**
     * Sets file type
     * @param type file type
     */
    void setFileType(EFileType type) {m_fileType = type;}

    /**
     * Returns the type of background
     * @return background type
     */
    const std::string& getBackgroundType() const {return m_backgroundType;}

    /**
     * Returns background tag value
     * @return tag value
     */
    BG_TAG getBackgroundTag() const {return m_backgroundTag;}

    /**
     * Returns real time that corresponds to this background sample
     * @return real time
     */
    float getRealTime() const {return m_realTime;}

    /**
     * Returns file type
     * @return file type
     */
    EFileType getFileType() const {return m_fileType;}

    /**
     * Implementation of abstract class function
     */
    virtual void merge(const Mergeable* other) override;

    /**
     * Implementation of abstract class function
     */
    virtual void clear() override;


  private:

    /**
     * Checks if other object can be merged with this object
     * @param other object to be merged with this object
     * @return true, if can be merged
     */
    bool canBeMerged(const BackgroundMetaData* other);

    std::string m_backgroundType; /**< beam background type */
    BG_TAG m_backgroundTag = bg_other; /**< background tag */
    float m_realTime = 0; /**< real time that corresponds to beam background sample */
    EFileType m_fileType = c_Usual; /**< file type */

    /**
     * Class definition required for creation of ROOT dictionary.
     */
    ClassDefOverride(BackgroundMetaData, 4); /** 4: moved over enum BG_TAG from SimHitBase */
  };
}

