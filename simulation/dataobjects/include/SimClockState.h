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
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/Clocks.h>
#include <TObject.h>

namespace Belle2 {
  /**
   * Provide clocks for each sub-detectors and relative phases with respect to global tag.
   **/

  class SimClockState : public TObject {

  public:

    /** Constructor */
    SimClockState();

    void    update();
    Float_t getClockFrequency(Const::EDetector detector, std::string label);
    Float_t getTriggerOffset(Const::EDetector detector, std::string label);
    Int_t   getRevo9Status();

  private:

    DBObjPtr<Clocks> m_clock;

    Float_t globalClockFrequency; //[MHz]

    Int_t   revo9Status;
    Int_t   revo9nbit = 1280 * 9;
    ClassDef(SimClockState, 1)
  };
}
