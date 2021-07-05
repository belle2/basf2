/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBase.h>

namespace Belle2 {

  /**
   * Nominal time-to-digit conversion parameters (simplified model)
   */
  class TOPNominalTDC: public TOPGeoBase {
  public:

    /**
     * Various constants
     */
    enum {c_WindowSize = 64, /**< number of samples per ASIC window */
          c_syncWindows = 2  /**< number of windows corresponding to syncTimeBase */
         };

    /**
     * Default constructor
     */
    TOPNominalTDC()
    {}

    /**
     * Useful constructor (new xml file version)
     * @param numWindows number of ASIC windows per waveform
     * @param subBits number of bits per sample
     * @param syncTimeBase synchonization time base (time width of c_syncWindows)
     * @param numofBunches number of bunches per syncTimeBase
     * @param offset time offset
     * @param pileupTime pile-up time
     * @param doubleHitResolution double hit resolution time
     * @param timeJitter r.m.s. of time jitter
     * @param efficiency electronic efficiency (fraction of hits above threshold)
     * @param name object name
     */
    TOPNominalTDC(int numWindows,
                  int subBits,
                  double syncTimeBase,
                  int numofBunches,
                  double offset,
                  double pileupTime,
                  double doubleHitResolution,
                  double timeJitter,
                  double efficiency,
                  const std::string& name = "TOPNominalTDC");

    /**
     * Useful constructor (old xml file version)
     * @param numBits number of bits
     * @param binWidth time width of a TDC bin
     * @param offset time offset
     * @param pileupTime pile-up time
     * @param doubleHitResolution double hit resolution time
     * @param timeJitter r.m.s. of time jitter
     * @param efficiency electronic efficiency (fraction of hits above threshold)
     * @param name object name
     */
    TOPNominalTDC(unsigned numBits,
                  double binWidth,
                  double offset,
                  double pileupTime,
                  double doubleHitResolution,
                  double timeJitter,
                  double efficiency,
                  const std::string& name = "TOPNominalTDCold"):
      TOPGeoBase(name),
      m_offset(offset),
      m_pileupTime(pileupTime),
      m_doubleHitResolution(doubleHitResolution),
      m_timeJitter(timeJitter),
      m_efficiency(efficiency),
      m_numBits(numBits),
      m_binWidth(binWidth),
      m_sampleWidth(binWidth)
    {}

    /**
     * Sets the number of ADC bits
     * @param adcBits number of adcBits
     */
    void setADCBits(unsigned adcBits) {m_adcBits = adcBits;}

    /**
     * Sets average of pedestals
     * @param averagePedestal average pedestal value
     */
    void setAveragePedestal(int averagePedestal) {m_averagePedestal = averagePedestal;}

    /**
     * Returns the number of ADC bits
     * @return number of ADC bits
     */
    unsigned getADCBits() const {return m_adcBits;}

    /**
     * Returns ADC range
     * @return overflow value
     */
    unsigned getADCRange() const {return 1 << m_adcBits;}

    /**
     * Returns average of pedestals
     * @return average of pedestals
     */
    int getAveragePedestal() const {return m_averagePedestal;}

    /**
     * Returns number of ASIC windows per waveform
     * @return number of ASIC windows per waveform
     */
    unsigned getNumWindows() const {return m_numWindows;}
    /**
     * Returns number of bits per sample
     * @return number of bits per sample
     */
    unsigned getSubBits() const {return m_subBits;}

    /**
     * Returns synchonization time base (time width of c_syncWindows)
     * @return synchonization time base
     */
    double getSyncTimeBase() const {return m_syncTimeBase;}

    /**
     * Returns number of bunches in the synchonization time base
     * @return number of bunches
     */
    unsigned getNumofBunches() const {return m_numofBunches;}

    /**
     * Returns bunch separation time (e.g. RF cycle - empty bunches are included!)
     * @return bunch separation time
     */
    double getBunchSeparationTime() const {return m_syncTimeBase / m_numofBunches;}

    /**
     * Returns time offset
     * @return time offset
     */
    double getOffset() const {return m_offset;}

    /**
     * Returns pile-up time
     * @return pile-up time
     */
    double getPileupTime() const {return m_pileupTime;}

    /**
     * Returns double hit resolution time
     * @return double hit resolution time
     */
    double getDoubleHitResolution() const {return m_doubleHitResolution;}

    /**
     * Returns r.m.s. of time jitter
     * @return r.m.s. of time jitter
     */
    double getTimeJitter() const {return m_timeJitter;}

    /**
     * Returns electronic efficiency
     * @return electronic efficiency
     */
    double getEfficiency() const {return m_efficiency;}

    /**
     * Returns number of bits
     * @return number of bits
     */
    unsigned getNumBits() const {return m_numBits;}

    /**
     * Returns time width of a TDC bin
     * @return time width of a TDC bin
     */
    double getBinWidth() const {return m_binWidth;}

    /**
     * Returns time difference between two samples
     * @return time between two samples
     */
    double getSampleWidth() const {return m_sampleWidth;}

    /**
     * Returns TDC overflow value
     * @return overflow value
     */
    int getOverflowValue() const {return 1 << m_numBits;}

    /**
     * Convert TDC count to time
     * @param TDC TDC count
     * @return time [ns]
     */
    double getTime(int TDC) const {return (TDC + 0.5) * m_binWidth - m_offset;}

    /**
     * Returns time range lower limit
     * @return lower limit
     */
    double getTimeMin() const {return -m_offset;}

    /**
     * Returns time range upper limit
     * @return upper limit
     */
    double getTimeMax() const {return getOverflowValue() * m_binWidth - m_offset;}

    /**
     * Converts time to TDC count.
     * For times being outside TDC range, TDC overflow value is returned.
     * @param time [ns]
     * @return TDC count
     */
    int getTDCcount(double time) const;

    /**
     * Converts time to sample number.
     * Use isSampleValid(int) to check the validity of sample number
     * @param time [ns]
     * @return sample number
     */
    int getSample(double time) const;

    /**
     * Converts sample number to time
     * @param sample sample number
     * @return time [ns]
     */
    double getSampleTime(int sample) const {return sample * m_sampleWidth - m_offset;}

    /**
     * Check for the validity of sample number
     * @param sample sample number
     * @return true if sample number is valid
     */
    bool isSampleValid(int sample) const;

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Nominal time-to-digit conversion parameters") const override;


  private:

    unsigned m_numWindows = 0; /**< number of ASIC windows per waveform */
    unsigned m_subBits = 0; /**< number of bits per sample */
    float m_syncTimeBase = 0; /**< time width of c_syncWindows */
    unsigned m_numofBunches = 0; /**< number of bunches per syncTimeBase */
    float m_offset = 0; /**< time offset */
    float m_pileupTime = 0; /**< pile-up time */
    float m_doubleHitResolution = 0; /**< double hit resolution time */
    float m_timeJitter = 0; /**< r.m.s. of time jitter */
    float m_efficiency = 0; /**< electronic efficiency (fract. of hits above threshold) */

    unsigned m_numBits = 0; /**< number of bits */
    float m_binWidth = 0; /**< time width of a TDC bin */
    float m_sampleWidth = 0; /**< time between two samples */

    unsigned m_adcBits = 12; /**< number of ADC bits */
    int m_averagePedestal = 0; /**< average of pedestals [ADC bins] */

    ClassDefOverride(TOPNominalTDC, 3); /**< ClassDef */

  };

} // end namespace Belle2
