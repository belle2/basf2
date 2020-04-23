/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
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

    Int_t   getISub(Const::EDetector detector, std::string label) const;
    Float_t getClock(Const::EDetector detector, std::string label) const;
    Float_t getGlobalClock() const;
    void    setClock(const Const::EDetector detector, std::string, Int_t iSub);
    void    setGlobalClock(Float_t globalClock);

  private:

    // Map of clocks derived from the global clock.
    // Clocks are stored as integer divisor of the global clock
    std::map<Const::EDetector, std::map<std::string, int>> iSubMap;

    // Map of clocks not  derived from the global clock.
    // Clocks are stored as frequency in MhZ
    std::map<Const::EDetector, std::map<std::string, float>> clocksMap;

    Float_t m_globalClock; //[MHz]

    ClassDef(Clocks, 1)
  };
}
