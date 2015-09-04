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
#include <vector>
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
    ARICHAsicInfo(): m_id(""), m_timeStamp(0, 0, 0, kTRUE, 0), m_quality(0), m_deadChannel(), m_cutChannel(), m_comment("") {};

    /**
     * Constructor
     */
    ARICHAsicInfo(std::string id, TTimeStamp timeStamp, int quality, std::vector<std::string> deadChannel,
                  std::vector<std::string> cutChannel, std::string comment)
    {
      m_id = id;
      m_timeStamp = timeStamp;
      m_quality = quality;
      m_deadChannel = deadChannel;
      m_cutChannel = cutChannel;
      m_comment = comment;
    }

    /**
     * Destructor
     */
    ~ARICHAsicInfo() {};

    /** Return Asic Identifier
     * @return Asic Identifier
     */
    std::string getAsicID() const {return m_id; }

    /** Set Asic Identifier
     * @param Asic Identifier
     */
    void setAsicID(const std::string& id) {m_id = id; }

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

    /**
     * Return a channel number from the list of dead channels
     * @param i index of the element in the list
     * @return channel id
     */
    std::string getDeadChannel(unsigned int i) const { if (i < m_deadChannel.size()) return m_deadChannel[i]; else return NULL;}

    /**
     * Add a channel number to the list of dead channels
     * @param channel ASIC channel id
     */
    void appendDeadChannel(std::string channel) {m_deadChannel.push_back(channel); }

    /**
     * Return size of the list of dead channels
     * @return size
     */
    int getDeadChannelsSize() const {return m_deadChannel.size();}

    /**
     * Return a channel number from the list of cut channels
     * @param i index of the element in the list
     * @return channel id
     */
    std::string getCutChannel(unsigned int i) const { if (i < m_cutChannel.size()) return m_cutChannel[i]; else return NULL;}

    /**
     * Add a channel number to the list of cut channels
     * @param channel ASIC channel id
     */
    void appendCutChannel(std::string ichannel) { m_cutChannel.push_back(ichannel); }

    /**
     * Return size of the list of cut channels
     * @return size
     */
    int getCutChannelsSize() const {return m_cutChannel.size();}

    /** Return Commment
     * @return Commment
     */
    std::string getComment() const {return m_comment; }

    /** Set comment
     * @param comment
     */
    void setComment(const std::string& comment) {m_comment = comment; }


  private:
    std::string m_id;        /**< Asic Identifier */
    TTimeStamp m_timeStamp;  /**< Test Date of the measurement */
    int m_quality;           /**< Quality class of the chip */
    std::vector<std::string> m_deadChannel;   /**< List of dead channels on the ASIC chip */
    std::vector<std::string> m_cutChannel;    /**< List of cut channels on the ASIC chip */
    std::string m_comment;   /**< Comment */

    ClassDef(ARICHAsicInfo, 1);  /**< ClassDef */
  };
} // end namespace Belle2

