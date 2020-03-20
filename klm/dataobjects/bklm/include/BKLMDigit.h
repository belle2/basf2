/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/bklm/BKLMStatus.h>
#include <klm/dataobjects/KLMDigit.h>

namespace Belle2 {
  class BKLMSimHit;

  //! Store one BKLM strip hit as a ROOT object
  class BKLMDigit : public KLMDigit {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMDigit() : KLMDigit()
    {
    }

    //! Constructor with initial values for an RPC hit
    //! @param simHit pointer to the BKLMSimHit
    //! @param strip RPC strip number in a contiguous set
    explicit BKLMDigit(const BKLMSimHit* simHit, int strip) :
      KLMDigit(simHit, strip)
    {
    }

    //! Constructor with initial values for a scint hit
    //! @param simHit pointer to the BKLMSimHit
    explicit BKLMDigit(const BKLMSimHit* simHit) : KLMDigit(simHit)
    {
    }

    //! Destructor
    virtual ~BKLMDigit() {}

    //! Determine whether hit is in RPC or scintillator
    //! @return whether hit is in RPC (true) or scintillator (false)
    bool inRPC() const
    {
      return getLayer() >= BKLMElementNumbers::c_FirstRPCLayer;
    }

    //! Get readout coordinate
    //! @return readout coordinate (TRUE=phi, FALSE=z) of this strip
    bool isPhiReadout() const
    {
      return m_Plane == BKLMElementNumbers::c_PhiPlane;
    }

  private:

    //! Needed to make the ROOT object storable
    //! version 4 adds ctime etc
    ClassDefOverride(BKLMDigit, 8)

  };

} // end of namespace Belle2
