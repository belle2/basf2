/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oskar Hartbrich, Marko Staric                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

namespace Belle2 {

  /**
   * Class to store IRSX waveform segments as trasnmitted by the TOP production firmware
   */
  class TOPProductionWaveform : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPProductionWaveform()
    {}

    /**
     * Full constructor
     * @param scrodID hardware SCROD ID
     * @param carrier hardware carrier number (0..3)
     * @param asic hardware asic number (0..3)
     * @param channel hardware channel number (0..7)
     * @param window hardware logic window number
     * @param startSample first sample number
     * @param data waveform ADC values (samples)
     */
    TOPProductionWaveform(unsigned scrodID,
                          unsigned carrier,
                          unsigned asic,
                          unsigned channel,
                          unsigned window,
                          unsigned startSample,
                          const std::vector<short>& data):
      m_scrodID(scrodID), m_carrier(carrier), m_asic(asic), m_channel(channel),
      m_window(window), m_physicalWindow(window), m_startSample(startSample), m_data(data)
    {

    }


    /**
     * Returns SCROD ID
     * @return SCROD ID
     */
    unsigned short getScrodID() const { return m_scrodID; }

    /**
     * Returns carrier number
     * @return carrier number
     */
    unsigned short getCarrier() const { return m_carrier;}

    /**
       * Returns ASIC number
       * @return ASIC number
       */
    unsigned short getASIC() const { return m_asic;}

    /**
     * Returns channel number
     * @return channel number
     */
    unsigned short getChannel() const { return m_channel;}

    /**
    * Returns window number
    * @return window number
    */
    unsigned short getWindow() const { return m_window;}

    /**
    * Returns physical window number
    * @return physical window number
    */
    unsigned short getPhysicalWindow() const { return m_physicalWindow;}

    /**
    * Set physical window number if != logic window number (heap window)
    */
    void setPhysicalWindow(unsigned physicalWindow) { m_physicalWindow = physicalWindow;}

    /**
    * Returns pedestal subtracted flag
    * @return pedestal subtracted flag
    */
    bool getPedestalSubtractedFlag() const { return m_isPedestalSubtracted;}

    /**
    * Set pedestal subtracted flag
    */
    void setPedestalSubtractedFlag(bool isPedestalSubtracted) { m_isPedestalSubtracted = isPedestalSubtracted;}

    /**
     * Returns waveform size (number of samples)
     * @return size
     */
    unsigned getSize() const {return m_data.size();}

    /**
     * Returns array of sample number corresponding to samples
     * @return vector of sample numbers
     */
    std::vector<short> getSampleNumbers() const
    {
      std::vector<short> sampleNumbers(m_data.size());

      std::iota(sampleNumbers.begin(), sampleNumbers.end(), m_startSample); //populate sampleNumbers vector

      return sampleNumbers;
    }

    /**
     * Returns waveform
     * @return vector of ADC values
     */
    const std::vector<short>& getWaveform() const
    {
      return m_data;
    }



  private:

    unsigned short m_scrodID = 0;       /**< SCROD ID */
    unsigned short m_carrier = 0;
    unsigned short m_asic = 0;
    unsigned short m_channel = 0;
    unsigned short m_window = 0;
    unsigned short m_physicalWindow = 0;
    unsigned short m_startSample = 0;

    bool m_isPedestalSubtracted = false;

    std::vector<short> m_data;      /**< waveform ADC values */

    ClassDef(TOPProductionWaveform, 1); /**< ClassDef */

  };


} // end namespace Belle2
