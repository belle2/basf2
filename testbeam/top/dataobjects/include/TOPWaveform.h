/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPWAVEFORM_H
#define TOPWAVEFORM_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * Class to store IRS waveforms
   */
  class TOPWaveform : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPWaveform():
      m_barID(0),
      m_channelID(0),
      m_hardChannelID(0),
      m_windowID(0) {
      for (int i = 0; i < dataSize; i++) m_data[i] = 0;
    }

    /** Full constructor for zero waveform (use setADC(i, adc) to set values)
     * @param barID quartz bar ID
     * @param channelID channel ID
     * @param hardchID  hardware channel ID
     * @param windowID  IRS window ID
     */
    TOPWaveform(int barID, int channelID, unsigned hardchID, unsigned windowID):
      m_barID(barID),
      m_channelID(channelID),
      m_hardChannelID(hardchID),
      m_windowID(windowID) {
      for (int i = 0; i < dataSize; i++) m_data[i] = 0;
    }

    /** Set waveform ADC value of sample i
     * @param i sample number (0-based index)
     * @param adc ADC value
     */
    void setADC(unsigned i, short adc) {
      if (i < dataSize) m_data[i] = adc;
    }

    /** Get quartz bar ID
     * @return bar ID
     */
    int getBarID() const { return m_barID; }

    /** Get channel ID
     * @return channel ID
     */
    int getChannelID() const { return m_channelID; }

    /** Get hardware channel ID
     * @return hardware channel ID
     */
    int getHardChannelID() const { return m_hardChannelID; }

    /** Get IRS window ID
     * @return window ID
     */
    int getWindowID() const { return m_windowID; }

    /** Get waveform size
     * @return size
     */
    unsigned getSize() const {return dataSize;}

    /** Get waveform ADC value of sample i
     * @param i sample number (0-based index)
     * @return ADC value
     */
    int getADC(unsigned i) const { if (i < dataSize) return m_data[i]; return 0; }

    /** Get complete waveform
     * @return vector of ADC values
     */
    std::vector<short> getWaveform() const {
      std::vector<short> wf;
      for (int i = 0; i < dataSize; i++) wf.push_back(m_data[i]);
      return wf;
    }

  private:
    enum {dataSize = 64}; /**< waveform sample size */

    int m_barID;                 /**< quartz bar ID */
    int m_channelID;             /**< software channel ID */
    unsigned m_hardChannelID;    /**< hardware channel ID */
    unsigned m_windowID;         /**< IRS window ID */
    short m_data[dataSize];      /**< waveform ADC values */

    ClassDef(TOPWaveform, 1); /**< ClassDef */

  };


} // end namespace Belle2

#endif
