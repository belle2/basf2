/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BACKGROUNDMETADATA_H
#define BACKGROUNDMETADATA_H

#include <TObject.h>
#include <simulation/dataobjects/SimHitBase.h>
#include <string>

namespace Belle2 {

  /**
   * Metadata information about the beam background file
   */
  class BackgroundMetaData: public TObject {

  public:
    /**
     * Enum for BG file types
     */
    enum EFileType { c_Usual = 0,  /**< usual BG file */
                     c_ECL   = 1,  /**< additional for ECL */
                     c_PXD   = 2   /**< additional for PXD */
                   };

    /**
     * Constructor
     */
    BackgroundMetaData() : m_backgroundTag(SimHitBase::bg_other),
      m_realTime(0.0), m_fileType(c_Usual)
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

  private:

    std::string m_backgroundType; /**< beam background type */
    SimHitBase::BG_TAG m_backgroundTag; /**< background tag value */
    float m_realTime; /**< real time that corresponds to beam background sample */
    EFileType m_fileType; /**< file type */

    /**
     * Class definition required for creation of ROOT dictionary.
     */
    ClassDef(BackgroundMetaData, 2);
  };
}


#endif /* BACKGROUNDMETADATA_H */
