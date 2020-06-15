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
   * the clock frequecies of the sub-detectors.
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
    Float_t getClockFrequency(Const::EDetector detector, std::string label) const;
    /**
     * Get the global clock frequency.
     */
    Float_t getGlobalClockFrequency() const;
    /**
     * Get the accelerator RF value.
     */
    Float_t getAcceleratorRF() const;
    /**
     * Set the prescale value used to derived a detector clock from the global clock frequency.
     * @param detector The ID of the detector.
     * @param label The label for the clock.
     * @param prescale The prescale for the clock.
     */
    void    setClockPrescale(const Const::EDetector detector, std::string, Int_t prescale);
    /**
     * Set the accelerator RF value.
     * @param acceleratorRF Accelerator RF value.
     */
    void    setAcceleratorRF(Float_t acceleratorRF);

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

    Float_t m_acceleratorRF; /** [MHz] */

    ClassDef(HardwareClockSettings, 1)
  };
}
