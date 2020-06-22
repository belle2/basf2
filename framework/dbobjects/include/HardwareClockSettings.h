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
   * Database object containing the nominal RF value and the prescales to derive
   * the clock frequencies of the sub-detectors.
   */
  class HardwareClockSettings : public TObject {

  public:

    /** Constructor */
    HardwareClockSettings();

    /**
     * Get the prescale value used to derived a detector clock from the global clock frequency.
     * @param detector The ID of the detector.
     * @param label The label for the clock.
     */
    Int_t   getClockPrescale(Const::EDetector detector, std::string label) const;
    /**
     * Get the frequency of a detector clock.
     * @param detector The ID of the detector.
     * @param label The label for the clock.
     */
    Double_t getClockFrequency(Const::EDetector detector, std::string label) const;
    /**
     * Get the global clock frequency.
     */
    Double_t getGlobalClockFrequency() const;
    /**
     * Get the accelerator RF value.
     */
    Double_t getAcceleratorRF() const;
    /**
     * Set the prescale value used to derive a detector clock from the global clock frequency.
     * @param detector The ID of the detector.
     * @param label The label for the clock.
     * @param prescale The prescale of the clock.
     */
    void    setClockPrescale(const Const::EDetector detector, std::string label, Int_t prescale);
    /**
    * Set the frequency value of detector clocks not derived from the global clock
    * @param detector The ID of the detector.
    * @param label The label for the clock.
    * @param frequency The frequency for the clock.
    */
    void    setClockFrequency(const Const::EDetector detector, std::string label, float frequency);
    /**
     * Set the accelerator RF value.
     * @param acceleratorRF Accelerator RF value.
     */
    void    setAcceleratorRF(Double_t acceleratorRF);
    /**
    * Check if the prescale of a clock is available.
    * @param detector The ID of the detector.
    * @param label The label for the clock.
    */
    bool isPrescaleAvailable(Const::EDetector detector, std::string label) const;
    /**
    * Check if the frequency of a detector clock is available.
    * The check is performed for both the clocks derived from the globalclock and the clocks not derived from it.
    * @param detector The ID of the detector.
    * @param label The label for the clock.
    */
    bool isFrequencyAvailable(Const::EDetector detector, std::string label) const;
    /**
    * Get the channel map from the database and
    * print it. fixme
    * Print the content of the two datamember containing the prescale and the clocks
    */
    void print();

  private:
    /**
     * Map of prescale used to derive the clocks from the global clock.
     */
    std::map<Const::EDetector, std::map<std::string, int>> m_prescaleMap;
    /**
     * Map of clocks not derived from the global clock.
     * HardwareClockSettings are stored as frequency in MhZ.
     */
    std::map<Const::EDetector, std::map<std::string, float>> m_clocksMap;

    Double_t m_acceleratorRF = 0; /** [MHz] */

    ClassDef(HardwareClockSettings, 1)
  };
}
