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
    ARICHHapdChipInfo(): m_biasVoltage(0), m_gain(0), m_leakCurrent(NULL), m_bombardmentGain(NULL), m_bombardmentCurrent(NULL),
      m_avalancheGain(NULL), m_avalancheCurrent(NULL), m_biasVoltage2D(NULL), m_biasCurrent2D(NULL), m_channelId(0) {};

    /**
     * Constructor
     */
    ARICHHapdChipInfo(float biasVoltage, float gain, TGraph* leakCurrent, TGraph* bombardmentGain, TGraph* bombardmentCurrent,
                      TGraph* avalancheGain, TGraph* avalancheCurrent, TH2F* biasVoltage2D, TH2F* biasCurrent2D, int channel)
    {
      m_biasVoltage = biasVoltage;
      m_gain = gain;
      m_leakCurrent = leakCurrent;
      m_bombardmentGain = bombardmentGain;
      m_bombardmentCurrent = bombardmentCurrent;
      m_avalancheGain = avalancheGain;
      m_avalancheCurrent = avalancheCurrent;
      m_biasVoltage2D = biasVoltage2D;
      m_biasCurrent2D = biasCurrent2D;
      m_channelId = channel;
    }


    /**
     * Destructor
     */
    ~ARICHHapdChipInfo();

    /**
     * Return Chip Bias Voltage
     * @return bias voltage
     */
    float getBiasVoltage() const {return m_biasVoltage;}

    /**
     * set Chip Bias Voltage
     * @param voltage bias voltage
     */
    void setBiasVoltage(float voltage)  { m_biasVoltage = voltage; }

    /**
     * Return Chip Gain at Operational Voltage
     * @return chip gain
     */
    float getGain() const {return m_gain;}

    /**
     * set Chip Gain at Operational Voltage
     * @param gain chip gain
     */
    void setGain(float gain) { m_gain = gain;}

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
    TGraph* getBombardmentCurrent() const {return m_bombardmentCurrent;}

    /**
     * set Bombardment Current as a function of high voltage
     * @param bombardment current current vs high voltage
     */
    void setBombardmentCurrent(TGraph* current) {m_bombardmentCurrent = current;}

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
    TGraph* getAvalancheCurrent() const {return m_avalancheCurrent;}

    /**
     * set Avalanche Current as a function of bias voltage
     * @param avalanche current current vs bias
     */
    void setAvalancheCurrent(TGraph* current) {m_avalancheCurrent = current;}

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
     * Return a channel number from the list of dead channels
     * @param i index of the element in the list
     * @return channel number
     */
    int getDeadChannel(unsigned int i) const { if (i < m_deadChannel.size()) return m_deadChannel[i]; else return -1;}

    /**
     * Add a channel number to the list of dead channels
     * @param channel HAPD channel number
     */
    void appendDeadChannel(unsigned int channel) {m_deadChannel.push_back(channel); }

    /**
     * Return size of the list of dead channels
     * @return size
     */
    int getDeadChannelsSize() const {return m_deadChannel.size();}

    /**
     * Return a channel number from the list of cut channels
     * @param i index of the element in the list
     * @return channel number
     */
    int getCutChannel(unsigned int i) const { if (i < m_cutChannel.size())return m_cutChannel[i]; else return -1;}

    /**
     * Add a channel number to the list of cut channels
     * @param channel HAPD channel number
     */
    void appendCutChannel(unsigned int ichannel) { m_cutChannel.push_back(ichannel); }

    /**
     * Return size of the list of cut channels
     * @return size
     */
    int getCutChannelsSize() const {return m_cutChannel.size();}

  private:
    float m_biasVoltage;            /**< chip bias voltage */
    float m_gain;                   /**< Total Gain at Operational Values */
    TGraph* m_leakCurrent;          /**< Leakege Current as a function of bias voltage */
    TGraph* m_bombardmentGain;      /**< Bombardment Gain as a function of high voltage */
    TGraph* m_bombardmentCurrent;   /**< Bombardment Current as a function of high voltage */
    TGraph* m_avalancheGain;        /**< Avalanche Gain as a function of bias voltage */
    TGraph* m_avalancheCurrent;     /**< Avalanche Current as a function of bias voltage */
    TH2F*   m_biasVoltage2D;        /**< Bias Voltage as a function of the channel */
    TH2F*   m_biasCurrent2D;        /**< Bias Current as a function of the channel */
    int m_channelId;                /**< Channel Number for the Bombardment and Avalanche measurements information */
    std::vector<unsigned int> m_deadChannel;      /**< List of dead channels on the HAPD chip */
    std::vector<unsigned int> m_cutChannel;       /**< List of cut channels on the HAPD chip */


    ClassDef(ARICHHapdChipInfo, 1); /**< ClassDef */
  };
} // end namespace Belle2
