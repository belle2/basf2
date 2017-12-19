/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Alessandro Gaz                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <TObject.h>
#include <TH1F.h>

namespace Belle2 {

  /**
   * Raw measurements of TTS (histograms) for each PMT channel
   */
  class TOPPmtTTSHisto : public TObject {
  public:
    /**
     * number of PMT channels
     */
    enum {c_NumChannels = 16};

    /**
     * Default constructor
     */
    TOPPmtTTSHisto():
      m_serialNumber(""), m_hv(0), m_histo()
    {}

    /**
     * Full constructor
     * @param serialNumber serial number
     * @param hv HV setting
     */
    TOPPmtTTSHisto(const std::string& serialNumber,
                   int hv):
      m_serialNumber(serialNumber), m_hv(hv), m_histo()
    {}

    /**
     * Set PMT serial number
     * @param serNum serial number
     */
    void setSerialNumber(const std::string& serNum) {m_serialNumber = serNum;}

    /**
     * Set HV setting used for TTS measurement
     * @param hv high voltage setting
     */
    void setHv(int hv) {m_hv = hv;}

    /**
     * Append new element to the array
     * @param channel channel number (1-based)
     * @param histo vector of TH1F's for the TTS histograms
     */
    void setHistogram(int channel, TH1F histo)
    {
      m_histo[channel - 1] = histo;
    }

    /**
     * Returns PMT serial number
     * @return serial number
     */
    const std::string& getSerialNumber() const {return m_serialNumber;}

    /**
     * Returns HV setting used for TTS measurement
     * @return hv
     */
    int getHv() const {return m_hv;}

    /**
     * Returns TTS histogram for a specified channel and HV setting
     * @param channel channel number
     * @return TTS histogram (TH1F*)
     */
    TH1F getTtsHisto(int channel) const
    {
      if (channel > c_NumChannels) channel = c_NumChannels;
      channel--;
      return m_histo[channel];
    }


  private:

    std::string m_serialNumber;      /**< serial number, e.g. JTxxxx */
    int m_hv;                        /**< HV setting for which the set of histograms were taken */
    TH1F m_histo[c_NumChannels];     /**< array of TTS histograms of measured TTS (one for each channel),
              *   for the specific HV setting
              */

    ClassDef(TOPPmtTTSHisto, 1); /**< ClassDef */

  };

} // end namespace Belle2


