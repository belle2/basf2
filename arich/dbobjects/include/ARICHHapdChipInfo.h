/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <TH2F.h>
#include <TGraph.h>
#include <vector>

namespace Belle2 {

  /**
   * Contains manufacturer data of one of the 4 photo sensors chips.
   */
  class ARICHHapdChipInfo: public TObject {

  public:

    /**
     * Default constructor.
     */
    ARICHHapdChipInfo(): m_serial(""), m_chip(""), m_biasVoltage(0), m_gain(0), m_leakCurrent(NULL), m_bombardmentGain(NULL),
      m_bombardmentCurrent(), m_avalancheGain(NULL), m_avalancheCurrent(), m_biasVoltage2D(NULL), m_biasCurrent2D(NULL), m_channelId(0),
      m_cutChannel(), m_badChannel() {};

    /**
     * Constructor.
     */
    explicit ARICHHapdChipInfo(const std::string& serial): m_serial(serial), m_chip(""), m_biasVoltage(0), m_gain(0),
      m_leakCurrent(NULL),
      m_bombardmentGain(NULL), m_bombardmentCurrent(), m_avalancheGain(NULL), m_avalancheCurrent(), m_biasVoltage2D(NULL),
      m_biasCurrent2D(NULL), m_channelId(0), m_cutChannel(), m_badChannel() {};

    /**
     * Destructor.
     */
    ~ARICHHapdChipInfo() {};

    /**
     * Get HAPD Serial number.
     * @return HAPD Serial number.
     */
    std::string getHapdSerial() const {return m_serial; };

    /**
     * Set HAPD serial number.
     * @param[in] serial HAPD serial number.
     */
    void setHapdSerial(const std::string& serial) {m_serial = serial; }

    /**
     * Get chip label.
     * @return Chip label.
     */
    std::string getChipLabel() const {return m_chip; };

    /**
     * Set Chip label.
     * @param[in] chip Chip label.
     */
    void setChipLabel(const std::string& chip) {m_chip = chip; }

    /**
     * Get chip bias Voltage.
     * @return Bias voltage.
     */
    int getBiasVoltage() const {return m_biasVoltage;}

    /**
     * Set chip bias voltage.
     * @param[in] voltage Bias voltage.
     */
    void setBiasVoltage(int voltage)  { m_biasVoltage = voltage; }

    /**
     * Get chip gain at operational voltage.
     * @return Chip gain.
     */
    int getGain() const {return m_gain;}

    /**
     * Set chip gain at operational voltage.
     * @param[in] gain Chip gain.
     */
    void setGain(int gain) { m_gain = gain;}

    /**
     * Get leakeage current as a function of bias voltage.
     * @return Leakage current.
     */
    TGraph* getLeakCurrent() const {return m_leakCurrent;}

    /**
     * Set leakeage current as a function of bias voltage.
     * @param[in] current Leakeage current.
     */
    void setLeakCurrent(TGraph* current) {m_leakCurrent = current;}

    /**
     * Get bombardment gain as a function of high voltage.
     * @return Bombardment gain.
     */
    TGraph* getBombardmentGain() const {return m_bombardmentGain;}

    /**
     * Set bombardment gain as a function of high voltage.
     * @param[in] gain Bombardment gain.
     */
    void setBombardmentGain(TGraph* gain) {m_bombardmentGain = gain;}

    /**
     * Get bombardment current as a function of high voltage.
     * @return Current.
     */
    TGraph* getBombardmentCurrent(unsigned int i) const;

    /**
     * Set bombardment current as a function of high voltage.
     * @param[in] bcurrent Bombardment current current vs high voltage.
     */
    void setBombardmentCurrent(std::vector<TGraph*> bcurrent);

    /**
     * Get avalanche Gain as a function of bias voltage.
     * @return Avalanche gain.
     */
    TGraph* getAvalancheGain() const {return m_avalancheGain;}

    /**
     * Set avalanche Gain as a function of bias voltage.
     * @param[in] gain Avalanche gain vs bias.
     */
    void setAvalancheGain(TGraph* gain) {m_avalancheGain = gain;}

    /**
     * Get avalanche current as a function of bias voltage.
     * @return Avalanche current
     */
    TGraph* getAvalancheCurrent(unsigned int i) const;

    /**
     * Set avalanche current as a function of bias voltage.
     * @param[in] acurrent Avalanche current current vs bias.
     */
    void setAvalancheCurrent(std::vector<TGraph*> acurrent);

    /**
     * Get bias voltage as a function of the channel.
     * @return 2D map of the voltage.
     */
    TH2F* getBiasVoltage2D() const {return m_biasVoltage2D;}

    /**
     * Set bias voltage as a function of the channel.
     * @param[in] h2d 2D map of the voltage.
     */
    void setBiasVoltage2D(TH2F* h2d) { m_biasVoltage2D = h2d;}

    /**
     * Get bias current as a function of the channel.
     * @return 2D map of the current.
     */
    TH2F* getBiasCurrent2D() const {return m_biasCurrent2D;}

    /**
     * Set bias current as a function of the channel.
     * @param[in] h2d 2D map of the current.
     */
    void setBiasCurrent2D(TH2F* h2d) { m_biasCurrent2D = h2d;}

    /**
     * Get channel number for the bombardment and avalanche
     * measurements information.
     * @return Channel number.
     */
    int getChannelNumber() const {return m_channelId;}

    /**
     * Set channel number for the bombardment and avalanche
     * measurements information.
     * @param[in] channel Channel number
     */
    void setChannelNumber(int channel) {m_channelId = channel;}

    /**
     * Get a channel number from the list of cut channels.
     * @param[in] i Index of the element in the list.
     * @return Channel number.
     */
    int getCutChannel(unsigned int i) const;

    /**
     * Add a channel number to the list of cut channels.
     * @param[in] channel HAPD channel number.
     */
    void appendCutChannel(int channel) {m_cutChannel.push_back(channel); }

    /**
     * Set the list of cut channels.
     * @param[in] channels HAPD channel numbers.
     */
    void setCutChannel(const std::vector<int>& channels) {m_cutChannel = channels; }

    /**
     * Get size of the list of cut channels.
     * @return Size.
     */
    int getCutChannelsSize() const {return m_cutChannel.size();}

    /**
     * Get a channel number from the list of bad channels.
     * @param[in] i Index of the element in the list.
     * @return Channel number.
     */
    int getBadChannel(unsigned int i) const;

    /**
     * Add a channel number to the list of bad channels.
     * @param[in] channel HAPD channel number.
     */
    void appendBadChannel(int channel) { m_badChannel.push_back(channel); }

    /**
     * Set the list of bad channels.
     * @param[in] channels HAPD channel numbers.
     */
    void setBadChannel(const std::vector<int>& channels) {m_badChannel = channels; }

    /**
     * Get size of the list of cut channels.
     * @return Size.
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
