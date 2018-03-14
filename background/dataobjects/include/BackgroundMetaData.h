/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/pcore/Mergeable.h>
#include <simulation/dataobjects/SimHitBase.h>
#include <framework/core/FrameworkExceptions.h>
#include <string>

namespace Belle2 {

  /**
   * Metadata information about the beam background file
   */
  class BackgroundMetaData: public Mergeable {

  public:

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
     * Sets background tag value that corresponds to background type (see SimHitBase.h)
     * @param tag a tag value
     */
    void setBackgroundTag(SimHitBase::BG_TAG tag) {m_backgroundTag = tag;}

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
    SimHitBase::BG_TAG getBackgroundTag() const {return m_backgroundTag;}

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
    virtual void merge(const Mergeable* other);

    /**
     * Implementation of abstract class function
     */
    virtual void clear();


  private:

    /**
     * Checks if other object can be merged with this object
     * @param other object to be merged with this object
     * @return true, if can be merged
     */
    bool canBeMerged(const BackgroundMetaData* other);

    std::string m_backgroundType; /**< beam background type */
    SimHitBase::BG_TAG m_backgroundTag = SimHitBase::bg_other; /**< background tag */
    float m_realTime = 0; /**< real time that corresponds to beam background sample */
    EFileType m_fileType = c_Usual; /**< file type */

    /**
     * Class definition required for creation of ROOT dictionary.
     */
    ClassDef(BackgroundMetaData, 3);
  };
}

