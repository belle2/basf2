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
#include <iostream>

namespace Belle2 {
  /**
   * Provide clocks for each sub-detectors.
   *
   **/

  class Clocks : public TObject {

  public:

    /** Constructor */
    Clocks();

    Int_t   getClockPrescale(Const::EDetector detector, std::string label) const;
    Float_t getClockFrequency(Const::EDetector detector, std::string label) const;
    Float_t getGlobalClockFrequency() const;
    Float_t getAcceleratorRF() const;
    void    setClockPrescale(const Const::EDetector detector, std::string, Int_t prescale);
    void    setAcceleratorRF(Float_t AcceleratorRF);

  private:

    // Map of clocks derived from the global clock.
    // Clocks are stored as integer divisor of the global clock
    std::map<Const::EDetector, std::map<std::string, int>> prescaleMap;

    // Map of clocks not  derived from the global clock.
    // Clocks are stored as frequency in MhZ
    std::map<Const::EDetector, std::map<std::string, float>> clocksMap;

    Float_t m_AcceleratorRF; //[MHz]

    ClassDef(Clocks, 1)
  };
}
