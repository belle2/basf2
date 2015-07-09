/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <TList.h>
#include <string>
#include <TTimeStamp.h>

namespace Belle2 {
  /**
  *   Tested ASIC chips
  */

  class ARICHAsicInfo: public TObject {
  public:
    /**
     * Default constructor
     */
    ARICHAsicInfo(): m_timeStamp(0, 0, 0, kTRUE, 0), m_quality(0) {};

    /**
     * Constructor
     */
    ARICHAsicInfo(TTimeStamp timeStamp, int quality)
    {
      m_timeStamp = timeStamp;
      m_quality = quality;
    }

    /**
     * Destructor
     */
    ~ARICHAsicInfo() {};



    /** Return Asic Identifier
     * @return Asic Identifier
     */
    std::string getAsicID();

    /** Set Asic Identifier
     * @param Asic Identifier
     */
    void setAsicID();

    /** Return Test date
     * @return Test date
     */
    TTimeStamp getTimeStamp() const {return m_timeStamp; }

    /** Set Test date
     * @param Test date
     */
    void setTimeStamp(TTimeStamp timeStamp) {m_timeStamp = timeStamp; }

    /** Return Quality of the chip
     * @return Quality of the chip
     */
    int getChipQuality() const {return m_quality; }

    /** Set Quality of the chip
     * @param Quality of the chip
     */
    void setChipQuality(int quality) {m_quality = quality; }

    /** Return Commment
     * @return Commment
     */
    std::string getCommment();

    /** Set Commment
     * @param Commment
     */
    void setCommment();

    /** Return List of dead channels
     * @return List of dead channels
     */
    TList* getDeadChannels() const {return m_deadChannels; }

    /** Set List of dead channels
     * @param List of dead channels
     */
    void setDeadChannels(TList* deadChannels) {m_deadChannels = deadChannels; }

    /** Return List of cut channels
     * @return List of cut channels
     */
    TList* getCutChannels() const {return m_cutChannels; }

    /** Set List of cut channels
     * @param List of cut channels
     */
    void setCutChannels(TList* cutChannels) {m_cutChannels = cutChannels; }

  private:
    std::string m_id;        /**< Asic Identifier */
    TTimeStamp m_timeStamp;  /**< Test Date of the measurement */
    int m_quality;           /**< Quality class of the chip */
    std::string m_comment;   /**< Comment */
    TList* m_deadChannels;   /**< List of dead channels on the ASIC chip */
    TList* m_cutChannels;    /**< List of cut channels on the ASIC chip */

    ClassDef(ARICHAsicInfo, 1);  /**< ClassDef */
  };
} // end namespace Belle2

