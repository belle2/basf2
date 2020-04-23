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
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/Clocks.h>
#include <TObject.h>

namespace Belle2 {
  /**
   * Provide clocks for each sub-detectors and relative phases with respect to global tag.
   **/

  class TriggerTimeOffset : public TObject {

  public:

    /** Constructor */
    TriggerTimeOffset();

    // Float_t getPhase(Const::EDetector detector);

    Float_t getClock(Const::EDetector detector, std::string label);
    Float_t getTriggerBit(Const::EDetector detector, std::string label);
    Int_t   getTriggerBitWrtRevo9();

  private:

    DBObjPtr<Clocks> m_clock;
    Float_t phase;
    Float_t globalClock; //[MHz]

    Int_t   triggerBitPosWrtRevo9;
    Int_t   revo9nbit = 1280 * 9;
    ClassDef(TriggerTimeOffset, 1)
  };
}