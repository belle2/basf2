/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <framework/logging/Logger.h>
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
    TOPPmtTTSHisto()
    {}

    /**
     * Full constructor
     * @param serialNumber PMT serial number
     * @param HV high voltage used for the measurement
     */
    TOPPmtTTSHisto(const std::string& serialNumber,
                   float HV):
      m_serialNumber(serialNumber), m_HV(HV)
    {}

    /**
     * Set PMT serial number
     * @param serNum serial number
     */
    void setSerialNumber(const std::string& serNum) {m_serialNumber = serNum;}

    /**
     * Set high voltage used for TTS measurement
     * @param HV high voltage
     */
    void setHV(float HV) {m_HV = HV;}

    /**
     * Set histogram for a given pixel (make a copy)
     * @param pmtPixel pmtPixel number (1-based)
     * @param histo TTS histogram
     */
    void setHistogram(unsigned pmtPixel, const TH1F* histo)
    {
      if (!histo) {
        B2ERROR("TOPPmtTTSHisto::setHistogram: null pointer received");
        return;
      }
      pmtPixel--;
      if (pmtPixel >= c_NumPmtPixels) {
        B2ERROR("TOPPmtTTSHisto::setHistogram: invalid PMT pixel "
                << LogVar("PMT pixel", pmtPixel + 1));
        return;
      }
      m_histo[pmtPixel] = *histo;
    }

    /**
     * Returns number of PMT pixels
     * @return number of pixels
     */
    int getNumOfPixels() const {return c_NumPmtPixels;}

    /**
     * Returns PMT serial number
     * @return serial number
     */
    const std::string& getSerialNumber() const {return m_serialNumber;}

    /**
     * Returns high voltage used for TTS measurement
     * @return high voltage
     */
    float getHV() const {return m_HV;}

    /**
     * Returns TTS histogram for a specified pmtPixel
     * @param pmtPixel pmtPixel number (1-based)
     * @return TTS histogram pointer or nullptr for invalid pmtPixel
     */
    const TH1F* getHistogram(unsigned pmtPixel) const
    {
      pmtPixel--;
      if (pmtPixel >= c_NumPmtPixels) return 0;
      return &m_histo[pmtPixel];
    }


  private:

    std::string m_serialNumber;      /**< PMT serial number, e.g. JTxxxx */
    float m_HV = 0;  /**< HV setting for which the set of histograms were taken */
    TH1F m_histo[c_NumPmtPixels];  /**< histograms of measured TTS (one per pmtPixel) */

    ClassDef(TOPPmtTTSHisto, 3); /**< ClassDef */

  };

} // end namespace Belle2


