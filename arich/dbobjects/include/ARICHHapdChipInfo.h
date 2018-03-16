/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Rok Pestotnik, Manca Mrvar                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <TH2F.h>
#include <TGraph.h>
#include <vector>

namespace Belle2 {

  /**
   *   Contains manufacturer data of one of the 4 photo sensors chips
   */

  class ARICHHapdChipInfo: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHHapdChipInfo(): m_serial(""), m_chip(""), m_biasVoltage(0), m_gain(0), m_leakCurrent(NULL), m_bombardmentGain(NULL),
      m_bombardmentCurrent(), m_avalancheGain(NULL), m_avalancheCurrent(), m_biasVoltage2D(NULL), m_biasCurrent2D(NULL), m_channelId(0),
      m_cutChannel(), m_badChannel() {};

    /**
     * Constructor
     */
    explicit ARICHHapdChipInfo(const std::string& serial): m_serial(serial), m_chip(""), m_biasVoltage(0), m_gain(0),
      m_leakCurrent(NULL),
      m_bombardmentGain(NULL), m_bombardmentCurrent(), m_avalancheGain(NULL), m_avalancheCurrent(), m_biasVoltage2D(NULL),
      m_biasCurrent2D(NULL), m_channelId(0), m_cutChannel(), m_badChannel() {};

    /**
     * Destructor
     */
    ~ARICHHapdChipInfo() {};

    /** Return Hapd Serial number
     * @return Hapd Serial number
     */
    std::string getHapdSerial() const {return m_serial; };

    /** Set Hapd Serial number
     * @param Hapd Serial number
     */
    void setHapdSerial(const std::string& serial) {m_serial = serial; }

    /** Return Chip label
     * @return Chip label
     */
    std::string getChipLabel() const {return m_chip; };

    /** Set Chip label
     * @param Chip label
     */
    void setChipLabel(const std::string& chip) {m_chip = chip; }

    /**
     * Return Chip Bias Voltage
     * @return bias voltage
     */
    int getBiasVoltage() const {return m_biasVoltage;}

    /**
     * set Chip Bias Voltage
     * @param voltage bias voltage
     */
    void setBiasVoltage(int voltage)  { m_biasVoltage = voltage; }

    /**
     * Return Chip Gain at Operational Voltage
     * @return chip gain
     */
    int getGain() const {return m_gain;}

    /**
     * set Chip Gain at Operational Voltage
     * @param gain chip gain
     */
    void setGain(int gain) { m_gain = gain;}

    /**
     * Return Leakeage Current as a function of bias voltage
     * @return chip gain
     */
    TGraph* getLeakCurrent() const {return m_leakCurrent;}

    /**
     * set Leakeage Current as a function of bias voltage
     * @param current Leakeage current
     */
    void setLeakCurrent(TGraph* current) {m_leakCurrent = current;}

    /**
     * Return Bombardment Gain as a function of high voltage
     * @return bombardment gain
     */
    TGraph* getBombardmentGain() const {return m_bombardmentGain;}

    /**
     * Set Bombardment Gain as a function of high voltage
     * @param gain bombardment gain
     */
    void setBombardmentGain(TGraph* gain) {m_bombardmentGain = gain;}

    /**
     * Return Bombardment Current as a function of high voltage
     * @return current
     */
    TGraph* getBombardmentCurrent(unsigned int i) const;

    /**
     * set Bombardment Current as a function of high voltage
     * @param bombardment current current vs high voltage
     */
    void setBombardmentCurrent(std::vector<TGraph*> bcurrent);

    /**
     * Return Avalanche Gain as a function of bias voltage
     * @return avalanche gain
     */
    TGraph* getAvalancheGain() const {return m_avalancheGain;}

    /**
     * set Avalanche Gain as a function of bias voltage
     * @param gain avalanche gain vs bias
     */
    void setAvalancheGain(TGraph* gain) {m_avalancheGain = gain;}


    /**
     * Return Avalanche Current as a function of bias voltage
     * @return avalanche current
     */
    TGraph* getAvalancheCurrent(unsigned int i) const;

    /**
     * set Avalanche Current as a function of bias voltage
     * @param avalanche current current vs bias
     */
    void setAvalancheCurrent(std::vector<TGraph*> acurrent);

    /**
     * Return Bias Voltage as a function of the channel
     * @return 2D map of the voltage
     */
    TH2F* getBiasVoltage2D() const {return m_biasVoltage2D;}

    /**
     * Set Bias Voltage as a function of the channel
     * @param h2d 2D map of the voltage
     */
    void setBiasVoltage2D(TH2F* h2d) { m_biasVoltage2D = h2d;}

    /**
     * Return Bias Current as a function of the channel
     * @return 2D map of the current
     */
    TH2F* getBiasCurrent2D() const {return m_biasCurrent2D;}

    /**
     * set Bias Current as a function of the channel
     * @param h2d 2D map of the current
     */
    void setBiasCurrent2D(TH2F* h2d) { m_biasCurrent2D = h2d;}

    /**
     * Return Channel Number for the Bombardment and Avalanche measurements information
     * @return channel number
     */
    int getChannelNumber() const {return m_channelId;}

    /**
     * Set Channel Number for the Bombardment and Avalanche measurements information
     * @param channel channel number
     */
    void setChannelNumber(int channel) {m_channelId = channel;}

    /**
     * Return a channel number from the list of cut channels
     * @param i index of the element in the list
     * @return channel number
     */
    int getCutChannel(unsigned int i) const;

    /**
     * Add a channel number to the list of cut channels
     * @param channel HAPD channel number
     */
    void appendCutChannel(int channel) {m_cutChannel.push_back(channel); }

    /**
     * Set the list of cut channels
     * @param channel HAPD channel numbers
     */
    void setCutChannel(const std::vector<int>& channels) {m_cutChannel = channels; }

    /**
     * Return size of the list of cut channels
     * @return size
     */
    int getCutChannelsSize() const {return m_cutChannel.size();}

    /**
     * Return a channel number from the list of cut channels
     * @param i index of the element in the list
     * @return channel number
     */
    int getBadChannel(unsigned int i) const;

    /**
     * Add a channel number to the list of cut channels
     * @param channel HAPD channel number
     */
    void appendBadChannel(int ichannel) { m_badChannel.push_back(ichannel); }

    /**
     * Set the list of bad channels
     * @param channel HAPD channel numbers
     */
    void setBadChannel(const std::vector<int>& channels) {m_badChannel = channels; }

    /**
     * Return size of the list of cut channels
     * @return size
     */
    int getBadChannelsSize() const {return m_badChannel.size();}

  private:
    std::string m_serial;             /**< serial number of the sensor */
    std::string m_chip;               /**< chip label */
    int m_biasVoltage;                /**< chip bias voltage */
    int m_gain;                       /**< Total Gain at Operational Values */
    TGraph* m_leakCurrent;            /**< Leakege Current as a function of bias voltage */
    TGraph* m_bombardmentGain;        /**< Bombardment Gain as a function of high voltage */
    std::vector<TGraph*> m_bombardmentCurrent; /**< Bombardment Current as a function of high voltage */
    TGraph* m_avalancheGain;                   /**< Avalanche Gain as a function of bias voltage */
    std::vector<TGraph*> m_avalancheCurrent;   /**< Avalanche Current as a function of bias voltage */
    TH2F* m_biasVoltage2D;                     /**< Bias Voltage as a function of the channel */
    TH2F* m_biasCurrent2D;                     /**< Bias Current as a function of the channel */
    int m_channelId;                           /**< Channel Number for the Bombardment and Avalanche measurements information */
    std::vector<int> m_cutChannel;             /**< List of cut channels on the HAPD chip */
    std::vector<int> m_badChannel;             /**< List of bad (cut and dead channels) on the HAPD chip */


    ClassDef(ARICHHapdChipInfo, 3); /**< ClassDef */
  };
} // end namespace Belle2
