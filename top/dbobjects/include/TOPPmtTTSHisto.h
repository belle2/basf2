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
   * Raw measurements of TTS (histograms) for each PMT pixel
   */
  class TOPPmtTTSHisto : public TObject {
  public:
    /**
     * number of PMT pixels
     */
    enum {c_NumPmtPixels = 16};

    /**
     * Default constructor
     */
    TOPPmtTTSHisto():
      m_serialNumber(""), m_HV(0), m_histo()
    {}

    /**
     * Full constructor
     * @param serialNumber serial number
     * @param HV HV setting
     */
    TOPPmtTTSHisto(const std::string& serialNumber,
                   float HV):
      m_serialNumber(serialNumber), m_HV(HV), m_histo()
    {}

    /**
     * Set PMT serial number
     * @param serNum serial number
     */
    void setSerialNumber(const std::string& serNum) {m_serialNumber = serNum;}

    /**
     * Set HV setting used for TTS measurement
     * @param HV high voltage setting
     */
    void setHV(float HV) {m_HV = HV;}

    /**
     * Append new element to the array
     * @param pmtPixel pmtPixel number (1-based)
     * @param histo vector of TH1F's for the TTS histograms
     */
    void setHistogram(int pmtPixel, TH1F* histo)
    {
      m_histo[pmtPixel - 1] = histo;
    }

    /**
     * Returns PMT serial number
     * @return serial number
     */
    const std::string& getSerialNumber() const {return m_serialNumber;}

    /**
     * Returns HV setting used for TTS measurement
     * @return HV
     */
    float getHV() const {return m_HV;}

    /**
     * Returns TTS histogram for a specified pmtPixel and HV setting
     * @param pmtPixel pmtPixel number
     * @return TTS histogram (TH1F*)
     */
    TH1F* getTTSHisto(int pmtPixel) const
    {
      if (pmtPixel > c_NumPmtPixels) pmtPixel = c_NumPmtPixels;
      pmtPixel--;
      return m_histo[pmtPixel];
    }


  private:

    std::string m_serialNumber;      /**< serial number, e.g. JTxxxx */
    float m_HV;                      /**< HV setting for which the set of histograms were taken */
    TH1F* m_histo[c_NumPmtPixels];     /**< array of TTS histograms of measured TTS (one for each pmtPixel),
              *   for the specific HV setting
              */

    ClassDef(TOPPmtTTSHisto, 2); /**< ClassDef */

  };

} // end namespace Belle2


