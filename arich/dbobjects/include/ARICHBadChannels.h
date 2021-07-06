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
   *   Contains manufacturer data of one of the 4 photo sensors chips
   */

  class ARICHBadChannels: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHBadChannels(): m_id(-1), m_hapdSerial(""), m_febSerial(0), m_hapdCutChannels(), m_hapdBadChannels(), m_febDeadChannels(),
      m_asicDeadChannels(), m_asicBadConnChannels(), m_asicBadOffsetChannels(), m_asicBadLinChannels() {};

    /**
     * Destructor
     */
    ~ARICHBadChannels() {};

    /** Return global ID
     * @return global ID
     */
    int getID() const {return m_id; }

    /** Set global ID
     * @param global ID
     */
    void setID(const int id) {m_id = id; }

    /** Return Hapd Serial number
     * @return Hapd Serial number
     */
    std::string getHapdSerial() const {return m_hapdSerial; }

    /** Set Hapd Serial number
     * @param Hapd Serial number
     */
    void setHapdSerial(const std::string& serial) {m_hapdSerial = serial; }

    /** Return FEB Serial number
     * @return FEB Serial number
     */
    int getFebSerial() const {return m_febSerial; }

    /** Set Hapd Serial number
     * @param Hapd Serial number
     */
    void setFebSerial(const int serial) {m_febSerial = serial; }

    /**
     * Return a channel number from the list of cut channels
     * @param i index of the element in the list
     * @return channel number
     */
    int getHapdCutChannel(unsigned int i) const;

    /**
     * Return a list of cut channels
     * @return vector of channels
     */
    std::vector<int> getHapdCutChannels() const {return m_hapdCutChannels; }

    /**
     * Add a channel number to the list of cut channels
     * @param channel HAPD channel number
     */
    void appendHapdCutChannel(int channel) {m_hapdCutChannels.push_back(channel); }

    /**
     * Set the list of cut channels
     * @param channel HAPD channel numbers
     */
    void setHapdCutChannel(std::vector<int> channels);

    /**
     * Return size of the list of cut channels
     * @return size
     */
    int getHapdCutChannelsSize() const {return m_hapdCutChannels.size(); }

    /**
     * Return a channel number from the list of cut channels
     * @param i index of the element in the list
     * @return channel number
     */
    int getHapdBadChannel(unsigned int i) const;

    /**
     * Return a list of bad channels
     * @return vector of channels
     */
    std::vector<int> getHapdBadChannels() const {return m_hapdBadChannels; }

    /**
     * Add a channel number to the list of cut channels
     * @param channel HAPD channel number
     */
    void appendHapdBadChannel(int ichannel) { m_hapdBadChannels.push_back(ichannel); }

    /**
     * Set the list of bad channels
     * @param channel HAPD channel numbers
     */
    void setHapdBadChannel(std::vector<int> channels);

    /**
     * Return size of the list of cut channels
     * @return size
     */
    int getHapdBadChannelsSize() const {return m_hapdBadChannels.size();}

    /**
     * Return a channel number from the list of channels with bad linearity
     * @param i index of the element in the list
     * @return channel id
     */
    int getHapdListOfBadChannel(unsigned int i) const;

    /**
     * Return a list of all bad channels
     * @return vector of channels
     */
    std::vector<int> getHapdListOfBadChannels() const { return m_hapdListOfBadChannels; }

    /**
     * Return size of the list of all bad channels
     * @return size
     */
    int getHapdListOfBadChannelsSize() const { return m_hapdListOfBadChannels.size(); }

    /**
     * Return a channel number from the list of dead channels
     * @param i index of the element in the list
     * @return channel id
     */
    int getFebDeadChannel(unsigned int i) const;

    /**
     * Return a list of dead channels
     * @return vector of channels
     */
    std::vector<int> getFebDeadChannels() const {return m_febDeadChannels; }

    /**
     * Add a channel number to the list of dead channels
     * @param channel FEB channel id
     */
    void appendFebDeadChannel(int channel) {m_febDeadChannels.push_back(channel); }

    /**
     * Set vector of dead channel numbers
     * @param channel FEB channel id
     */
    void setFebDeadChannels(std::vector<int> deadChannels);

    /**
     * Return size of the list of dead channels
     * @return size
     */
    int getFebDeadChannelsSize() const {return m_febDeadChannels.size();}

    /**
     * Return a channel number from the list of dead channels
     * @param i index of the element in the list
     * @return channel id
     */
    int getAsicDeadChannel(unsigned int i) const;

    /**
     * Return a list of dead channels
     * @return vector of channels
     */
    std::vector<int> getAsicDeadChannels() const {return m_asicDeadChannels; }

    /**
     * Add a channel number to the list of dead channels
     * @param channel ASIC channel id
     */
    void appendAsicDeadChannel(int channel) {m_asicDeadChannels.push_back(channel); }

    /**
     * Set vector of dead channel numbers
     * @param channel ASIC channel id
     */
    void setAsicDeadChannels(std::vector<int> deadChannels);

    /**
     * Return size of the list of dead channels
     * @return size
     */
    int getAsicDeadChannelsSize() const {return m_asicDeadChannels.size();}

    /**
     * Return a channel number from the list of channels with bad connections
     * @param i index of the element in the list
     * @return channel id
     */
    int getAsicBadConnChannel(unsigned int i) const;

    /**
     * Return a list of channels with bad connections
     * @return vector of channels
     */
    std::vector<int> getAsicBadConnChannels() const {return m_asicBadConnChannels; }

    /**
     * Add a channel number to the list of channels with bad connections
     * @param channel ASIC channel id
     */
    void appendAsicBadConnChannel(int ichannel) { m_asicBadConnChannels.push_back(ichannel); }

    /**
     * Return size of the list of channels with bad connections
     * @return size
     */
    int getAsicBadConnChannelsSize() const {return m_asicBadConnChannels.size();}

    /**
     * Set vector of bad connection channel numbers
     * @param channel ASIC channel id
     */
    void setAsicBadConnChannels(std::vector<int> badConnChannels);

    /**
     * Return a channel number from the list of channels with bad offset adjustment
     * @param i index of the element in the list
     * @return channel id
     */
    int getAsicBadOffsetChannel(unsigned int i) const;

    /**
     * Return a list of channels with bad offset adjustment
     * @return vector of channels
     */
    std::vector<int> getAsicBadOffsetChannels() const {return m_asicBadOffsetChannels; }

    /**
     * Add a channel number to the list of channels with bad offset adjustment
     * @param channel ASIC channel id
     */
    void appendAsicBadOffsetChannel(int ichannel) { m_asicBadOffsetChannels.push_back(ichannel); }

    /**
     * Return size of the list of channels with bad offset adjustment
     * @return size
     */
    int getAsicBadOffsetChannelsSize() const {return m_asicBadOffsetChannels.size();}

    /**
     * Set vector of bad offset channel numbers
     * @param channel ASIC channel id
     */
    void setAsicBadOffsetChannels(std::vector<int> badOffsetChannels);

    /**
     * Return a channel number from the list of channels with bad linearity
     * @param i index of the element in the list
     * @return channel id
     */
    int getAsicBadLinChannel(unsigned int i) const;

    /**
     * Return a list of channels with bad linearity
     * @return vector of channels
     */
    std::vector<int> getAsicBadLinChannels() const {return m_asicBadLinChannels; }

    /**
     * Add a channel number to the list of channels with bad linearity
     * @param channel ASIC channel id
     */
    void appendAsicBadLinChannel(int ichannel) { m_asicBadLinChannels.push_back(ichannel); }

    /**
     * Return size of the list of channels with bad linearity
     * @return size
     */
    int getAsicBadLinChannelsSize() const {return m_asicBadLinChannels.size();}

    /**
     * Set vector of bad linearity channel numbers
     * @param channel ASIC channel id
     */
    void setAsicBadLinChannels(std::vector<int> badLinChannels);

    /**
     * Return a channel number from the list of channels with bad linearity
     * @param i index of the element in the list
     * @return channel id
     */
    int getFebListOfBadChannel(unsigned int i) const;

    /**
     * Return a list of all bad channels
     * @return vector of channels
     */
    std::vector<int> getFebListOfBadChannels() const { return m_febListOfBadChannels; }

    /**
     * Return size of the list of all bad channels
     * @return size
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
