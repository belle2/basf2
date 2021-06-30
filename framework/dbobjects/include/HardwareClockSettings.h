/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gian Luca Pinna Angioni                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/gearbox/Const.h>
#include <TObject.h>
#include <map>

namespace Belle2 {

  /**
   * Database object containing the nominal accelerator RF value and the prescales to derive
   * the clock frequencies of the sub-detectors.
   * Global clock frequency (system clock) is RF/4.
   */
  class HardwareClockSettings : public TObject {

  public:

    /** Constructor */
    HardwareClockSettings();

    /**
     * Get the prescale factor used to derive a detector clock from the global clock frequency.
     * @param detector The ID of the detector.
     * @param label The label for the clock.
     * @return prescale factor
     */
    double getClockPrescale(Const::EDetector detector, std::string label) const;

    /**
     * Get the frequency of a detector clock.
     * @param detector The ID of the detector.
     * @param label The label for the clock.
     * @return frequency [GHz]
     */
    double getClockFrequency(Const::EDetector detector, std::string label) const;

    /**
     * Get the global clock (system clock) frequency.
     * @return frequency [GHz]
     */
    double getGlobalClockFrequency() const;

    /**
     * Get the accelerator RF value.
     * @return frequency [GHz]
     */
    double getAcceleratorRF() const;

    /**
     * Set the prescale value used to derive a detector clock from the global clock frequency.
     * @param detector The ID of the detector.
     * @param label The label for the clock.
     * @param prescale The prescale factor of the clock.
     */
    void setClockPrescale(const Const::EDetector detector, std::string label, double prescale);

    /**
     * Set the frequency value of detector clock not derived from the global clock
     * @param detector The ID of the detector.
     * @param label The label for the clock.
     * @param frequency The frequency for the clock [GHz]
     */
    void setClockFrequency(const Const::EDetector detector, std::string label, double frequency);

    /**
     * Set the accelerator RF value.
     * @param acceleratorRF Accelerator RF value [GHz].
     */
    void setAcceleratorRF(double acceleratorRF);

    /**
     * Check if the prescale of a clock is available.
     * @param detector The ID of the detector.
     * @param label The label for the clock.
     * @return true, if available
     */
    bool isPrescaleAvailable(Const::EDetector detector, std::string label) const;

    /**
     * Check if the frequency of a detector clock is available.
     * The check is performed for both the clocks derived from the globalclock and the clocks not derived from it.
     * @param detector The ID of the detector.
     * @param label The label for the clock.
     * @return true, if available
     */
    bool isFrequencyAvailable(Const::EDetector detector, std::string label) const;

    /**
     * Print the content of the class
     */
    void print() const;

  private:
    /**
     * Map of prescale factors used to derive the clock frequencies from the global clock frequency.
     */
    std::map<Const::EDetector, std::map<std::string, double>> m_prescaleMap;

    /**
     * Map of clock frequencies not derived from the global clock frequency.
     * Clock frequency units are GHz.
     */
    std::map<Const::EDetector, std::map<std::string, double>> m_clocksMap;

    /**
     * Accelerator radio frequency [GHz]
     */
    double m_acceleratorRF = 0;

    ClassDef(HardwareClockSettings, 3)

  };

}
