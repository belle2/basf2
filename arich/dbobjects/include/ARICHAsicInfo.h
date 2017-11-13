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
    ARICHAsicInfo(): m_id(""), m_timeFinishGain(), m_timeFinishOffset(), m_deadChannel(),
      m_badConnChannel(), m_badOffsetChannel(), m_badLinChannel(), m_numProblematicChannels(0), m_gain(), m_offset(), m_comment("") {};

    /**
     * Constructor
     */
    ARICHAsicInfo(const std::string& id, TTimeStamp timeFinishGain, TTimeStamp timeFinishOffset, std::vector<int>& deadChannel,
                  std::vector<int>& badConnChannel, std::vector<int>& badOffsetChannel, std::vector<int>& badLinChannel, int numProbCh,
                  std::vector<TH3F*>& gain, std::vector<TH3F*>& offset, const std::string& comment): m_id(id),  m_timeFinishGain(timeFinishGain),
      m_timeFinishOffset(timeFinishOffset), m_deadChannel(deadChannel), m_badConnChannel(badConnChannel),
      m_badOffsetChannel(badOffsetChannel), m_badLinChannel(badLinChannel), m_numProblematicChannels(numProbCh), m_gain(gain),
      m_offset(offset), m_comment(comment) {};

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
    int getDeadChannel(unsigned int i) const;

    /**
     * Add a channel number to the list of dead channels
     * @param channel ASIC channel id
     */
    void appendDeadChannel(int channel) {m_deadChannel.push_back(channel); }

    /**
     * Set vector of dead channel numbers
     * @param channel ASIC channel id
     */
    void setDeadChannels(const std::vector<int>& deadChannels) {m_deadChannel = deadChannels; }

    /**
     * Return size of the list of dead channels
     * @return size
     */
    int getDeadChannelsSize() const {return m_deadChannel.size();}

    /**
     * Return a channel number from the list of channels with bad connections
     * @param i index of the element in the list
     * @return channel id
     */
    int getBadConnChannel(unsigned int i) const;

    /**
     * Add a channel number to the list of channels with bad connections
     * @param channel ASIC channel id
     */
    void appendBadConnChannel(int ichannel) { m_badConnChannel.push_back(ichannel); }

    /**
     * Return size of the list of channels with bad connections
     * @return size
     */
    int getBadConnChannelsSize() const {return m_badConnChannel.size();}

    /**
     * Set vector of bad connection channel numbers
     * @param channel ASIC channel id
     */
    void setBadConnChannels(const std::vector<int>& badConnChannels) {m_badConnChannel = badConnChannels; }

    /**
     * Return a channel number from the list of channels with bad offset adjustment
     * @param i index of the element in the list
     * @return channel id
     */
    int getBadOffsetChannel(unsigned int i) const;

    /**
     * Add a channel number to the list of channels with bad offset adjustment
     * @param channel ASIC channel id
     */
    void appendBadOffsetChannel(int ichannel) { m_badOffsetChannel.push_back(ichannel); }

    /**
     * Return size of the list of channels with bad offset adjustment
     * @return size
     */
    int getBadOffsetChannelsSize() const {return m_badOffsetChannel.size();}

    /**
     * Set vector of bad offset channel numbers
     * @param channel ASIC channel id
     */
    void setBadOffsetChannels(const std::vector<int>&  badOffsetChannels) {m_badOffsetChannel = badOffsetChannels; }

    /**
     * Return a channel number from the list of channels with bad linearity
     * @param i index of the element in the list
     * @return channel id
     */
    int getBadLinChannel(unsigned int i) const;

    /**
     * Add a channel number to the list of channels with bad linearity
     * @param channel ASIC channel id
     */
    void appendBadLinChannel(int ichannel) { m_badLinChannel.push_back(ichannel); }

    /**
     * Return size of the list of channels with bad linearity
     * @return size
     */
    int getBadLinChannelsSize() const {return m_badLinChannel.size();}

    /**
     * Set vector of bad linearity channel numbers
     * @param channel ASIC channel id
     */
    void setBadLinChannels(const std::vector<int>& badLinChannels) {m_badLinChannel = badLinChannels; }

    /** Return number of problematic channels
     * @return number of problematic channels
     */
    float getNumOfProblematicChannels() const {return m_numProblematicChannels; }

    /** Set number of problematic channels
     * @param number of problematic channels
     */
    void setNumOfProblematicChannels(int numProbCh) {m_numProblematicChannels = numProbCh; }

    /**
     * Return Measurements with different gain settings
     * @return Measurements with different gain settings
     */
    TH3F* getGainMeasurement(unsigned int i) const;

    /**
     * set Measurements with different gain settings
     * @param Measurements with different gain settings
     */
    void setGainMeasurement(std::vector<TH3F*> gain);
    /**
     * Return Measurements with different offset settings
     * @return Measurements with different offset settings
     */
    TH3F* getOffsetMeasurement(unsigned int i) const;

    /**
     * set Measurements with different offset settings
     * @param Measurements with different offset settings
     */
    void setOffsetMeasurement(std::vector<TH3F*> offset);

    /** Return Commment
     * @return Commment
     */
    std::string getComment() const {return m_comment; }

    /** Set comment
     * @param comment
     */
    void setComment(const std::string& comment) {m_comment = comment; }


  private:
    std::string m_id;                     /**< Asic Identifier */
    TTimeStamp m_timeFinishGain;          /**< Test Date of gain measurements - finish */
    TTimeStamp m_timeFinishOffset;        /**< Test Date of offset measurements - finish */
    std::vector<int> m_deadChannel;       /**< List of dead channels on the ASIC chip */
    std::vector<int> m_badConnChannel;    /**< List of channels with bad connections during the measurement */
    std::vector<int> m_badOffsetChannel;  /**< List of channels with bad offset adjustment */
    std::vector<int> m_badLinChannel;     /**< List of channels with bad linearity */
    int m_numProblematicChannels;         /**< Number of channels with problems */
    std::vector<TH3F*> m_gain;            /**< Threshold scans with different gain settings */
    std::vector<TH3F*> m_offset;          /**< Threshold scans with different offset settings (course & fine offset)*/
    std::string m_comment;                /**< Comment */

    ClassDef(ARICHAsicInfo, 3);  /**< ClassDef */
  };
} // end namespace Belle2

