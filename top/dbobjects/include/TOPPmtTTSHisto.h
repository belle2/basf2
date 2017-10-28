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
    TOPPmtTTSHisto()
    {}

    /**
     * Full constructor
     * @param serialNumber serial number
     */
    TOPPmtTTSHisto(const std::string& serialNumber):
      m_serialNumber(serialNumber)
    {}


    /**
     * Append new element to the map
     * @param channel PMT channel number (1-based)
     * @param hv high voltage used in taking the TTS histogram
     * @param histo TH1F of the TTS histogram
     */
    void appendHistogram(unsigned channel, double hv, TH1F* histo)
    {
      m_histo[channel - 1][hv] = histo;
    }


    /**
     * Returns PMT serial number
     * @return serial number
     */
    const std::string& getSerialNumber() const {return m_serialNumber;}

    /**
     * Returns map of TTS histograms, all HV settings for a given channel
     * @param channel channel number
     * @return map of TTS histograms
     */
    const std::map<float, TH1F*>& getTtsHisto(unsigned channel) const
    {
      if (channel > c_NumChannels) channel = c_NumChannels;
      channel--;
      return m_histo[channel];
    }


  private:

    std::string m_serialNumber;                   /**< serial number, e.g. JTxxxx */
    std::map<float, TH1F*> m_histo[c_NumChannels];  /**< histograms of measured TTS (one for each channel,
                 *   for several HV settings)
                 */

    ClassDef(TOPPmtTTSHisto, 1); /**< ClassDef */

  };

} // end namespace Belle2


