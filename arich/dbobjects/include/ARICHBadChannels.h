/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <vector>

namespace Belle2 {

  /**
   * ARICH bad channels.
   */
  class ARICHBadChannels: public TObject {

  public:

    /**
     * Default constructor.
     */
    ARICHBadChannels(): m_id(-1), m_hapdSerial(""), m_febSerial(0), m_hapdCutChannels(), m_hapdBadChannels(), m_febDeadChannels(),
      m_asicDeadChannels(), m_asicBadConnChannels(), m_asicBadOffsetChannels(), m_asicBadLinChannels() {};

    /**
     * Destructor.
     */
    ~ARICHBadChannels() {};

    /**
     * Get global ID.
     * @return Global ID.
     */
    int getID() const {return m_id; }

    /**
     * Set global ID.
     * @param[in] id Global ID.
     */
    void setID(const int id) {m_id = id; }

    /**
     * Get HAPD serial number.
     * @return HAPD serial number.
     */
    std::string getHapdSerial() const {return m_hapdSerial; }

    /**
     * Set HAPD serial number.
     * @param[in] serial HAPD serial number.
     */
    void setHapdSerial(const std::string& serial) {m_hapdSerial = serial; }

    /**
     * Get FEB serial number.
     * @return FEB serial number.
     */
    int getFebSerial() const {return m_febSerial; }

    /**
     * Set FEB serial number.
     * @param[in] serial FEB serial number.
     */
    void setFebSerial(const int serial) {m_febSerial = serial; }

    /**
     * Get a channel number from the list of cut channels.
     * @param[in] i Index of the element in the list.
     * @return Channel number.
     */
    int getHapdCutChannel(unsigned int i) const;

    /**
     * Get a list of cut channels.
     * @return Vector of channels.
     */
    std::vector<int> getHapdCutChannels() const {return m_hapdCutChannels; }

    /**
     * Add a channel number to the list of cut channels.
     * @param[in] channel HAPD channel number.
     */
    void appendHapdCutChannel(int channel) {m_hapdCutChannels.push_back(channel); }

    /**
     * Set the list of cut channels.
     * @param[in] channels HAPD channel numbers.
     */
    void setHapdCutChannel(std::vector<int> channels);

    /**
     * Get size of the list of cut channels.
     * @return Size.
     */
    int getHapdCutChannelsSize() const {return m_hapdCutChannels.size(); }

    /**
     * Get a channel number from the list of bad channels.
     * @param[in] i Index of the element in the list.
     * @return Channel number.
     */
    int getHapdBadChannel(unsigned int i) const;

    /**
     * Get a list of bad channels.
     * @return vector of channels.
     */
    std::vector<int> getHapdBadChannels() const {return m_hapdBadChannels; }

    /**
     * Add a channel number to the list of bad channels.
     * @param[in] channel HAPD channel number.
     */
    void appendHapdBadChannel(int channel) { m_hapdBadChannels.push_back(channel); }

    /**
     * Set the list of bad channels.
     * @param[in] channels HAPD channel numbers.
     */
    void setHapdBadChannel(std::vector<int> channels);

    /**
     * Get size of the list of cut channels.
     * @return Size.
     */
    int getHapdBadChannelsSize() const {return m_hapdBadChannels.size();}

    /**
     * Get a channel number from the list of bad channels.
     * @param[in] i Index of the element in the list.
     * @return Channel id.
     */
    int getHapdListOfBadChannel(unsigned int i) const;

    /**
     * Get a list of all bad channels.
     * @return Vector of channels.
     */
    std::vector<int> getHapdListOfBadChannels() const { return m_hapdListOfBadChannels; }

    /**
     * Get size of the list of all bad channels.
     * @return Size.
     */
    int getHapdListOfBadChannelsSize() const { return m_hapdListOfBadChannels.size(); }

    /**
     * Get a channel number from the list of dead channels.
     * @param[in] i Index of the element in the list.
     * @return Channel id.
     */
    int getFebDeadChannel(unsigned int i) const;

    /**
     * Get a list of dead channels.
     * @return Vector of channels.
     */
    std::vector<int> getFebDeadChannels() const {return m_febDeadChannels; }

    /**
     * Add a channel number to the list of dead channels.
     * @param[in] channel FEB channel id.
     */
    void appendFebDeadChannel(int channel) {m_febDeadChannels.push_back(channel); }

    /**
     * Set vector of dead channel numbers.
     * @param[in] deadChannels FEB channel id.
     */
    void setFebDeadChannels(std::vector<int> deadChannels);

    /**
     * Get size of the list of dead channels.
     * @return Size.
     */
    int getFebDeadChannelsSize() const {return m_febDeadChannels.size();}

    /**
     * Get a channel number from the list of dead channels.
     * @param[in] i Index of the element in the list.
     * @return Channel id.
     */
    int getAsicDeadChannel(unsigned int i) const;

    /**
     * Get a list of dead channels.
     * @return Vector of channels.
     */
    std::vector<int> getAsicDeadChannels() const {return m_asicDeadChannels; }

    /**
     * Add a channel number to the list of dead channels.
     * @param[in] channel ASIC channel id.
     */
    void appendAsicDeadChannel(int channel) {m_asicDeadChannels.push_back(channel); }

    /**
     * Set vector of dead channel numbers.
     * @param[in] deadChannels ASIC channel id.
     */
    void setAsicDeadChannels(std::vector<int> deadChannels);

    /**
     * Get size of the list of dead channels.
     * @return Size.
     */
    int getAsicDeadChannelsSize() const {return m_asicDeadChannels.size();}

    /**
     * Get a channel number from the list of channels with bad connections.
     * @param[in] i Index of the element in the list.
     * @return Channel id.
     */
    int getAsicBadConnChannel(unsigned int i) const;

    /**
     * Get a list of channels with bad connections.
     * @return vector of channels.
     */
    std::vector<int> getAsicBadConnChannels() const {return m_asicBadConnChannels; }

    /**
     * Add a channel number to the list of channels with bad connections.
     * @param[in] channel ASIC channel id.
     */
    void appendAsicBadConnChannel(int channel) { m_asicBadConnChannels.push_back(channel); }

    /**
     * Set vector of bad connection channel numbers.
     * @param[in] badConnChannels ASIC channel id
     */
    void setAsicBadConnChannels(std::vector<int> badConnChannels);

    /**
     * Get size of the list of channels with bad connections.
     * @return Size.
     */
    int getAsicBadConnChannelsSize() const {return m_asicBadConnChannels.size();}

    /**
     * Get a channel number from the list of channels with
     * bad offset adjustment.
     * @param[in] i Index of the element in the list.
     * @return Channel id.
     */
    int getAsicBadOffsetChannel(unsigned int i) const;

    /**
     * Get a list of channels with bad offset adjustment.
     * @return Vector of channels.
     */
    std::vector<int> getAsicBadOffsetChannels() const {return m_asicBadOffsetChannels; }

    /**
     * Add a channel number to the list of channels with bad offset adjustment.
     * @param[in] channel ASIC channel id.
     */
    void appendAsicBadOffsetChannel(int channel) { m_asicBadOffsetChannels.push_back(channel); }

    /**
     * Set vector of bad offset channel numbers.
     * @param[in] badOffsetChannels ASIC channel id.
     */
    void setAsicBadOffsetChannels(std::vector<int> badOffsetChannels);

    /**
     * Get size of the list of channels with bad offset adjustment.
     * @return Size.
     */
    int getAsicBadOffsetChannelsSize() const {return m_asicBadOffsetChannels.size();}

    /**
     * Get a channel number from the list of channels with bad linearity.
     * @param[in] i Index of the element in the list.
     * @return channel id
     */
    int getAsicBadLinChannel(unsigned int i) const;

    /**
     * Get a list of channels with bad linearity.
     * @return Vector of channels.
     */
    std::vector<int> getAsicBadLinChannels() const {return m_asicBadLinChannels; }

    /**
     * Add a channel number to the list of channels with bad linearity.
     * @param[in] channel ASIC channel id.
     */
    void appendAsicBadLinChannel(int channel) { m_asicBadLinChannels.push_back(channel); }

    /**
     * Set vector of bad linearity channel numbers.
     * @param[in] badLinChannels ASIC channel id
     */
    void setAsicBadLinChannels(std::vector<int> badLinChannels);

    /**
     * Get size of the list of channels with bad linearity.
     * @return Size.
     */
    int getAsicBadLinChannelsSize() const {return m_asicBadLinChannels.size();}

    /**
     * Get a channel number from the list of channels with bad linearity.
     * @param[in] i Index of the element in the list.
     * @return Channel id.
     */
    int getFebListOfBadChannel(unsigned int i) const;

    /**
     * Get a list of all bad channels.
     * @return Vector of channels
     */
    std::vector<int> getFebListOfBadChannels() const { return m_febListOfBadChannels; }

    /**
     * Get size of the list of all bad channels.
     * @return Size.
     */
    int getFebListOfBadChannelsSize() const { return m_febListOfBadChannels.size(); }


  private:
    int m_id;                   /**< global ID */
    std::string m_hapdSerial;   /**< serial number of the sensor */
    int m_febSerial;            /**< serial number of the FEB */

    std::vector<int> m_hapdCutChannels;       /**< List of cut channels on the HAPD chip */
    std::vector<int> m_hapdBadChannels;       /**< List of bad (cut and dead channels) on the HAPD chip */

    std::vector<int> m_febDeadChannels;       /**< List of dead channels on the FEB */

    std::vector<int> m_asicDeadChannels;      /**< List of dead channels on the ASIC chip */
    std::vector<int> m_asicBadConnChannels;   /**< List of channels with bad connections during the measurement */
    std::vector<int> m_asicBadOffsetChannels; /**< List of channels with bad offset adjustment */
    std::vector<int> m_asicBadLinChannels;    /**< List of channels with bad linearity */

    std::vector<int> m_hapdListOfBadChannels; /**< List of all bad channels on the HAPD */
    std::vector<int> m_febListOfBadChannels;  /**< List of all bad channels on the FEB */


    ClassDef(ARICHBadChannels, 1); /**< ClassDef */
  };
} // end namespace Belle2
