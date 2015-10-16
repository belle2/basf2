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
#include <vector>
#include <string>
#include <TObject.h>
#include <TTimeStamp.h>
#include <TH3F.h>
#include <TH2F.h>

namespace Belle2 {
  /**
  *   Tested ASIC chips
  */

  class ARICHAsicInfo: public TObject {
  public:
    /**
     * Default constructor
     */
    ARICHAsicInfo(): m_id(""), m_timeFinishGain(0, 0, 0, kTRUE, 0), m_timeFinishOffset(0, 0, 0, kTRUE, 0),
      m_deadChannel(), m_cutChannel(), m_gain(), m_offset(), m_comment("") {};

    /**
     * Constructor
     */
    ARICHAsicInfo(std::string id, TTimeStamp timeFinishGain, TTimeStamp timeFinishOffset, std::vector<std::string> deadChannel,
                  std::vector<std::string> cutChannel, std::string comment): m_id(id), m_timeFinishGain(timeFinishGain),
      m_timeFinishOffset(timeFinishOffset),
      m_deadChannel(deadChannel), m_cutChannel(cutChannel), m_gain(), m_offset(), m_comment(comment) {};

    /**
     * Constructor
     */
    ARICHAsicInfo(std::string id, TTimeStamp timeFinishGain, TTimeStamp timeFinishOffset, std::vector<TH3F*> gain,
                  std::vector<TH3F*> offset): m_id(id), m_timeFinishGain(timeFinishGain), m_timeFinishOffset(timeFinishOffset),
      m_deadChannel(), m_cutChannel(), m_gain(gain), m_offset(offset), m_comment("") {};

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

    /** Return Test date gain - finish
     * @return Test date gain - finish
     */
    TTimeStamp getTimeFinishGain() const {return m_timeFinishGain; }

    /** Set Test date gain - finish
     * @param Test date gain - finish
     */
    void setTimeFinishGain(TTimeStamp timeFinishGain) {m_timeFinishGain = timeFinishGain; }

    /** Return Test date offset - finish
     * @return Test date offset - finish
     */
    TTimeStamp getTimeFinishOffset() const {return m_timeFinishOffset; }

    /** Set Test date offset - finish
     * @param Test date offset - finish
     */
    void setTimeFinishOffset(TTimeStamp timeFinishOffset) {m_timeFinishOffset = timeFinishOffset; }

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

    /**
     * Return Measurements with different gain settings
     * @return Measurements with different gain settings
     */
    TH3F* getGainMeasurement(unsigned int i) const { if (i < m_gain.size()) return m_gain[i]; else return NULL;}

    /**
     * set Measurements with different gain settings
     * @param Measurements with different gain settings
     */
    void setGainMeasurement(std::vector<TH3F*> gain)
    {
      for (unsigned int i = 0; i < m_gain.size(); i++) m_gain.push_back(gain[i]);
    }

    /**
     * Return Measurements with different offset settings
     * @return Measurements with different offset settings
     */
    TH3F* getOffsetMeasurement(unsigned int i) const { if (i < m_offset.size()) return m_offset[i]; else return NULL;}

    /**
     * set Measurements with different offset settings
     * @param Measurements with different offset settings
     */
    void setOffsetMeasurement(std::vector<TH3F*> offset)
    {
      for (unsigned int i = 0; i < m_offset.size(); i++) m_offset.push_back(offset[i]);
    }

    /** Return Commment
     * @return Commment
     */
    std::string getComment() const {return m_comment; }

    /** Set comment
     * @param comment
     */
    void setComment(const std::string& comment) {m_comment = comment; }


  private:
    std::string m_id;                 /**< Asic Identifier */
    TTimeStamp m_timeFinishGain;      /**< Test Date of gain measurements - finish */
    TTimeStamp m_timeFinishOffset;    /**< Test Date of offset measurements - finish */
    std::vector<std::string> m_deadChannel;   /**< List of dead channels on the ASIC chip */
    std::vector<std::string> m_cutChannel;    /**< List of cut channels on the ASIC chip */
    std::vector<TH3F*> m_gain;      /**< Threshold scans with different gain settings */
    std::vector<TH3F*> m_offset;                /**< Threshold scans with different offset settings (course & fine offset)*/
    std::string m_comment;          /**< Comment */

    ClassDef(ARICHAsicInfo, 2);  /**< ClassDef */
  };
} // end namespace Belle2

