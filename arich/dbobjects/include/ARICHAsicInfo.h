/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <vector>
#include <string>
#include <TObject.h>
#include <TTimeStamp.h>
#include <TH3F.h>

namespace Belle2 {

  /**
   * Tested ASIC chips.
   */
  class ARICHAsicInfo: public TObject {

  public:

    /**
     * Default constructor.
     */
    ARICHAsicInfo(): m_id(""), m_timeFinishGain(), m_timeFinishOffset(), m_deadChannel(),
      m_badConnChannel(), m_badOffsetChannel(), m_badLinChannel(), m_numProblematicChannels(0), m_gain(), m_offset(), m_comment("") {};

    /**
     * Constructor.
     */
    ARICHAsicInfo(const std::string& id, TTimeStamp timeFinishGain, TTimeStamp timeFinishOffset, std::vector<int>& deadChannel,
                  std::vector<int>& badConnChannel, std::vector<int>& badOffsetChannel, std::vector<int>& badLinChannel, int numProbCh,
                  std::vector<TH3F*>& gain, std::vector<TH3F*>& offset, const std::string& comment): m_id(id),  m_timeFinishGain(timeFinishGain),
      m_timeFinishOffset(timeFinishOffset), m_deadChannel(deadChannel), m_badConnChannel(badConnChannel),
      m_badOffsetChannel(badOffsetChannel), m_badLinChannel(badLinChannel), m_numProblematicChannels(numProbCh), m_gain(gain),
      m_offset(offset), m_comment(comment) {};

    /**
     * Destructor.
     */
    ~ARICHAsicInfo() {};

    /**
     * Get ASIC identifier.
     * @return ASIC identifier.
     */
    std::string getAsicID() const {return m_id; }

    /**
     * Set ASIC identifier.
     * @param[in] id ASIC identifier.
     */
    void setAsicID(const std::string& id) {m_id = id; }

    /**
     * Get test date gain - finish.
     * @return Test date gain - finish.
     */
    TTimeStamp getTimeFinishGain() const {return m_timeFinishGain; }

    /**
     * Set test date gain - finish.
     * @param[in] timeFinishGain Test date gain - finish.
     */
    void setTimeFinishGain(TTimeStamp timeFinishGain) {m_timeFinishGain = timeFinishGain; }

    /**
     * Get test date offset - finish.
     * @return Test date offset - finish.
     */
    TTimeStamp getTimeFinishOffset() const {return m_timeFinishOffset; }

    /**
     * Set Test date offset - finish.
     * @param[in] timeFinishOffset Test date offset - finish.
     */
    void setTimeFinishOffset(TTimeStamp timeFinishOffset) {m_timeFinishOffset = timeFinishOffset; }

    /**
     * Get a channel number from the list of dead channels.
     * @param[in] i Index of the element in the list.
     * @return Channel id.
     */
    int getDeadChannel(unsigned int i) const;

    /**
     * Add a channel number to the list of dead channels.
     * @param[in] channel ASIC channel id
     */
    void appendDeadChannel(int channel) {m_deadChannel.push_back(channel); }

    /**
     * Set vector of dead channel numbers.
     * @param[in] deadChannels ASIC channel id.
     */
    void setDeadChannels(const std::vector<int>& deadChannels) {m_deadChannel = deadChannels; }

    /**
     * Get size of the list of dead channels.
     * @return Size.
     */
    int getDeadChannelsSize() const {return m_deadChannel.size();}

    /**
     * Get a channel number from the list of channels with bad connections.
     * @param[in] i Index of the element in the list.
     * @return Channel id.
     */
    int getBadConnChannel(unsigned int i) const;

    /**
     * Add a channel number to the list of channels with bad connections.
     * @param[in] channel ASIC channel id.
     */
    void appendBadConnChannel(int channel) { m_badConnChannel.push_back(channel); }

    /**
     * Set vector of bad connection channel numbers.
     * @param[in] badConnChannels ASIC channel id.
     */
    void setBadConnChannels(const std::vector<int>& badConnChannels) {m_badConnChannel = badConnChannels; }

    /**
     * Get size of the list of channels with bad connections.
     * @return Size.
     */
    int getBadConnChannelsSize() const {return m_badConnChannel.size();}

    /**
     * Get a channel number from the list of channels with bad
     * offset adjustment.
     * @param[in] i Index of the element in the list.
     * @return Channel id.
     */
    int getBadOffsetChannel(unsigned int i) const;

    /**
     * Add a channel number to the list of channels with bad offset adjustment.
     * @param[in] channel ASIC channel id.
     */
    void appendBadOffsetChannel(int channel) { m_badOffsetChannel.push_back(channel); }

    /**
     * Set vector of bad offset channel numbers.
     * @param[in] badOffsetChannels ASIC channel id.
     */
    void setBadOffsetChannels(const std::vector<int>& badOffsetChannels) {m_badOffsetChannel = badOffsetChannels; }

    /**
     * Get size of the list of channels with bad offset adjustment.
     * @return Size.
     */
    int getBadOffsetChannelsSize() const {return m_badOffsetChannel.size();}

    /**
     * Get a channel number from the list of channels with bad linearity.
     * @param[in] i Index of the element in the list.
     * @return channel id
     */
    int getBadLinChannel(unsigned int i) const;

    /**
     * Add a channel number to the list of channels with bad linearity.
     * @param[in] channel ASIC channel id.
     */
    void appendBadLinChannel(int channel) { m_badLinChannel.push_back(channel); }

    /**
     * Set vector of bad linearity channel numbers.
     * @param[in] badLinChannels ASIC channel id.
     */
    void setBadLinChannels(const std::vector<int>& badLinChannels) {m_badLinChannel = badLinChannels; }

    /**
     * Get size of the list of channels with bad linearity.
     * @return Size.
     */
    int getBadLinChannelsSize() const {return m_badLinChannel.size();}

    /**
     * Get number of problematic channels.
     * @return Number of problematic channels.
     */
    float getNumOfProblematicChannels() const {return m_numProblematicChannels; }

    /**
     * Set number of problematic channels.
     * @param[in] numProbCh Number of problematic channels.
     */
    void setNumOfProblematicChannels(int numProbCh) {m_numProblematicChannels = numProbCh; }

    /**
     * Get measurements with different gain settings.
     * @return Measurements with different gain settings.
     */
    TH3F* getGainMeasurement(unsigned int i) const;

    /**
     * Set measurements with different gain settings.
     * @param[in] gain Measurements with different gain settings.
     */
    void setGainMeasurement(std::vector<TH3F*> gain);

    /**
     * Get measurements with different offset settings.
     * @return Measurements with different offset settings.
     */
    TH3F* getOffsetMeasurement(unsigned int i) const;

    /**
     * Set measurements with different offset settings.
     * @param[in] offset Measurements with different offset settings.
     */
    void setOffsetMeasurement(std::vector<TH3F*> offset);

    /**
     * Get comment.
     * @return Comment.
     */
    std::string getComment() const {return m_comment; }

    /**
     * Set comment.
     * @param[in] comment Comment.
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

